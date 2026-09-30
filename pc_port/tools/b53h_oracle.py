#!/usr/bin/env python3
"""Phase 6E-B53H literal oracle for func_80076EE4 (libgpu queue pump).

This is an independent retail proof.  It verifies all 152 literal words of
the pump against the SHA-exact executable and then executes them on a
MIPS-I interpreter with exact delay slots.

The model has three explicit, caller-supplied inputs and NO autonomous
progress whatsoever:

  * DMA2 CHCR (the busy bit is an input, never cleared by the model);
  * GPUSTAT (readiness is an input; polling it does not make it ready);
  * guest RAM holding the retail ring, its indices, the work marker, the
    DrawSync callback word, and the DMA callback slots.

There is deliberately no DMA completion, no interrupt delivery, no
callback dispatch, and no wall clock.  Queued workers and the DrawSync
callback are controlled boundary events that record their invocation and
return a scripted value; they never touch hardware.

Nothing here imports, links, or calls production C.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_words


EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
PUMP_SHA256 = (
    "a124857ab6fd91a3b68ea3e5c2efa5337bf6ed5528ef94ca23bc4a781337a78c"
)

MASK = 0xFFFFFFFF
RAM_BASE = 0x80000000
RAM_SIZE = 0x00200000
RA_SENTINEL = 0xDEADBEE4
SP_INITIAL = 0x801FFF00

PUMP = 0x80076EE4
PUMP_END = 0x80077144
PUMP_SIZE = 0x260
PUMP_WORD_COUNT = 152
PUMP_FILE_OFFSET = 0x676E4

DISPATCHER = 0x80076C34
DRAWSYNC = 0x80077294
RESTORE_PUMP_CB = 0x80077A00

# Retail .data pointer words and queue globals.
CHCR_POINTER = 0x80095860
GPUSTAT_POINTER = 0x80095854
CHCR_MMIO = 0x1F8010A8
GPUSTAT_MMIO = 0x1F801814

WORK_MARKER = 0x80095754
DRAWSYNC_CALLBACK = 0x80095758
PRODUCER = 0x80095874
CONSUMER = 0x80095878
SAVED_IMASK_DISPATCH = 0x8009587C
SAVED_IMASK_PUMP = 0x80095880

RING = 0x800BD030
RING_STRIDE = 0x60
RING_SLOTS = 64
RING_SIZE = RING_STRIDE * RING_SLOTS

CB_TABLE = 0x800956C0
CB_SLOT2 = CB_TABLE + 8

JUMP_TABLE = 0x80095704
JTB_PUMP_FIELD = 0x24

CHCR_BUSY = 0x01000000
GPUSTAT_READY = 0x04000000

CANONICAL_WORKER = 0x80076664
CANONICAL_ARGUMENT = 0x800BD03C
CANONICAL_AUXILIARY = 0x8012B8B8


# ── Literal retail words (transcribed, never read back from the exe) ────
PUMP_WORDS = exe_words(0x80076EE4, 152)
    # 80076EE4  lui   v0,0x8009
    # 80076EE8  lw    v0,0x5860(v0)     ; -> DMA2 CHCR
    # 80076EEC  addiu sp,sp,-0x20
    # 80076EF0  sw    ra,0x18(sp)
    # 80076EF4  sw    s1,0x14(sp)
    # 80076EF8  sw    s0,0x10(sp)
    # 80076EFC  lw    v0,0(v0)          ; CHCR
    # 80076F00  lui   s0,0x0100         ; 0x01000000
    # 80076F04  and   v0,v0,s0
    # 80076F08  bnez  v0,0x80077130     ; BUSY -> epilogue
    # 80076F0C  addiu v0,zero,1         ; delay slot: return 1
    # 80076F10  jal   func_80073E10
    # 80076F14  addu  a0,zero,zero      ; delay slot: mask 0
    # 80076F18  lui   a0,0x8009
    # 80076F1C  lw    a0,0x5874(a0)     ; producer
    # 80076F20  lui   v1,0x8009
    # 80076F24  lw    v1,0x5878(v1)     ; consumer
    # 80076F28  lui   at,0x8009
    # 80076F2C  beq   a0,v1,0x80077094  ; empty -> restore
    # 80076F30  sw    v0,0x5880(at)     ; delay: save I_MASK
    # 80076F34  lui   v0,0x8009
    # 80076F38  lw    v0,0x5860(v0)
    # 80076F3C  nop
    # 80076F40  lw    v0,0(v0)          ; CHCR again
    # 80076F44  nop
    # 80076F48  and   v0,v0,s0
    # 80076F4C  bnez  v0,0x80077094     ; became busy -> restore
    # 80076F50  nop
    # 80076F54  lui   s1,0x0400         ; 0x04000000
    # 80076F58  lui   s0,0x0100
    # 80076F5C  lui   v0,0x8009         ; .L80076F5C loop head
    # 80076F60  lw    v0,0x5878(v0)     ; consumer
    # 80076F64  lui   v1,0x8009
    # 80076F68  lw    v1,0x5874(v1)     ; producer
    # 80076F6C  addiu v0,v0,1
    # 80076F70  andi  v0,v0,0x3F
    # 80076F74  bne   v0,v1,0x80076F98  ; not the last entry
    # 80076F78  nop
    # 80076F7C  lui   v0,0x8009
    # 80076F80  lw    v0,0x5758(v0)     ; DrawSync callback
    # 80076F84  nop
    # 80076F88  bnez  v0,0x80076F98
    # 80076F8C  addiu a0,zero,2         ; delay slot: channel 2
    # 80076F90  jal   func_80073CF4     ; deregister slot 2
    # 80076F94  addu  a1,zero,zero      ; delay slot: handler 0
    # 80076F98  lui   v1,0x8009         ; .L80076F98
    # 80076F9C  lw    v1,0x5854(v1)     ; -> GPUSTAT
    # 80076FA0  nop
    # 80076FA4  lw    v0,0(v1)          ; GPUSTAT
    # 80076FA8  nop
    # 80076FAC  and   v0,v0,s1
    # 80076FB0  bnez  v0,0x80076FCC     ; ready -> issue
    # 80076FB4  lui   a0,0x0400         ; delay slot
    # 80076FB8  lw    v0,0(v1)          ; .L80076FB8 poll
    # 80076FBC  nop
    # 80076FC0  and   v0,v0,a0
    # 80076FC4  beqz  v0,0x80076FB8     ; unbounded tight loop
    # 80076FC8  nop
    # 80076FCC  lui   a1,0x8009         ; .L80076FCC
    # 80076FD0  lw    a1,0x5878(a1)
    # 80076FD4  lui   v1,0x8009
    # 80076FD8  lw    v1,0x5878(v1)
    # 80076FDC  nop
    # 80076FE0  sll   v0,v1,1
    # 80076FE4  addu  v0,v0,v1
    # 80076FE8  sll   v0,v0,5           ; consumer*96
    # 80076FEC  sll   v1,a1,1
    # 80076FF0  addu  v1,v1,a1
    # 80076FF4  lui   a0,0x800C
    # 80076FF8  addu  a0,a0,v0
    # 80076FFC  lw    a0,-0x2FCC(a0)    ; entry+0x04 argument
    # 80077000  lui   a1,0x8009
    # 80077004  lw    a1,0x5878(a1)
    # 80077008  sll   v1,v1,5
    # 8007700C  sll   v0,a1,1
    # 80077010  addu  v0,v0,a1
    # 80077014  sll   v0,v0,5
    # 80077018  lui   a1,0x800C
    # 8007701C  addu  a1,a1,v0
    # 80077020  lw    a1,-0x2FC8(a1)    ; entry+0x08 auxiliary
    # 80077024  lui   v0,0x800C
    # 80077028  addu  v0,v0,v1
    # 8007702C  lw    v0,-0x2FD0(v0)    ; entry+0x00 worker
    # 80077030  nop
    # 80077034  jalr  v0                ; worker(arg, aux)
    # 80077038  nop
    # 8007703C  lui   v0,0x8009
    # 80077040  lw    v0,0x5878(v0)
    # 80077044  nop
    # 80077048  addiu v0,v0,1
    # 8007704C  andi  v0,v0,0x3F
    # 80077050  lui   at,0x8009
    # 80077054  sw    v0,0x5878(at)     ; consumer advances
    # 80077058  lui   v1,0x8009
    # 8007705C  lw    v1,0x5874(v1)
    # 80077060  lui   v0,0x8009
    # 80077064  lw    v0,0x5878(v0)
    # 80077068  nop
    # 8007706C  beq   v1,v0,0x80077094  ; drained -> restore
    # 80077070  nop
    # 80077074  lui   v0,0x8009
    # 80077078  lw    v0,0x5860(v0)
    # 8007707C  nop
    # 80077080  lw    v0,0(v0)          ; CHCR after issue
    # 80077084  nop
    # 80077088  and   v0,v0,s0
    # 8007708C  beqz  v0,0x80076F5C     ; still idle -> loop
    # 80077090  nop
    # 80077094  lui   a0,0x8009         ; .L80077094
    # 80077098  lw    a0,0x5880(a0)     ; saved I_MASK
    # 8007709C  jal   func_80073E10     ; RESTORE
    # 800770A0  nop
    # 800770A4  lui   v1,0x8009
    # 800770A8  lw    v1,0x5874(v1)
    # 800770AC  lui   v0,0x8009
    # 800770B0  lw    v0,0x5878(v0)
    # 800770B4  nop
    # 800770B8  bne   v1,v0,0x80077114  ; nonempty -> return
    # 800770BC  nop
    # 800770C0  lui   v0,0x8009
    # 800770C4  lw    v0,0x5860(v0)
    # 800770C8  nop
    # 800770CC  lw    v0,0(v0)          ; CHCR
    # 800770D0  lui   v1,0x0100
    # 800770D4  and   v0,v0,v1
    # 800770D8  bnez  v0,0x80077114     ; busy -> return
    # 800770DC  nop
    # 800770E0  lui   v1,0x8009
    # 800770E4  addiu v1,v1,0x5754      ; &D_80095754
    # 800770E8  lw    v0,0(v1)          ; work marker
    # 800770EC  nop
    # 800770F0  beqz  v0,0x80077114
    # 800770F4  nop
    # 800770F8  lw    a0,4(v1)          ; DrawSync callback
    # 800770FC  nop
    # 80077100  beqz  a0,0x80077114
    # 80077104  addiu v0,v1,-0x8        ; delay slot
    # 80077108  sw    zero,8(v0)        ; clear marker FIRST
    # 8007710C  jalr  a0                ; DrawSync callback()
    # 80077110  nop
    # 80077114  lui   v0,0x8009         ; .L80077114
    # 80077118  lw    v0,0x5874(v0)
    # 8007711C  lui   v1,0x8009
    # 80077120  lw    v1,0x5878(v1)
    # 80077124  nop
    # 80077128  subu  v0,v0,v1
    # 8007712C  andi  v0,v0,0x3F        ; pending count
    # 80077130  lw    ra,0x18(sp)       ; .L80077130 epilogue
    # 80077134  lw    s1,0x14(sp)
    # 80077138  lw    s0,0x10(sp)
    # 8007713C  jr    ra
    # 80077140  addiu sp,sp,0x20        ; jr delay slot

# Every direct jal to the pump, with the smallest literal window proving it.
CALL_SITES = [
    ("dispatcher-ring-full", 0x80076C78, DISPATCHER),
    ("dispatcher-post-publish", 0x80076EA4, DISPATCHER),
    ("drawsync-blocking", 0x800772B4, DRAWSYNC),
    ("drawsync-poll", 0x8007736C, DRAWSYNC),
]

# Identity materializations (registration, not calls).
IDENTITY_SITES = [0x80076D58, 0x80077A08]


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"FATAL: {message}")


def u32(value: int) -> int:
    return value & MASK


def sx16(value: int) -> int:
    value &= 0xFFFF
    return value - 0x10000 if value & 0x8000 else value


class Exe:
    def __init__(self, path: pathlib.Path):
        data = path.read_bytes()
        got = hashlib.sha1(data).hexdigest()
        require(got == EXE_SHA1, f"{path} SHA-1 {got} != {EXE_SHA1}")
        require(data[:8] == b"PS-X EXE", f"{path} is not a PS-X EXE")
        self.data = data
        self.taddr, self.tsize = struct.unpack_from("<II", data, 0x18)
        require(len(data) == 0x800 + self.tsize,
                "PS-X EXE size/header mismatch")
        self.image = data[0x800:]

    def file_offset(self, address: int) -> int:
        return 0x800 + address - self.taddr

    def body(self, address: int, size: int) -> bytes:
        offset = address - self.taddr
        require(offset >= 0 and offset + size <= len(self.image),
                f"executable range 0x{address:08X}+0x{size:X}")
        return self.image[offset:offset + size]

    def word(self, address: int) -> int:
        return struct.unpack("<I", self.body(address, 4))[0]

    def words(self):
        for index in range(0, len(self.image) - 3, 4):
            yield (self.taddr + index,
                   struct.unpack_from("<I", self.image, index)[0])


class WorkerEvent(Exception):
    """A queued worker or the DrawSync callback was invoked."""


class Machine:
    """MIPS-I executor for exactly the 152-word pump body.

    CHCR and GPUSTAT are caller-supplied inputs.  Reading them NEVER changes
    them: an unready GPUSTAT therefore makes retail's tight poll a genuine
    infinite loop, which this model reports as a step-limit event rather
    than silently making the GPU ready.
    """

    def __init__(self, exe: Exe, *, chcr: int, gpustat: int,
                 producer: int = 0, consumer: int = 0,
                 entries=None, work_marker: int = 0,
                 drawsync_callback: int = 0,
                 imask: int = 0,
                 worker_returns: int = 0,
                 worker_starts_dma: bool = False):
        self.ram = bytearray(RAM_SIZE)
        image_offset = exe.taddr - RAM_BASE
        require(0 <= image_offset <= RAM_SIZE - exe.tsize,
                "retail image does not fit modeled guest RAM")
        self.ram[image_offset:image_offset + exe.tsize] = exe.image

        self.chcr = chcr & MASK
        self.gpustat = gpustat & MASK
        self.imask = imask & 0xFFFF
        self.worker_returns = worker_returns
        self.worker_starts_dma = worker_starts_dma

        self._set32(PRODUCER, producer)
        self._set32(CONSUMER, consumer)
        self._set32(WORK_MARKER, work_marker)
        self._set32(DRAWSYNC_CALLBACK, drawsync_callback)
        for slot, (worker, argument, auxiliary) in (entries or {}).items():
            base = RING + slot * RING_STRIDE
            self._set32(base + 0x00, worker)
            self._set32(base + 0x04, argument)
            self._set32(base + 0x08, auxiliary)

        self.reg = [0] * 32
        self.reg[29] = SP_INITIAL
        self.reg[31] = RA_SENTINEL
        self.trace: list[int] = []
        self.guest_writes: list[tuple[int, int]] = []
        self.chcr_reads = 0
        self.gpustat_reads = 0
        self.imask_ops: list[tuple[str, int, int]] = []
        self.worker_calls: list[tuple[int, int, int]] = []
        self.callback_calls: list[int] = []
        self.deregister_calls: list[tuple[int, int]] = []
        self.dma_serviced = False   # must stay False, always

    # ── memory ─────────────────────────────────────────────────────────
    def _offset(self, address: int, size: int) -> int:
        require(RAM_BASE <= address and address + size <= RAM_BASE + RAM_SIZE,
                f"guest access outside modeled RAM 0x{address:08X}+{size}")
        return address - RAM_BASE

    def _set32(self, address: int, value: int) -> None:
        struct.pack_into("<I", self.ram, self._offset(address, 4),
                         value & MASK)

    def get32(self, address: int) -> int:
        return struct.unpack_from("<I", self.ram,
                                  self._offset(address, 4))[0]

    def load32(self, address: int) -> int:
        require(address & 3 == 0, f"unaligned lw 0x{address:08X}")
        if address == CHCR_MMIO:
            self.chcr_reads += 1
            return self.chcr              # inert: never auto-clears busy
        if address == GPUSTAT_MMIO:
            self.gpustat_reads += 1
            return self.gpustat           # inert: never becomes ready
        require(address < 0x1F000000 or address >= RAM_BASE,
                f"unmodeled MMIO read 0x{address:08X}")
        return self.get32(address)

    def store32(self, address: int, value: int) -> None:
        require(address & 3 == 0, f"unaligned sw 0x{address:08X}")
        require(address < 0x1F000000 or address >= RAM_BASE,
                f"unmodeled MMIO write 0x{address:08X}")
        self._set32(address, value)
        if not (SP_INITIAL - 0x40 <= address < SP_INITIAL):
            self.guest_writes.append((address, value & MASK))

    def ring_bytes(self) -> bytes:
        offset = self._offset(RING, RING_SIZE)
        return bytes(self.ram[offset:offset + RING_SIZE])

    # ── modeled callees ────────────────────────────────────────────────
    def _call(self, target: int, arguments) -> int:
        if target == 0x80073E10:                      # I_MASK exchange
            previous = self.imask
            self.imask = arguments[0] & 0xFFFF
            self.imask_ops.append(("exchange", previous, self.imask))
            return previous
        if target == 0x80073CF4:                      # callback setter
            channel, handler = arguments[0], arguments[1]
            self.deregister_calls.append((channel, handler))
            slot = CB_TABLE + 4 * channel
            previous = self.get32(slot)
            if handler != previous:
                self.store32(slot, handler)
            return previous
        raise SystemExit(f"FATAL: unmodeled direct call to 0x{target:08X}")

    def _call_indirect(self, target: int, arguments) -> int:
        if target == 0:
            raise SystemExit("FATAL: indirect call through a null identity")
        if self.reg[31] and target == self.get32(DRAWSYNC_CALLBACK):
            self.callback_calls.append(target)
            return 0
        self.worker_calls.append((target, arguments[0], arguments[1]))
        if self.worker_starts_dma:
            self.chcr |= CHCR_BUSY
        return self.worker_returns

    # ── execution ──────────────────────────────────────────────────────
    def _set(self, index: int, value: int) -> None:
        if index:
            self.reg[index] = value & MASK

    def step(self, pc: int, *, in_delay_slot: bool = False) -> int:
        self.trace.append(pc)
        word = self.load32(pc)
        op = word >> 26
        rs = (word >> 21) & 31
        rt = (word >> 16) & 31
        rd = (word >> 11) & 31
        sa = (word >> 6) & 31
        imm = word & 0xFFFF
        simm = sx16(word)

        if op == 0x0F:                                  # lui
            self._set(rt, imm << 16)
            return pc + 4
        if op == 0x09:                                  # addiu
            self._set(rt, self.reg[rs] + simm)
            return pc + 4
        if op == 0x0C:                                  # andi
            self._set(rt, self.reg[rs] & imm)
            return pc + 4
        if op == 0x23:                                  # lw
            self._set(rt, self.load32(u32(self.reg[rs] + simm)))
            return pc + 4
        if op == 0x2B:                                  # sw
            self.store32(u32(self.reg[rs] + simm), self.reg[rt])
            return pc + 4
        if op in (0x04, 0x05):                          # beq / bne
            require(not in_delay_slot, "nested control transfer")
            equal = self.reg[rs] == self.reg[rt]
            taken = equal if op == 0x04 else not equal
            target = u32(pc + 4 + (simm << 2))
            self.step(pc + 4, in_delay_slot=True)
            return target if taken else pc + 8
        if op == 0x03:                                  # jal
            require(not in_delay_slot, "nested control transfer")
            target = (u32(pc + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)
            self.step(pc + 4, in_delay_slot=True)
            arguments = tuple(self.reg[i] for i in range(4, 8))
            self.reg[31] = pc + 8
            self.reg[2] = self._call(target, arguments) & MASK
            return pc + 8
        if op == 0:
            funct = word & 0x3F
            if funct == 0x00:                           # sll / nop
                self._set(rd, u32(self.reg[rt] << sa))
                return pc + 4
            if funct == 0x21:                           # addu
                self._set(rd, self.reg[rs] + self.reg[rt])
                return pc + 4
            if funct == 0x23:                           # subu
                self._set(rd, self.reg[rs] - self.reg[rt])
                return pc + 4
            if funct == 0x24:                           # and
                self._set(rd, self.reg[rs] & self.reg[rt])
                return pc + 4
            if funct == 0x09:                           # jalr
                require(not in_delay_slot, "nested control transfer")
                target = self.reg[rs]
                self._set(rd or 31, pc + 8)
                arguments = tuple(self.reg[i] for i in range(4, 8))
                self.step(pc + 4, in_delay_slot=True)
                self.reg[2] = self._call_indirect(target, arguments) & MASK
                return pc + 8
            if funct == 0x08:                           # jr
                require(not in_delay_slot, "nested control transfer")
                target = self.reg[rs]
                self.step(pc + 4, in_delay_slot=True)
                return target
        raise SystemExit(f"FATAL: unsupported word 0x{word:08X} "
                         f"at 0x{pc:08X}")

    def run(self, limit: int = 4000):
        pc = PUMP
        for _ in range(limit):
            pc = self.step(pc)
            if pc == RA_SENTINEL:
                return self.reg[2]
        return None     # step limit: an unready GPUSTAT poll never exits


# ── static verification ────────────────────────────────────────────────
def verify_static(exe: Exe) -> None:
    body = exe.body(PUMP, PUMP_SIZE)
    require(exe.file_offset(PUMP) == PUMP_FILE_OFFSET,
            "func_80076EE4 file offset changed")
    require(PUMP_END - PUMP == PUMP_SIZE,
            "func_80076EE4 declared range is not 0x260")
    require(len(PUMP_WORDS) == PUMP_WORD_COUNT,
            "func_80076EE4 transcription is not 152 words")
    require(list(struct.unpack(f"<{PUMP_WORD_COUNT}I", body)) == PUMP_WORDS,
            "func_80076EE4 literal words differ from executable")
    require(hashlib.sha256(body).hexdigest() == PUMP_SHA256,
            "func_80076EE4 body SHA-256 mismatch")
    require(exe.word(PUMP_END) == 0x27BDFFE8,
            "exclusive end is not the func_80077144 prologue")

    # The busy fast path, proven arithmetically.
    busy_branch = PUMP_WORDS[9]
    require(busy_branch >> 26 == 0x05 and (busy_branch >> 21) & 31 == 2
            and (busy_branch >> 16) & 31 == 0,
            "0x80076F08 is not bnez v0")
    target = 0x80076F08 + 4 + (sx16(busy_branch) << 2)
    require(target == 0x80077130,
            f"busy branch target is 0x{target:08X}, not the epilogue")
    require(PUMP_WORDS[10] == 0x24020001,
            "busy branch delay slot does not set v0 = 1")
    require(PUMP_WORDS[7] == 0x3C100100,
            "busy mask is not lui s0,0x0100 (0x01000000)")
    require(PUMP_WORDS[8] == 0x00501024,
            "busy test is not and v0,v0,s0")
    require(PUMP_WORDS[6] == 0x8C420000,
            "CHCR is not read through lw v0,0(v0)")

    # Nothing before the busy branch may call, poll GPUSTAT, or touch the
    # queue.  Words 0..9 are the entire pre-branch prefix.
    for index in range(0, 10):
        word = PUMP_WORDS[index]
        require(word >> 26 not in (0x02, 0x03),
                f"pre-branch word {index} is a jump/call")
        if word >> 26 == 0:
            require(word & 0x3F not in (0x08, 0x09),
                    f"pre-branch word {index} is jr/jalr")
        if word >> 26 == 0x2B:      # sw
            base = (word >> 21) & 31
            require(base == 29,
                    f"pre-branch store at word {index} is not stack-relative")
        if word >> 26 == 0x23:      # lw
            offset = word & 0xFFFF
            require(offset in (0x5860, 0x0000),
                    f"pre-branch load at word {index} reads offset "
                    f"0x{offset:04X}")

    # Epilogue restores callee-saved registers and does not touch v0.
    require(PUMP_WORDS[147:152] == [0x8FBF0018, 0x8FB10014, 0x8FB00010,
                                    0x03E00008, 0x27BD0020],
            "func_80076EE4 epilogue changed")

    # Pointer authorities.
    require(exe.word(CHCR_POINTER) == CHCR_MMIO,
            "D_80095860 does not hold the DMA2 CHCR MMIO address")
    require(exe.word(GPUSTAT_POINTER) == GPUSTAT_MMIO,
            "D_80095854 does not hold the GPUSTAT MMIO address")

    # Deregistration and callback details of the untranslated idle path.
    require(PUMP_WORDS[43] == 0x0C01CF3D and PUMP_WORDS[42] == 0x24040002
            and PUMP_WORDS[44] == 0x00002821,
            "idle-path deregistration is not func_80073CF4(2, 0)")
    require(PUMP_WORDS[84] == 0x0040F809,
            "queued worker is not invoked through jalr v0")
    require(PUMP_WORDS[137] == 0xAC400008,
            "work marker is not cleared before the DrawSync callback")
    require(PUMP_WORDS[138] == 0x0080F809,
            "DrawSync callback is not invoked through jalr a0")

    # Exhaustive caller census.
    jal_word = 0x0C000000 | ((PUMP >> 2) & 0x03FFFFFF)
    direct = [address for address, word in exe.words() if word == jal_word]
    require(direct == [site for _, site, _ in CALL_SITES],
            f"direct call sites changed: {[hex(a) for a in direct]}")
    materializations = []
    previous = None
    for address, word in exe.words():
        if (previous is not None and previous[1] >> 26 == 0x0F
                and previous[1] & 0xFFFF == 0x8007
                and word >> 26 == 0x09 and word & 0xFFFF == 0x6EE4
                and (previous[1] >> 16) & 31 == (word >> 21) & 31):
            materializations.append(previous[0])
        previous = (address, word)
    require(materializations == IDENTITY_SITES,
            f"identity materializations changed: "
            f"{[hex(a) for a in materializations]}")

    # jtb[9] holds the pump but no executable site dispatches that field.
    require(exe.word(JUMP_TABLE + JTB_PUMP_FIELD) == PUMP,
            "jump-table slot 9 does not hold func_80076EE4")
    pointer_loads = []
    previous = None
    for address, word in exe.words():
        if (previous is not None and previous[1] >> 26 == 0x0F
                and previous[1] & 0xFFFF == 0x8009
                and word >> 26 == 0x23 and word & 0xFFFF == 0x5744
                and (previous[1] >> 16) & 31 == (word >> 21) & 31):
            pointer_loads.append((address, (word >> 16) & 31))
        previous = (address, word)
    for site, register in pointer_loads:
        for step in range(1, 25):
            word = exe.word(site + 4 * step)
            if (word >> 26 == 0x23 and (word >> 21) & 31 == register
                    and (word & 0xFFFF) == JTB_PUMP_FIELD):
                raise SystemExit(
                    f"FATAL: jump-table field +0x24 dispatched at "
                    f"0x{site + 4 * step:08X}")
            if word >> 26 == 0 and word & 0x3F == 0x08:
                break


# ── executed behaviour ─────────────────────────────────────────────────
def canonical_entries():
    return {0: (CANONICAL_WORKER, CANONICAL_ARGUMENT, CANONICAL_AUXILIARY)}


def verify_behaviour(exe: Exe) -> None:
    # ── THE canonical B53H scenario ────────────────────────────────────
    machine = Machine(exe, chcr=0x01000201, gpustat=GPUSTAT_READY,
                      producer=1, consumer=0, entries=canonical_entries(),
                      work_marker=1, drawsync_callback=0, imask=0)
    ring_before = machine.ring_bytes()
    returned = machine.run()

    require(returned == 1,
            f"canonical busy pump returned {returned}, not 1")
    require(machine.chcr_reads == 1,
            "canonical busy pump did not read CHCR exactly once")
    require(machine.gpustat_reads == 0,
            "canonical busy pump read GPUSTAT")
    require(machine.imask_ops == [],
            "canonical busy pump exchanged I_MASK")
    require(machine.worker_calls == [],
            "canonical busy pump invoked a queued worker")
    require(machine.callback_calls == [],
            "canonical busy pump invoked the DrawSync callback")
    require(machine.deregister_calls == [],
            "canonical busy pump touched callback registration")
    require(machine.guest_writes == [],
            f"canonical busy pump wrote guest state: {machine.guest_writes}")
    require(machine.get32(PRODUCER) == 1 and machine.get32(CONSUMER) == 0,
            "canonical busy pump moved a ring index")
    require(machine.ring_bytes() == ring_before,
            "canonical busy pump mutated the ring")
    require(machine.get32(CB_SLOT2) == 0,
            "canonical busy pump wrote callback slot 2")
    require(machine.get32(WORK_MARKER) == 1,
            "canonical busy pump cleared the work marker")
    require(machine.get32(SAVED_IMASK_PUMP) == 0,
            "canonical busy pump wrote its saved-I_MASK word")
    require(machine.chcr == 0x01000201 and not machine.dma_serviced,
            "canonical busy pump changed DMA state")
    # It returns through the shared epilogue without touching the body.
    require(0x80076F10 not in machine.trace,
            "canonical busy pump executed past the busy branch")
    require(machine.trace[-5:] == [0x80077130, 0x80077134, 0x80077138,
                                   0x8007713C, 0x80077140],
            "canonical busy pump did not exit through the shared epilogue")

    # Repeating it is deterministic and still inert.
    for _ in range(3):
        again = Machine(exe, chcr=0x01000201, gpustat=GPUSTAT_READY,
                        producer=1, consumer=0, entries=canonical_entries(),
                        work_marker=1, imask=0)
        require(again.run() == 1 and again.guest_writes == []
                and again.ring_bytes() == ring_before,
                "repeated canonical busy pump is not deterministic")

    # ── busy with an EMPTY queue still returns 1 without any read ──────
    machine = Machine(exe, chcr=CHCR_BUSY, gpustat=0,
                      producer=0, consumer=0)
    require(machine.run() == 1 and machine.guest_writes == []
            and machine.gpustat_reads == 0 and machine.imask_ops == [],
            "busy fast path depends on queue or GPU state")

    # Any CHCR value with bit 24 set takes the fast path.
    for chcr in (0x01000000, 0x01000201, 0xFFFFFFFF, 0x81000401):
        probe = Machine(exe, chcr=chcr, gpustat=0, producer=5, consumer=1)
        require(probe.run() == 1 and probe.guest_writes == [],
                f"CHCR 0x{chcr:08X} did not take the busy fast path")
    # ...and no value with bit 24 clear does.
    for chcr in (0x00000000, 0x00000401, 0x00000201, 0xFEFFFFFF):
        probe = Machine(exe, chcr=chcr, gpustat=GPUSTAT_READY,
                        producer=0, consumer=0)
        require(probe.run() == 0,
                f"CHCR 0x{chcr:08X} wrongly took the busy fast path")

    # ── idle + empty queue: exchange, restore, return 0 ────────────────
    machine = Machine(exe, chcr=0x00000401, gpustat=GPUSTAT_READY,
                      producer=3, consumer=3, work_marker=0, imask=0xBEEF)
    require(machine.run() == 0, "idle empty pump did not return 0")
    require([kind for kind, _, _ in machine.imask_ops] ==
            ["exchange", "exchange"],
            "idle empty pump did not exchange and restore I_MASK")
    require(machine.imask_ops[0][1] == 0xBEEF
            and machine.imask_ops[0][2] == 0
            and machine.imask_ops[1][2] == 0xBEEF,
            "idle empty pump lost the exact I_MASK value")
    require(machine.get32(SAVED_IMASK_PUMP) == 0xBEEF,
            "idle empty pump did not save the previous mask")
    require(machine.worker_calls == [] and machine.callback_calls == [],
            "idle empty pump invoked something")

    # ── idle + one entry: issue exactly once, consumer advances ────────
    machine = Machine(exe, chcr=0x00000401, gpustat=GPUSTAT_READY,
                      producer=1, consumer=0, entries=canonical_entries(),
                      work_marker=1, drawsync_callback=0, imask=0x1234,
                      worker_starts_dma=True)
    returned = machine.run()
    require(machine.worker_calls ==
            [(CANONICAL_WORKER, CANONICAL_ARGUMENT, CANONICAL_AUXILIARY)],
            f"idle pump worker calls wrong: {machine.worker_calls}")
    require(machine.get32(CONSUMER) == 1,
            "idle pump did not advance the consumer after issue")
    require(returned == 0, "drained pump did not return zero pending")
    # Last entry with no DrawSync callback deregisters slot 2 first.
    require(machine.deregister_calls == [(2, 0)],
            f"idle pump deregistration wrong: {machine.deregister_calls}")
    require([kind for kind, _, _ in machine.imask_ops] ==
            ["exchange", "exchange"]
            and machine.imask_ops[1][2] == 0x1234,
            "idle pump lost the I_MASK restore around issue")
    require(machine.callback_calls == [],
            "idle pump fired a DrawSync callback that was not installed")

    # With a DrawSync callback installed the last entry keeps slot 2.
    machine = Machine(exe, chcr=0x00000401, gpustat=GPUSTAT_READY,
                      producer=1, consumer=0, entries=canonical_entries(),
                      work_marker=1, drawsync_callback=0x80012345,
                      imask=0, worker_starts_dma=True)
    machine.run()
    require(machine.deregister_calls == [],
            "installed DrawSync callback did not keep slot 2 registered")

    # ── drained + idle + marker + callback: clear marker, then fire ────
    machine = Machine(exe, chcr=0x00000401, gpustat=GPUSTAT_READY,
                      producer=2, consumer=2, work_marker=1,
                      drawsync_callback=0x80012345, imask=0)
    require(machine.run() == 0, "completion path did not return 0")
    require(machine.callback_calls == [0x80012345],
            "DrawSync callback was not invoked exactly once")
    marker_index = next(i for i, (a, _) in enumerate(machine.guest_writes)
                        if a == WORK_MARKER)
    require(machine.get32(WORK_MARKER) == 0,
            "work marker was not cleared")
    require(marker_index is not None,
            "work marker clear was not observed as a guest write")

    # A still-busy channel suppresses the completion callback entirely.
    machine = Machine(exe, chcr=0x00000401, gpustat=GPUSTAT_READY,
                      producer=1, consumer=0, entries=canonical_entries(),
                      work_marker=1, drawsync_callback=0x80012345,
                      imask=0, worker_starts_dma=True)
    machine.run()
    require(machine.callback_calls == [],
            "DrawSync callback fired while DMA was still busy")

    # ── producer/consumer wrap ─────────────────────────────────────────
    machine = Machine(exe, chcr=0x00000401, gpustat=GPUSTAT_READY,
                      producer=0, consumer=63,
                      entries={63: (0x80076664, 0x800B0000, 0x800B1000)},
                      work_marker=1, imask=0, worker_starts_dma=True)
    machine.run()
    require(machine.worker_calls ==
            [(0x80076664, 0x800B0000, 0x800B1000)],
            "wrap issued the wrong entry")
    require(machine.get32(CONSUMER) == 0,
            "consumer did not wrap 63 -> 0")

    # ── GPUSTAT not ready: retail tight-polls forever ──────────────────
    machine = Machine(exe, chcr=0x00000401, gpustat=0,
                      producer=1, consumer=0, entries=canonical_entries(),
                      work_marker=1, imask=0)
    require(machine.run() is None,
            "unready GPUSTAT did not produce retail's unbounded poll")
    require(machine.worker_calls == [] and machine.get32(CONSUMER) == 0,
            "unready GPUSTAT still issued work or advanced the consumer")
    require(machine.gpustat_reads > 1,
            "unready GPUSTAT was not polled repeatedly")

    # The model never completes DMA on its own, in any scenario above.
    require(not machine.dma_serviced, "the oracle model serviced DMA")


def main(argv: list[str]) -> int:
    default = pathlib.Path(__file__).resolve().parents[2] / \
        "build" / "disc1.candidate.exe"
    path = pathlib.Path(argv[1]) if len(argv) > 1 else default
    exe = Exe(path)
    verify_static(exe)
    verify_behaviour(exe)
    print(f"B53H oracle: PASS  ({path})")
    print(f"  func_80076EE4  0x{PUMP:08X}..0x{PUMP_END - 1:08X} "
          f"(exclusive end 0x{PUMP_END:08X}), "
          f"0x{PUMP_SIZE:X} bytes, {PUMP_WORD_COUNT} words")
    print("  ABI            int func_80076EE4(void)")
    print("  busy test      *(0x1F8010A8) & 0x01000000 -> return 1")
    print("  callers        4 direct; jtb[9] holds it but is never dispatched")
    print("  canonical      producer=1 consumer=0 CHCR=0x01000201 -> 1, "
          "zero guest writes")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
