#!/usr/bin/env python3
"""Phase 6E-B53D retail oracle: I_MASK exchange helper func_80073E10.

Standalone proof, independent of production C.  This oracle:

* asserts the SHA-exact retail executable;
* embeds and verifies all 6 literal func_80073E10 words plus the caller
  context words inside func_80076C34;
* executes the literal words with a tiny MIPS-I interpreter against a
  modeled 16-bit I_MASK register at 0x1F801074;
* proves previous-mask return, exact 16-bit write replacement, zero
  extension, argument truncation, repeated/alternating determinism;
* proves the exchange touches no I_STAT, no callback, and no GPU/DMA
  state;
* executes the dispatcher slice 0x80076CA0..0x80076D50 from the literal
  words to prove the func_80076C34 caller context: saved-mask store,
  marker delay slot, initialization-byte branch, enqueue dependency
  func_80073CF4, readiness poll, and the unresolved worker boundary.

It never imports or calls production code and never invents interrupt
delivery, I_STAT semantics, or hardware auto-progress.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys


EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"

MASK = 0xFFFFFFFF
RAM_BASE = 0x80000000
RAM_SIZE = 0x200000

EXCHANGE = 0x80073E10
EXCHANGE_END = 0x80073E28
EXCHANGE_WORDS = [
    0x3C038009,  # lui  v1,0x8009
    0x8C635674,  # lw   v1,0x5674(v1)      v1 = D_80095674
    0x00000000,  # nop
    0x94620000,  # lhu  v0,0(v1)           v0 = previous I_MASK
    0x03E00008,  # jr   ra
    0xA4640000,  # sh   a0,0(v1)           I_MASK = a0 (delay slot)
]
EXCHANGE_FILE_OFFSET = 0x64610

I_MASK_POINTER = 0x80095674
I_MASK = 0x1F801074
I_STAT = 0x1F801070

# func_80076C34 caller context (B53C-proven body SHA-256 applies).
D_INIT_BYTE = 0x8009574D
D_WORK_MARKER = 0x80095754
D_DRAWSYNC_CALLBACK = 0x80095758
D_SAVED_IMASK = 0x8009587C
D_PRODUCER = 0x80095874
D_CONSUMER = 0x80095878
D_GPUSTAT_POINTER = 0x80095854
D_DMA2_CHCR_POINTER = 0x80095860
GPUSTAT = 0x1F801814
DMA2_CHCR = 0x1F8010A8
GPUSTAT_READY_GP0 = 0x04000000
DMA2_BUSY = 0x01000000
SET_DMA_CALLBACK = 0x80073CF4
QUEUE_PUMP = 0x80076EE4

# Every executable jal func_80073E10 site (independent rescan; B53A census).
JAL_EXCHANGE = 0x0C000000 | ((EXCHANGE >> 2) & 0x3FFFFFF)
JAL_SITES = [
    0x80076CA0, 0x80076D48, 0x80076E9C,   # func_80076C34 dispatcher
    0x80076F10, 0x8007709C,               # func_80076EE4 pump
    0x80077154, 0x80077268,               # func_80077144 queue/GPU reset
    0x800774A8, 0x80077524,               # func_80077404 timeout recovery
]


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"FATAL: {message}")


class Exe:
    def __init__(self, path: pathlib.Path):
        data = path.read_bytes()
        got = hashlib.sha1(data).hexdigest()
        require(got == EXE_SHA1, f"{path} SHA-1 {got} != {EXE_SHA1}")
        require(data[:8] == b"PS-X EXE", f"{path} is not a PS-X EXE")
        self.taddr, self.tsize = struct.unpack_from("<II", data, 0x18)
        require(len(data) == 0x800 + self.tsize,
                "PS-X EXE size/header mismatch")
        self.image = data[0x800:]

    def body(self, address: int, size: int) -> bytes:
        offset = address - self.taddr
        require(offset >= 0 and offset + size <= len(self.image),
                f"executable range 0x{address:08X}+0x{size:X}")
        return self.image[offset:offset + size]

    def word(self, address: int) -> int:
        return struct.unpack("<I", self.body(address, 4))[0]


class DependencyStop(Exception):
    def __init__(self, pc: int, target: int, a0: int, a1: int):
        super().__init__(f"dependency 0x{target:08X} at 0x{pc:08X}")
        self.pc = pc
        self.target = target
        self.a0 = a0
        self.a1 = a1


class Machine:
    """Tiny MIPS-I interpreter for the B53D slice.

    Memory is the exact executable image inside a zeroed 2 MiB guest RAM
    window plus a hardware model holding only I_MASK, I_STAT, GPUSTAT, and
    DMA2 CHCR.  16-bit MMIO widths are enforced: a 32-bit access to I_MASK
    or I_STAT is a hard failure.  GPUSTAT/CHCR read scripts are explicit
    per-scenario controls, never automatic progress.
    """

    def __init__(self, exe: Exe, *, i_mask: int = 0,
                 gpu_status_reads: list[int] | None = None,
                 dma2_chcr: int = 0x00000401):
        self.ram = bytearray(RAM_SIZE)
        offset = exe.taddr - RAM_BASE
        self.ram[offset:offset + exe.tsize] = exe.image
        self.i_mask = i_mask & 0xFFFF
        self.i_stat = 0x0000
        self.dma2_chcr = dma2_chcr & MASK
        self.gpu_status_reads = list(gpu_status_reads or [])
        self.i_stat_accesses: list[tuple[int, str]] = []
        self.gpu_status_read_count = 0
        self.mmio_writes: list[tuple[int, int, int]] = []
        self.ram_writes: list[tuple[int, int, int]] = []
        self.reg = [0] * 32
        self.reg[31] = 0xDEADBEEC  # ra sentinel: jr ra ends func_80073E10
        self.steps = 0
        self.exec_count = 0

    # -- memory -----------------------------------------------------------
    def _check_i_stat(self, address: int, kind: str) -> None:
        if address == I_STAT:
            self.i_stat_accesses.append((self.pc, kind))

    def load32(self, address: int) -> int:
        require(address % 4 == 0, f"unaligned lw 0x{address:08X}")
        if address in (I_MASK, I_STAT):
            raise SystemExit(
                f"FATAL: 32-bit read of 16-bit register 0x{address:08X}")
        if address == GPUSTAT:
            self.gpu_status_read_count += 1
            if self.gpu_status_reads:
                return self.gpu_status_reads.pop(0) & MASK
            return GPUSTAT_READY_GP0
        if address == DMA2_CHCR:
            return self.dma2_chcr
        require(RAM_BASE <= address < RAM_BASE + RAM_SIZE,
                f"lw outside guest RAM 0x{address:08X}")
        return struct.unpack_from("<I", self.ram, address - RAM_BASE)[0]

    def load16(self, address: int) -> int:
        require(address % 2 == 0, f"unaligned lhu 0x{address:08X}")
        self._check_i_stat(address, "lhu")
        if address == I_MASK:
            return self.i_mask
        if address == I_STAT:
            return self.i_stat
        require(RAM_BASE <= address < RAM_BASE + RAM_SIZE,
                f"lhu outside guest RAM 0x{address:08X}")
        return struct.unpack_from("<H", self.ram, address - RAM_BASE)[0]

    def load8(self, address: int) -> int:
        require(RAM_BASE <= address < RAM_BASE + RAM_SIZE,
                f"lbu outside guest RAM 0x{address:08X}")
        return self.ram[address - RAM_BASE]

    def store32(self, address: int, value: int) -> None:
        require(address % 4 == 0, f"unaligned sw 0x{address:08X}")
        if address in (I_MASK, I_STAT):
            raise SystemExit(
                f"FATAL: 32-bit write of 16-bit register 0x{address:08X}")
        if address in (GPUSTAT, DMA2_CHCR):
            self.mmio_writes.append((address, value & MASK, 32))
            return
        require(RAM_BASE <= address < RAM_BASE + RAM_SIZE,
                f"sw outside guest RAM 0x{address:08X}")
        struct.pack_into("<I", self.ram, address - RAM_BASE, value & MASK)
        self.ram_writes.append((address, value & MASK, 32))

    def store16(self, address: int, value: int) -> None:
        require(address % 2 == 0, f"unaligned sh 0x{address:08X}")
        self._check_i_stat(address, "sh")
        if address == I_MASK:
            self.i_mask = value & 0xFFFF
            self.mmio_writes.append((address, value & 0xFFFF, 16))
            return
        if address == I_STAT:
            self.i_stat = value & 0xFFFF
            return
        require(RAM_BASE <= address < RAM_BASE + RAM_SIZE,
                f"sh outside guest RAM 0x{address:08X}")
        struct.pack_into("<H", self.ram, address - RAM_BASE, value & 0xFFFF)
        self.ram_writes.append((address, value & 0xFFFF, 16))

    def ram32(self, address: int) -> int:
        return struct.unpack_from("<I", self.ram, address - RAM_BASE)[0]

    # -- execution --------------------------------------------------------
    def step(self, pc: int) -> int:
        """Execute one instruction; return next PC (already delay-aware)."""
        self.exec_count += 1
        word = self.load32(pc)
        self.pc = pc
        op = word >> 26
        rs = (word >> 21) & 31
        rt = (word >> 16) & 31
        rd = (word >> 11) & 31
        imm = word & 0xFFFF
        simm = imm - 0x10000 if imm & 0x8000 else imm

        def setr(index: int, value: int) -> None:
            if index:
                self.reg[index] = value & MASK

        if op == 0x0F:                                  # lui
            setr(rt, imm << 16)
            return pc + 4
        if op == 0x09:                                  # addiu
            setr(rt, self.reg[rs] + simm)
            return pc + 4
        if op == 0x23:                                  # lw
            setr(rt, self.load32(self.reg[rs] + simm))
            return pc + 4
        if op == 0x25:                                  # lhu
            setr(rt, self.load16(self.reg[rs] + simm))
            return pc + 4
        if op == 0x24:                                  # lbu
            setr(rt, self.load8(self.reg[rs] + simm))
            return pc + 4
        if op == 0x2B:                                  # sw
            self.store32(self.reg[rs] + simm, self.reg[rt])
            return pc + 4
        if op == 0x29:                                  # sh
            self.store16(self.reg[rs] + simm, self.reg[rt])
            return pc + 4
        if op == 0x04:                                  # beq (delay slot runs)
            target = pc + 4 + 4 * simm
            taken = self.reg[rs] == self.reg[rt]
            self.step(pc + 4)
            return target if taken else pc + 8
        if op == 0x05:                                  # bne (delay slot runs)
            target = pc + 4 + 4 * simm
            taken = self.reg[rs] != self.reg[rt]
            self.step(pc + 4)
            return target if taken else pc + 8
        if op == 0x02:                                  # j (delay slot runs)
            target = ((word & 0x3FFFFFF) << 2) | 0x80000000
            self.step(pc + 4)
            return target
        if op == 0x03:                                  # jal (delay slot runs)
            target = ((word & 0x3FFFFFF) << 2) | 0x80000000
            setr(31, pc + 8)
            self.step(pc + 4)
            return target
        if op == 0x00:
            funct = word & 0x3F
            if funct == 0x00:                           # sll / nop
                setr(rd, self.reg[rt] << ((word >> 6) & 31))
                return pc + 4
            if funct == 0x21:                           # addu
                setr(rd, self.reg[rs] + self.reg[rt])
                return pc + 4
            if funct == 0x24:                           # and
                setr(rd, self.reg[rs] & self.reg[rt])
                return pc + 4
            if funct == 0x08:                           # jr (delay slot runs)
                target = self.reg[rs]
                self.step(pc + 4)
                return target
            if funct == 0x09:                           # jalr (delay slot)
                target = self.reg[rs]
                a0 = self.reg[4]
                self.step(pc + 4)
                raise DependencyStop(pc, target, a0, self.reg[5])
        raise SystemExit(f"FATAL: unimplemented word 0x{word:08X} "
                         f"at 0x{pc:08X}")

    def run(self, pc: int, limit: int, stop: int | None = None) -> int:
        while True:
            self.steps += 1
            require(self.steps <= limit, "execution step limit exceeded")
            if stop is not None and pc == stop:
                return pc
            pc = self.step(pc)
            if pc == 0xDEADBEEC:      # jr ra with the sentinel return
                return pc


def run_exchange(exe: Exe, machine: Machine, argument: int) -> int:
    """Execute literal func_80073E10 and return its v0."""
    machine.reg[4] = argument & MASK          # a0
    machine.reg[31] = 0xDEADBEEC
    machine.steps = 0
    machine.exec_count = 0
    machine.run(EXCHANGE, 16)
    require(machine.exec_count == 6,
            f"func_80073E10 executed {machine.exec_count} words, expected 6")
    return machine.reg[2]


def verify_static(exe: Exe) -> None:
    # Exact body, range, and exclusive end.
    body = exe.body(EXCHANGE, 0x18)
    require(list(struct.unpack("<6I", body)) == EXCHANGE_WORDS,
            "func_80073E10 literal words differ from executable")
    require(exe.word(EXCHANGE_END) == 0x27BDFFE8,
            "exclusive end 0x80073E28 is not the next function prologue")

    # Exact access widths and ordering inside the body.
    require(EXCHANGE_WORDS[3] >> 26 == 0x25, "read is not lhu (16-bit)")
    require(EXCHANGE_WORDS[5] >> 26 == 0x29, "write is not sh (16-bit)")
    require(EXCHANGE_WORDS[4] == 0x03E00008, "return is not jr ra")
    # No call of any kind inside the helper: no IRQ dispatch or callback.
    for word in EXCHANGE_WORDS:
        require(word >> 26 != 0x03 and not (
            word >> 26 == 0 and word & 0x3F == 0x09),
            "func_80073E10 contains a call")

    # The I_MASK guest pointer is executable initial data, not code state.
    require(exe.word(I_MASK_POINTER) == I_MASK,
            "D_80095674 initial data is not 0x1F801074")
    require(exe.word(0x80095670) == I_STAT,
            "D_80095670 initial data is not 0x1F801070")

    # All nine executable call sites, with exact delay slots.
    for site in JAL_SITES:
        require(exe.word(site) == JAL_EXCHANGE,
                f"call site 0x{site:08X} is not jal func_80073E10")
    require(exe.word(0x80076CA4) == 0x00002021, "dispatcher mask-off a0 != 0")
    require(exe.word(0x80076F14) == 0x00002021, "pump mask-off a0 != 0")
    require(exe.word(0x80077158) == 0x00002021, "reset mask-off a0 != 0")
    require(exe.word(0x800774AC) == 0x00002021, "timeout mask-off a0 != 0")
    require(exe.word(0x80076D44) == 0x8C84587C
            and exe.word(0x80076D4C) == 0x00000000,
            "dispatcher direct restore does not reload D_8009587C")
    require(exe.word(0x80077098) == 0x8C845880,
            "pump restore does not reload D_80095880")
    require(exe.word(0x80077264) == 0x8C845884
            and exe.word(0x80077520) == 0x8C845884,
            "reset/timeout restore does not reload D_80095884")
    require(exe.word(0x80076EA0) == 0xAC225874,
            "enqueue restore delay slot is not producer publication")

    # func_80076C34 caller context around the exchange.
    require(exe.word(0x80076CB4) == 0xAC22587C,
            "previous mask store to D_8009587C missing")
    require(exe.word(0x80076CB8) == 0x90830001,
            "initialization byte lbu missing")
    require(exe.word(0x80076CC0) == 0x10600014,
            "initialization byte branch/target changed")
    require(exe.word(0x80076CC4) == 0xAC820008,
            "marker delay-slot store to D_80095754 missing")
    require(exe.word(0x80076D60) == (0x0C000000 | (SET_DMA_CALLBACK >> 2
                                                   & 0x3FFFFFF))
            and exe.word(0x80076D64) == 0x24040002,
            "enqueue path does not call func_80073CF4 with a0=2")
    require(exe.word(0x80076D5C) == 0x24A56EE4,
            "enqueue path pump identity is not 0x80076EE4")
    require(exe.word(0x80076D38) == 0x0260F809
            and exe.word(0x80076D34) == 0x02002021
            and exe.word(0x80076D3C) == 0x02402821,
            "direct worker call/argument registers changed")


def scenario_exchange_semantics(exe: Exe) -> None:
    # Initial state, exchange zero.
    m = Machine(exe)
    require(run_exchange(exe, m, 0) == 0 and m.i_mask == 0,
            "exchange(0) from reset is not 0/0")
    # Nonzero exchange: previous value returned, exact replacement.
    require(run_exchange(exe, m, 0xFFFF) == 0 and m.i_mask == 0xFFFF,
            "exchange(0xFFFF) did not return previous 0")
    require(run_exchange(exe, m, 0x1234) == 0xFFFF and m.i_mask == 0x1234,
            "exchange did not return 0xFFFF or replace exactly")
    # Argument truncation: sh stores only a0's low halfword.
    require(run_exchange(exe, m, 0xDEAD00A5) == 0x1234
            and m.i_mask == 0x00A5,
            "a0 upper bits leaked into the 16-bit mask")
    # Return zero extension: v0 upper bits are always clear.
    m2 = Machine(exe, i_mask=0x8000)
    result = run_exchange(exe, m2, 0x0001)
    require(result == 0x8000 and result >> 16 == 0,
            "previous mask return is not zero-extended")
    # High-bit and alternating masks, all 16 bits preserved.
    m3 = Machine(exe)
    previous = 0
    for value in (0x8000, 0xAAAA, 0x5555, 0xFFFF, 0x0000, 0xFFFF,
                  0x0001, 0x4000, 0x2000, 0x1000):
        got = run_exchange(exe, m3, value)
        require(got == previous and m3.i_mask == value,
                f"alternating exchange broke at 0x{value:04X}")
        previous = value
    m4 = Machine(exe)
    for bit in range(16):
        got = run_exchange(exe, m4, 1 << bit)
        require(got == (1 << bit) >> 1 and m4.i_mask == 1 << bit,
                f"bit {bit} not preserved")
    # Repeated determinism: identical sequences give identical results.
    seq = [0x0000, 0xFFFF, 0x0F0F, 0xF0F0, 0xBEEF, 0x0000]
    runs = []
    for _ in range(2):
        mx = Machine(exe)
        runs.append([run_exchange(exe, mx, v) for v in seq] + [mx.i_mask])
    require(runs[0] == runs[1], "repeated exchange sequence not deterministic")
    # No I_STAT access, no 32-bit MMIO access, no extra writes at all.
    for machine in (m, m2, m3, m4):
        require(machine.i_stat_accesses == [],
                "exchange touched I_STAT")
        require(all(w == 16 for _, _, w in machine.mmio_writes),
                "exchange performed a non-16-bit MMIO write")
        require(machine.ram_writes == [],
                "exchange wrote guest RAM")


def seed_dispatch_state(m: Machine, *, init: int, producer: int, consumer: int,
                        callback: int) -> None:
    m.ram[D_INIT_BYTE - RAM_BASE] = init & 0xFF
    struct.pack_into("<I", m.ram, D_PRODUCER - RAM_BASE, producer)
    struct.pack_into("<I", m.ram, D_CONSUMER - RAM_BASE, consumer)
    struct.pack_into("<I", m.ram, D_DRAWSYNC_CALLBACK - RAM_BASE, callback)


def run_dispatch_slice(exe: Exe, m: Machine, argument: int,
                       auxiliary: int) -> DependencyStop:
    """Execute 0x80076CA0.. from the literal words to the first dependency."""
    m.reg[16] = argument & MASK               # s0 = argument
    m.reg[18] = auxiliary & MASK              # s2 = auxiliary
    m.reg[19] = 0x80076664                    # s3 = worker (canonical)
    m.reg[31] = 0xDEADBEEC
    m.steps = 0
    try:
        m.run(0x80076CA0, 400)
    except DependencyStop as stop:
        return stop
    raise SystemExit("FATAL: dispatcher slice ran past both dependencies")


def scenario_caller_context(exe: Exe) -> None:
    # Canonical B53C state: init=1, empty ring, idle CHCR, no callback,
    # GPUSTAT ready -> direct worker boundary with exact arguments.
    m = Machine(exe)
    seed_dispatch_state(m, init=1, producer=0, consumer=0, callback=0)
    stop = run_dispatch_slice(exe, m, 0x8012A8B8, 0x8012A8B8)
    require(stop.pc == 0x80076D38 and stop.target == 0x80076664,
            "canonical slice did not stop at the direct worker call")
    require(stop.a0 == 0x8012A8B8 and stop.a1 == 0x8012A8B8,
            "worker arguments are not s0=argument / s1=auxiliary")
    require(m.i_mask == 0, "mask-off exchange did not leave I_MASK at 0")
    require(m.ram32(D_SAVED_IMASK) == 0, "saved previous mask wrong")
    require(m.ram32(D_WORK_MARKER) == 1, "marker delay slot did not run")
    require(m.gpu_status_read_count == 1,
            "ready GPUSTAT was not polled exactly once")
    require(m.ram32(D_PRODUCER) == 0 and m.ram32(D_CONSUMER) == 0,
            "slice mutated ring indices")

    # Nonzero previous mask is returned and saved exactly.
    m = Machine(exe, i_mask=0xBEEF)
    seed_dispatch_state(m, init=1, producer=0, consumer=0, callback=0)
    run_dispatch_slice(exe, m, 0x80100000, 0x80110000)
    require(m.ram32(D_SAVED_IMASK) == 0xBEEF and m.i_mask == 0,
            "previous mask 0xBEEF not saved / mask not cleared")

    # Initialization byte zero skips all checks: even a nonempty ring, a
    # busy channel, and a set callback still reach the direct worker.
    m = Machine(exe)
    seed_dispatch_state(m, init=0, producer=5, consumer=1,
                        callback=0x80012345)
    m.dma2_chcr = 0x01000201
    stop = run_dispatch_slice(exe, m, 0x80100000, 0x80110000)
    require(stop.pc == 0x80076D38,
            "uninitialized path did not skip to the direct worker")
    require(m.gpu_status_read_count == 1,
            "uninitialized path touched queue/DMA/callback state")

    # Each enqueue selector reaches func_80073CF4(2, 0x80076EE4) first and
    # publishes no ring entry before it.  The jal target itself is the
    # dependency boundary, so the run stops at the func_80073CF4 PC.
    for label, kwargs in (
        ("nonempty", dict(init=1, producer=1, consumer=0, callback=0)),
        ("dma-busy", dict(init=1, producer=0, consumer=0, callback=0)),
        ("callback", dict(init=1, producer=0, consumer=0,
                          callback=0x80012345)),
    ):
        m = Machine(exe)
        if label == "dma-busy":
            m.dma2_chcr = 0x01000201
        seed_dispatch_state(m, **kwargs)
        m.reg[31] = 0xDEADBEEC
        pc = m.run(0x80076CA0, 400, stop=SET_DMA_CALLBACK)
        require(pc == SET_DMA_CALLBACK,
                f"{label} slice did not reach func_80073CF4")
        require(m.reg[4] == 2 and m.reg[5] == QUEUE_PUMP,
                f"{label} enqueue arguments are not (2, 0x80076EE4)")
        require(m.ram32(D_SAVED_IMASK) == 0 and m.ram32(D_WORK_MARKER) == 1,
                f"{label} enqueue lost saved mask or marker")
        require(all(not (0x800BD030 <= a < 0x800BE830)
                    for a, _, _ in m.ram_writes),
                f"{label} slice published a ring entry before registration")

    # GPUSTAT readiness poll: a controlled not-ready/not-ready/ready script
    # is re-read without any queue or mask mutation inside the loop.
    m = Machine(exe, gpu_status_reads=[0, 0, GPUSTAT_READY_GP0])
    seed_dispatch_state(m, init=1, producer=0, consumer=0, callback=0)
    stop = run_dispatch_slice(exe, m, 0x80100000, 0x80110000)
    require(stop.pc == 0x80076D38, "scripted poll did not reach the worker")
    require(m.gpu_status_read_count == 3,
            "readiness poll did not re-read until ready")
    require(m.ram32(D_PRODUCER) == 0 and m.ram32(D_CONSUMER) == 0,
            "readiness poll mutated queue state")


def main() -> int:
    if len(sys.argv) > 2:
        raise SystemExit(f"usage: {sys.argv[0]} [SHA-exact-SLUS_006.62]")
    path = pathlib.Path(sys.argv[1] if len(sys.argv) == 2
                        else "build/extracted/disc1/SLUS_006.62")
    exe = Exe(path)
    verify_static(exe)
    scenario_exchange_semantics(exe)
    scenario_caller_context(exe)
    print("B53D ORACLE: PASS — literal I_MASK exchange executed; caller "
          "context proven; no I_STAT/callback/dispatch behavior")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
