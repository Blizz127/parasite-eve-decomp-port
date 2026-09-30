#!/usr/bin/env python3
"""Phase 6E-B53E literal oracle for LoadImage worker func_80076664.

This standalone program executes the exact 143-word retail MIPS-I body.  It
does not import production C.  GPUSTAT reads, GPU/DMAC writes, VSync, and the
untranslated func_80077404 dependency are finite, explicit scenario scripts;
reading status never changes it and no DMA completion is modeled.

The retail worker itself does not validate its source span.  Separate safety
vectors below prove the native preflight envelope required before executing
the first GP0 mutation or CPU-prefix load.  Literal fault scenarios remain
faults and are never converted into invented retail return values.
"""

from __future__ import annotations

from collections import deque
from dataclasses import dataclass
import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_words


EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
WORKER_SHA256 = (
    "79dd44e3819f51eb5928c9af3ec0d6906cc3d95765dc2718c3c10e49ef78e0f7"
)
WAIT_INIT_SHA256 = (
    "3b0637e8b6472855bf28115f4712ea1f43c3d3ad130c388ab71c8101d3d00cb5"
)

MASK = 0xFFFFFFFF
RAM_BASE = 0x80000000
RAM_SIZE = 0x00200000
RAM_END = RAM_BASE + RAM_SIZE
SP = 0x801FFF00
RA_SENTINEL = 0xDEADBEEC

WORKER = 0x80076664
WORKER_END = 0x800768A0
WAIT_INIT = 0x800773D0
WAIT_INIT_END = 0x80077404
WAIT_TIMEOUT = 0x80077404
VSYNC = 0x80073A44

LIMIT_W = 0x80095750
LIMIT_H = 0x80095752
GP0_POINTER = 0x80095850
GPUSTAT_POINTER = 0x80095854
DMA2_MADR_POINTER = 0x80095858
DMA2_BCR_POINTER = 0x8009585C
DMA2_CHCR_POINTER = 0x80095860
WAIT_DEADLINE = 0x80095888
WAIT_COUNTER = 0x8009588C

GP0 = 0x1F801810
GPUSTAT_GP1 = 0x1F801814
DMA2_MADR = 0x1F8010A0
DMA2_BCR = 0x1F8010A4
DMA2_CHCR = 0x1F8010A8
GPUSTAT_READY_GP0 = 0x04000000

RECT = 0x801E0000


# Literal constants, never extracted expectations.  verify_static compares
# every byte against the independently opened SHA-exact executable.
WORKER_WORDS = exe_words(0x80076664, 143)

WAIT_INIT_WORDS = exe_words(0x800773D0, 13)


# PC, decoded kind, target (None for register return), exact delay word.
WORKER_CONTROL = [
    (0x80076688, "jal", 0x800773D0, 0xAFB00010),
    (0x80076698, "bltz", 0x800766C8, 0x0000A821),
    (0x800766B8, "beq", 0x800766CC, 0x00000000),
    (0x800766C0, "j", 0x800766CC, 0x00602021),
    (0x800766D4, "bltz", 0x80076704, 0xA6240004),
    (0x800766F4, "beq", 0x8007670C, 0x00041400),
    (0x800766FC, "j", 0x80076708, 0x00602021),
    (0x80076730, "bgtz", 0x80076740, 0x00038143),
    (0x80076738, "j", 0x8007687C, 0x2402FFFF),
    (0x80076764, "bne", 0x800767A0, 0x3C04A000),
    (0x80076770, "jal", 0x80077404, 0x00000000),
    (0x80076778, "bne", 0x8007687C, 0x2402FFFF),
    (0x80076798, "beq", 0x80076770, 0x3C04A000),
    (0x800767C8, "beq", 0x800767D4, 0x00000000),
    (0x80076804, "beq", 0x8007682C, 0x00000000),
    (0x80076824, "bne", 0x80076810, 0xAC430000),
    (0x8007682C, "beq", 0x80076878, 0x3C030400),
    (0x80076898, "jr", None, 0x27BD0030),
]

WAIT_INIT_CONTROL = [
    (0x800773D8, "jal", 0x80073A44, 0x2404FFFF),
    (0x800773FC, "jr", None, 0x00000000),
]


def u32(value: int) -> int:
    return value & MASK


def s32(value: int) -> int:
    value &= MASK
    return value - 0x100000000 if value & 0x80000000 else value


def sx16(value: int) -> int:
    value &= 0xFFFF
    return value - 0x10000 if value & 0x8000 else value


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"FATAL: {message}")


def words_bytes(words: list[int]) -> bytes:
    return b"".join(struct.pack("<I", word) for word in words)


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

    def body(self, address: int, size: int) -> bytes:
        offset = address - self.taddr
        require(offset >= 0 and offset + size <= len(self.image),
                f"executable range 0x{address:08X}+0x{size:X}")
        return self.image[offset:offset + size]

    def word(self, address: int) -> int:
        return struct.unpack("<I", self.body(address, 4))[0]


@dataclass(frozen=True)
class HardwareEvent:
    kind: str
    pc: int
    address: int
    value: int


@dataclass(frozen=True)
class CalleeEvent:
    pc: int
    target: int
    args: tuple[int, ...]
    result: int
    writes: tuple[tuple[int, int, int], ...] = ()


@dataclass(frozen=True)
class Call:
    pc: int
    target: int
    args: tuple[int, int, int, int]


@dataclass(frozen=True)
class Control:
    kind: str
    target: int | None


class DependencyStop(Exception):
    def __init__(self, call: Call):
        super().__init__(f"dependency 0x{call.target:08X}")
        self.call = call


class HardwareStop(Exception):
    def __init__(self, event: HardwareEvent):
        super().__init__(f"hardware event {event.kind} 0x{event.address:08X}")
        self.event = event


class GuestFault(Exception):
    def __init__(self, pc: int, address: int, size: int, reason: str):
        super().__init__(
            f"guest fault @{pc:08X}: 0x{address:08X}+{size} {reason}"
        )
        self.pc = pc
        self.address = address
        self.size = size
        self.reason = reason


class Machine:
    """Small MIPS-I machine with explicit branch and load delay semantics."""

    def __init__(self, exe: Exe, hardware: list[HardwareEvent],
                 callees: list[CalleeEvent],
                 stop_at: set[int] | None = None):
        self.ram = bytearray(RAM_SIZE)
        image_offset = exe.taddr - RAM_BASE
        require(image_offset >= 0 and image_offset + exe.tsize <= RAM_SIZE,
                "executable does not fit guest RAM")
        self.ram[image_offset:image_offset + exe.tsize] = exe.image
        self.hardware = deque(hardware)
        self.callees = deque(callees)
        self.stop_at = set(stop_at or ())

        self.regs = [0] * 32
        self.regs[16:22] = [
            0x16161616, 0x17171717, 0x18181818,
            0x19191919, 0x1A1A1A1A, 0x1B1B1B1B,
        ]
        self.initial_saved = tuple(self.regs[16:22])
        self.regs[29] = SP
        self.regs[31] = RA_SENTINEL
        self.lo = 0
        self.pending_load: tuple[int, int, int] | None = None

        self.calls: list[Call] = []
        self.hardware_observed: list[HardwareEvent] = []
        self.memory_reads: list[tuple[int, int, int, int]] = []
        self.retail_writes: list[tuple[int, int, int, int]] = []
        self.controlled_writes: list[tuple[int, int, int]] = []
        self.regular_writes: list[tuple[int, int, int]] = []
        self.load_commits: list[tuple[int, int, int]] = []
        self.lo_writes: list[tuple[int, int]] = []
        self.pc_counts: dict[int, int] = {}
        self.current_pc = 0
        self.steps = 0

    @staticmethod
    def offset(address: int, size: int, pc: int = 0,
               aligned: bool = False) -> int:
        if aligned and address & (size - 1):
            raise GuestFault(pc, address, size, "unaligned")
        offset = address - RAM_BASE
        if offset < 0 or offset + size > RAM_SIZE:
            raise GuestFault(pc, address, size, "outside modeled RAM")
        return offset

    def seed(self, address: int, size: int, value: int) -> None:
        offset = self.offset(address, size)
        mask = (1 << (size * 8)) - 1
        self.ram[offset:offset + size] = (value & mask).to_bytes(size, "little")

    def seed_words(self, address: int, values: list[int]) -> None:
        for index, value in enumerate(values):
            self.seed(address + index * 4, 4, value)

    def peek(self, address: int, size: int) -> int:
        offset = self.offset(address, size)
        return int.from_bytes(self.ram[offset:offset + size], "little")

    def fetch(self, address: int) -> int:
        return self.peek(address, 4)

    def consume_hardware(self, kind: str, address: int,
                         value: int | None = None) -> int:
        request = HardwareEvent(kind, self.current_pc, address,
                                0 if value is None else u32(value))
        if not self.hardware:
            raise HardwareStop(request)
        expected = self.hardware.popleft()
        require(expected.kind == kind and expected.pc == self.current_pc and
                expected.address == address,
                f"hardware event {request} != script {expected}")
        if kind == "write":
            require(expected.value == u32(value or 0),
                    f"hardware write 0x{u32(value or 0):08X} != "
                    f"0x{expected.value:08X} @0x{self.current_pc:08X}")
        self.hardware_observed.append(expected)
        return expected.value

    def load(self, address: int, size: int, signed: bool = False) -> int:
        if address == GPUSTAT_GP1 and size == 4:
            return self.consume_hardware("read", address)
        offset = self.offset(address, size, self.current_pc, aligned=True)
        value = int.from_bytes(self.ram[offset:offset + size], "little")
        self.memory_reads.append((self.current_pc, address, size, value))
        if signed:
            sign = 1 << (size * 8 - 1)
            if value & sign:
                value -= 1 << (size * 8)
        return u32(value)

    def store(self, address: int, size: int, value: int) -> None:
        if address in (GP0, GPUSTAT_GP1, DMA2_MADR, DMA2_BCR, DMA2_CHCR):
            require(size == 4, f"non-word MMIO store @0x{self.current_pc:08X}")
            self.consume_hardware("write", address, value)
            return
        offset = self.offset(address, size, self.current_pc, aligned=True)
        mask = (1 << (size * 8)) - 1
        value &= mask
        self.ram[offset:offset + size] = value.to_bytes(size, "little")
        self.retail_writes.append((self.current_pc, address, size, value))

    def controlled_store(self, address: int, size: int, value: int) -> None:
        self.seed(address, size, value)
        self.controlled_writes.append((address, size, value & MASK))

    @staticmethod
    def is_control(word: int) -> bool:
        op = (word >> 26) & 0x3F
        fn = word & 0x3F
        return op in (1, 2, 3, 4, 5, 7) or (
            op == 0 and fn in (8, 9)
        )

    def execute(self, pc: int) -> Control | None:
        """Execute one instruction, then retire the prior delayed load.

        All operands and store addresses are captured before the old load is
        committed.  Current non-load writes retire after that commit, so an
        instruction immediately following a load still observes the old
        value and wins a same-register write.  The current load is queued for
        retirement after the next instruction.
        """
        self.current_pc = pc
        self.steps += 1
        self.pc_counts[pc] = self.pc_counts.get(pc, 0) + 1
        word = self.fetch(pc)
        op = (word >> 26) & 0x3F
        rs = (word >> 21) & 31
        rt = (word >> 16) & 31
        rd = (word >> 11) & 31
        shift = (word >> 6) & 31
        immediate = word & 0xFFFF
        simm = sx16(immediate)
        before = tuple(self.regs)
        writes: list[tuple[int, int]] = []
        new_load: tuple[int, int, int] | None = None
        control: Control | None = None

        def write(reg: int, value: int) -> None:
            if reg != 0:
                writes.append((reg, u32(value)))

        if word == 0:
            pass
        elif op == 0:
            fn = word & 0x3F
            if fn == 0x00:             # sll
                write(rd, before[rt] << shift)
            elif fn == 0x02:           # srl
                write(rd, before[rt] >> shift)
            elif fn == 0x03:           # sra
                write(rd, s32(before[rt]) >> shift)
            elif fn == 0x08:           # jr
                control = Control("jr", before[rs])
            elif fn == 0x09:           # jalr (not present in worker)
                write(rd, pc + 8)
                control = Control("jalr", before[rs])
            elif fn == 0x12:           # mflo
                write(rd, self.lo)
            elif fn == 0x18:           # signed mult
                self.lo = u32(s32(before[rs]) * s32(before[rt]))
                self.lo_writes.append((pc, self.lo))
            elif fn == 0x21:           # addu
                write(rd, before[rs] + before[rt])
            elif fn == 0x23:           # subu
                write(rd, before[rs] - before[rt])
            elif fn == 0x24:           # and
                write(rd, before[rs] & before[rt])
            elif fn == 0x2A:           # slt
                write(rd, int(s32(before[rs]) < s32(before[rt])))
            else:
                raise SystemExit(f"FATAL: SPECIAL {fn:02X} @0x{pc:08X}")
        elif op == 1:                  # bltz
            require(rt == 0, f"REGIMM rt={rt} @0x{pc:08X}")
            target = u32(pc + 4 + (simm << 2))
            control = Control("bltz", target if s32(before[rs]) < 0 else None)
        elif op == 2:                  # j
            target = ((pc + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)
            control = Control("j", target)
        elif op == 3:                  # jal
            write(31, pc + 8)
            target = ((pc + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)
            control = Control("jal", target)
        elif op == 4:                  # beq
            target = u32(pc + 4 + (simm << 2))
            control = Control(
                "beq", target if before[rs] == before[rt] else None
            )
        elif op == 5:                  # bne
            target = u32(pc + 4 + (simm << 2))
            control = Control(
                "bne", target if before[rs] != before[rt] else None
            )
        elif op == 7:                  # bgtz
            target = u32(pc + 4 + (simm << 2))
            control = Control("bgtz", target if s32(before[rs]) > 0 else None)
        elif op == 9:                  # addiu
            write(rt, before[rs] + simm)
        elif op == 0x0D:               # ori
            write(rt, before[rs] | immediate)
        elif op == 0x0F:               # lui
            write(rt, immediate << 16)
        elif op in (0x21, 0x23, 0x25): # lh, lw, lhu
            size = 4 if op == 0x23 else 2
            value = self.load(u32(before[rs] + simm), size, op == 0x21)
            new_load = (rt, value, pc)
        elif op == 0x29:               # sh
            self.store(u32(before[rs] + simm), 2, before[rt])
        elif op == 0x2B:               # sw
            self.store(u32(before[rs] + simm), 4, before[rt])
        else:
            raise SystemExit(f"FATAL: opcode {op:02X} @0x{pc:08X}")

        if self.pending_load is not None:
            reg, value, load_pc = self.pending_load
            if reg != 0:
                self.regs[reg] = value
                self.load_commits.append((load_pc, reg, value))
        for reg, value in writes:
            self.regs[reg] = value
            self.regular_writes.append((pc, reg, value))
        self.regs[0] = 0
        self.pending_load = new_load
        return control

    def external_call(self, call: Call, resume_pc: int) -> int:
        if call.target in self.stop_at:
            raise DependencyStop(call)
        require(self.callees, f"uncontrolled callee 0x{call.target:08X}")
        expected = self.callees.popleft()
        require(expected.pc == call.pc and expected.target == call.target,
                f"callee {call} != script {expected}")
        require(call.args[:len(expected.args)] == expected.args,
                f"callee args {call.args} do not start with {expected.args}")
        for address, size, value in expected.writes:
            self.controlled_store(address, size, value)
        self.regs[2] = u32(expected.result)
        return resume_pc

    def run(self, rect: int, source: int, max_steps: int = 100000, *, entry: int = WORKER) -> int:
        self.regs[4] = u32(rect)
        self.regs[5] = u32(source)
        pc = entry

        while self.steps < max_steps:
            word = self.fetch(pc)
            if not self.is_control(word):
                require(self.execute(pc) is None,
                        f"unexpected control decode @0x{pc:08X}")
                pc = u32(pc + 4)
                continue

            control = self.execute(pc)
            require(control is not None, f"missing control @0x{pc:08X}")
            delay_pc = u32(pc + 4)
            require(not self.is_control(self.fetch(delay_pc)),
                    f"control in delay slot @0x{delay_pc:08X}")
            require(self.execute(delay_pc) is None,
                    f"delay control @0x{delay_pc:08X}")

            if control.kind in ("jal", "jalr"):
                require(control.target is not None,
                        f"null call target @0x{pc:08X}")
                call = Call(pc, control.target,
                            tuple(self.regs[index] for index in range(4, 8)))
                self.calls.append(call)
                if call.target == WAIT_INIT:
                    pc = WAIT_INIT
                else:
                    pc = self.external_call(call, u32(pc + 8))
                continue

            if control.kind == "jr":
                require(control.target is not None,
                        f"null jr target @0x{pc:08X}")
                if control.target == RA_SENTINEL:
                    require(self.pending_load is None,
                            "pending load survived worker return")
                    require(self.regs[29] == SP, "stack pointer not restored")
                    require(tuple(self.regs[16:22]) == self.initial_saved,
                            "callee-saved registers not restored")
                    return self.regs[2]
                pc = control.target
                continue

            pc = u32(pc + 8) if control.target is None else control.target

        raise SystemExit("FATAL: interpreter step limit")

    def assert_scripts_consumed(self) -> None:
        require(not self.hardware,
                f"unused hardware events: {list(self.hardware)}")
        require(not self.callees,
                f"unused callee events: {list(self.callees)}")


def decode_control(pc: int, word: int) -> tuple[str, int | None] | None:
    op = (word >> 26) & 0x3F
    fn = word & 0x3F
    simm = sx16(word)
    if op == 1 and ((word >> 16) & 31) == 0:
        return "bltz", u32(pc + 4 + (simm << 2))
    if op == 2:
        return "j", ((pc + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)
    if op == 3:
        return "jal", ((pc + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)
    if op == 4:
        return "beq", u32(pc + 4 + (simm << 2))
    if op == 5:
        return "bne", u32(pc + 4 + (simm << 2))
    if op == 7:
        return "bgtz", u32(pc + 4 + (simm << 2))
    if op == 0 and fn == 8:
        return "jr", None
    if op == 0 and fn == 9:
        return "jalr", None
    return None


def control_fingerprint(base: int, words: list[int]) -> list[tuple]:
    result = []
    for index, word in enumerate(words):
        decoded = decode_control(base + index * 4, word)
        if decoded is not None:
            require(index + 1 < len(words), "control lacks delay slot")
            kind, target = decoded
            result.append((base + index * 4, kind, target, words[index + 1]))
    return result


def verify_static(exe: Exe) -> None:
    require(len(WORKER_WORDS) == 143, "worker literal count is not 143")
    require(WORKER_END - WORKER == 0x23C, "worker range is not 0x23C")
    body = exe.body(WORKER, WORKER_END - WORKER)
    require(words_bytes(WORKER_WORDS) == body,
            "one or more of 143 worker words differ from executable")
    require(hashlib.sha256(body).hexdigest() == WORKER_SHA256,
            "worker body SHA-256 mismatch")
    require(exe.word(WORKER_END) == 0x27BDFFD8,
            "next word is not func_800768A0 prologue")
    require(control_fingerprint(WORKER, WORKER_WORDS) == WORKER_CONTROL,
            "worker control/delay fingerprint mismatch")

    wait_body = exe.body(WAIT_INIT, WAIT_INIT_END - WAIT_INIT)
    require(words_bytes(WAIT_INIT_WORDS) == wait_body,
            "wait-init literal differs from executable")
    require(hashlib.sha256(wait_body).hexdigest() == WAIT_INIT_SHA256,
            "wait-init body SHA-256 mismatch")
    require(control_fingerprint(WAIT_INIT, WAIT_INIT_WORDS) ==
            WAIT_INIT_CONTROL, "wait-init control fingerprint mismatch")

    pointers = {
        GP0_POINTER: GP0,
        GPUSTAT_POINTER: GPUSTAT_GP1,
        DMA2_MADR_POINTER: DMA2_MADR,
        DMA2_BCR_POINTER: DMA2_BCR,
        DMA2_CHCR_POINTER: DMA2_CHCR,
    }
    for address, expected in pointers.items():
        require(exe.word(address) == expected,
                f"retail pointer 0x{address:08X} changed")

    aligned_words = list(struct.iter_unpack("<I", exe.image))
    literal_sites = [exe.taddr + index * 4
                     for index, (word,) in enumerate(aligned_words)
                     if word == WORKER]
    direct_jal = 0x0C000000 | ((WORKER >> 2) & 0x03FFFFFF)
    direct_sites = [exe.taddr + index * 4
                    for index, (word,) in enumerate(aligned_words)
                    if word == direct_jal]
    require(literal_sites == [0x80095724],
            f"worker literal sites changed: {literal_sites}")
    require(not direct_sites, f"unexpected direct-JAL sites: {direct_sites}")

    caller_words = {
        0x80076D34: 0x02002021, 0x80076D38: 0x0260F809,
        0x80076D3C: 0x02402821,
        0x80077020: 0x8CA5D038, 0x8007702C: 0x8C42D030,
        0x80077034: 0x0040F809, 0x80077038: 0x00000000,
        0x800776A8: 0x02002021, 0x800776AC: 0x8C420020,
        0x800776B4: 0x0040F809, 0x800776B8: 0x02202821,
        0x800750A4: 0x8C440020, 0x800750A8: 0x8C420008,
        0x800750B0: 0x0040F809, 0x800750B4: 0x02203821,
    }
    for address, expected in caller_words.items():
        require(exe.word(address) == expected,
                f"caller fingerprint changed @0x{address:08X}")

    print(f"SHA-1 verified: {EXE_SHA1}")
    print("func_80076664: 143 literal words EXACT, "
          "0x80076664..0x800768A0, file 0x66E64")
    print(f"body SHA-256 verified: {WORKER_SHA256}")
    print("control flow verified: 18 transfers with exact delay slots")
    print("caller fingerprint verified: dispatcher, pump, LoadImage2; "
          "jtb[8] sole literal")


SCENARIO_COUNT = 0


def passed(name: str, detail: str) -> None:
    global SCENARIO_COUNT
    SCENARIO_COUNT += 1
    print(f"scenario {name}: PASS {detail}")


def read_status(pc: int, value: int) -> HardwareEvent:
    return HardwareEvent("read", pc, GPUSTAT_GP1, u32(value))


def hw_write(pc: int, address: int, value: int) -> HardwareEvent:
    return HardwareEvent("write", pc, address, u32(value))


def issue_hardware(statuses: list[int], rect_word0: int, rect_word1: int,
                   prefix_words: list[int], blocks: int,
                   source_after_prefix: int) -> list[HardwareEvent]:
    require(statuses and statuses[-1] & GPUSTAT_READY_GP0,
            "successful issue script must end ready")
    events = [read_status(0x80076758, statuses[0])]
    events.extend(read_status(0x8007678C, value)
                  for value in statuses[1:])
    events.extend([
        hw_write(0x800767AC, GPUSTAT_GP1, 0x04000000),
        hw_write(0x800767BC, GP0, 0x01000000),
        hw_write(0x800767D4, GP0, 0xA0000000),
        hw_write(0x800767E8, GP0, rect_word0),
        hw_write(0x800767FC, GP0, rect_word1),
    ])
    events.extend(hw_write(0x80076828, GP0, value)
                  for value in prefix_words)
    if blocks:
        events.extend([
            hw_write(0x80076840, GPUSTAT_GP1, 0x04000002),
            hw_write(0x80076850, DMA2_MADR, source_after_prefix),
            hw_write(0x80076864, DMA2_BCR,
                     u32((blocks << 16) | 0x10)),
            hw_write(0x80076874, DMA2_CHCR, 0x01000201),
        ])
    return events


def callee_script(vsync: int,
                  timeout_results: list[int] | None = None,
                  update_counter: bool = False) -> list[CalleeEvent]:
    events = [CalleeEvent(0x800773D8, VSYNC, (MASK,), u32(vsync))]
    for index, result in enumerate(timeout_results or []):
        writes = (((WAIT_COUNTER, 4, index + 1),)
                  if update_counter and result == 0 else ())
        events.append(CalleeEvent(
            0x80076770, WAIT_TIMEOUT, (0xA0000000,), u32(result), writes
        ))
    return events


def seed_rect(machine: Machine, x: int, y: int, w: int, h: int,
              address: int = RECT) -> None:
    machine.seed(address + 0, 2, x)
    machine.seed(address + 2, 2, y)
    machine.seed(address + 4, 2, w)
    machine.seed(address + 6, 2, h)


def make_machine(exe: Exe, *, rect: tuple[int, int, int, int],
                 hardware: list[HardwareEvent], vsync: int = 17,
                 timeout_results: list[int] | None = None,
                 update_counter: bool = False,
                 width_limit: int = 1024, height_limit: int = 512,
                 source: int | None = None,
                 source_words: list[int] | None = None,
                 stop_at: set[int] | None = None) -> Machine:
    machine = Machine(
        exe, hardware,
        callee_script(vsync, timeout_results, update_counter), stop_at
    )
    machine.seed(LIMIT_W, 2, width_limit)
    machine.seed(LIMIT_H, 2, height_limit)
    machine.seed(WAIT_DEADLINE, 4, 0xDDDDDDDD)
    machine.seed(WAIT_COUNTER, 4, 0xEEEEEEEE)
    seed_rect(machine, *rect)
    if source is not None and source_words:
        machine.seed_words(source, source_words)
    return machine


def rect_words(machine: Machine, address: int = RECT) -> tuple[int, int]:
    return machine.peek(address, 4), machine.peek(address + 4, 4)


def calls(machine: Machine) -> list[int]:
    return [call.target for call in machine.calls]


def source_reads(machine: Machine) -> list[tuple[int, int]]:
    return [(address, value) for pc, address, size, value in machine.memory_reads
            if pc == 0x80076810 and size == 4]


def regular_value(machine: Machine, pc: int, reg: int) -> int:
    values = [value for write_pc, write_reg, value in machine.regular_writes
              if write_pc == pc and write_reg == reg]
    require(len(values) == 1,
            f"regular write @0x{pc:08X} r{reg} count {len(values)}")
    return values[0]


def assert_arithmetic(machine: Machine, *, product: int, plus_one: int,
                      sign: int, adjusted: int, words: int,
                      blocks: int) -> None:
    require(machine.lo_writes == [(0x80076714, u32(product))],
            f"MULT/LO trace {machine.lo_writes}")
    expected = [
        (0x8007671C, 6, product),
        (0x80076720, 3, plus_one),
        (0x80076724, 2, sign),
        (0x80076728, 3, adjusted),
        (0x8007672C, 4, words),
        (0x80076734, 16, blocks),
    ]
    for pc, reg, value in expected:
        require(regular_value(machine, pc, reg) == u32(value),
                f"arithmetic write @0x{pc:08X} r{reg}")


def assert_wait_init(machine: Machine, vsync: int,
                     counter: int = 0) -> None:
    require(machine.peek(WAIT_DEADLINE, 4) == u32(vsync + 240),
            "deadline is not explicit VSync+240")
    require(machine.peek(WAIT_COUNTER, 4) == u32(counter),
            f"wait counter {machine.peek(WAIT_COUNTER, 4)} != {counter}")
    require(calls(machine)[:2] == [WAIT_INIT, VSYNC],
            f"wait-init call prefix {calls(machine)}")
    query = machine.calls[1]
    require(query.pc == 0x800773D8 and query.args[0] == MASK,
            "VSync jal delay-slot ABI mismatch")
    require(machine.pc_counts.get(0x800773DC) == 1,
            "VSync delay slot did not execute exactly once")


def assert_rect_clamp(machine: Machine, original_xy: int,
                      width_bits: int, height_bits: int) -> None:
    word0, word1 = rect_words(machine)
    require(word0 == u32(original_xy), "x/y were unexpectedly mutated")
    require(word1 == ((height_bits & 0xFFFF) << 16 |
                      (width_bits & 0xFFFF)),
            f"clamped w/h word 0x{word1:08X}")
    rect_stores = [(pc, address, size, value)
                   for pc, address, size, value in machine.retail_writes
                   if RECT <= address < RECT + 8]
    require(rect_stores == [
        (0x800766D8, RECT + 4, 2, width_bits & 0xFFFF),
        (0x80076718, RECT + 6, 2, height_bits & 0xFFFF),
    ], f"RECT store order {rect_stores}")


def assert_no_issue(machine: Machine) -> None:
    writes = [event for event in machine.hardware_observed
              if event.kind == "write"]
    require(not writes, f"unexpected GPU/DMA writes: {writes}")
    require(not source_reads(machine),
            f"unexpected source reads: {source_reads(machine)}")


def source_span_safe(source: int, transfer_words: int) -> bool:
    """Independent native safety contract, not a retail return path."""
    source = u32(source)
    if source == 0 or source & 3 or transfer_words <= 0:
        return False
    byte_count = transfer_words * 4
    if byte_count > MASK:
        return False
    end = source + byte_count
    return source >= RAM_BASE and end <= RAM_END and end <= 0x100000000


def scenario_nonpositive_and_signed_limits(exe: Exe) -> None:
    cases = [
        # name, rect, limits, final w/h, p,t,sign,adjusted,words,blocks
        ("negative-width", (0x1234, 0x5678, -1, 5), (1024, 512),
         (0, 5), 0, 1, 0, 1, 0, 0),
        ("negative-height", (0x1234, 0x5678, 5, -1), (1024, 512),
         (5, 0), 0, 1, 0, 1, 0, 0),
        ("zero-width", (0x1234, 0x5678, 0, 512), (1024, 512),
         (0, 512), 0, 1, 0, 1, 0, 0),
        ("width-overlimit-zero-height", (0x1234, 0x5678, 2000, 0),
         (1024, 512), (1024, 0), 0, 1, 0, 1, 0, 0),
        ("height-overlimit-zero-width", (0x1234, 0x5678, 0, 600),
         (1024, 512), (0, 512), 0, 1, 0, 1, 0, 0),
        # Signed compare selects the unsigned high-bit limit bit pattern.
        ("negative-limit-sign-adjust", (0x1234, 0x5678, 5, 1),
         (0x8000, 512), (0x8000, 1), 0xFFFF8000, 0xFFFF8001, 1,
         0xFFFF8002, 0xFFFFC001, 0xFFFFFC00),
    ]
    for name, rect, limits, final, p, t, sign, adjusted, words, blocks in cases:
        machine = make_machine(
            exe, rect=rect, hardware=[], width_limit=limits[0],
            height_limit=limits[1]
        )
        result = machine.run(RECT, 0x80110000)
        machine.assert_scripts_consumed()
        require(result == MASK, f"{name} result is not -1")
        assert_wait_init(machine, 17)
        assert_rect_clamp(machine, 0x56781234, final[0], final[1])
        assert_arithmetic(machine, product=p, plus_one=t, sign=sign,
                          adjusted=adjusted, words=words, blocks=blocks)
        assert_no_issue(machine)
        passed(name, "result=-1 exact-clamp/arithmetic no-MMIO")


def run_success(exe: Exe, *, name: str,
                rect: tuple[int, int, int, int], source: int,
                source_words: list[int], rect_word1: int,
                prefix_words: list[int], blocks: int,
                arithmetic: tuple[int, int, int, int, int, int],
                statuses: list[int] | None = None,
                timeout_results: list[int] | None = None,
                update_counter: bool = False,
                width_limit: int = 1024, height_limit: int = 512,
                detail: str = "return=0") -> Machine:
    rect_word0 = ((rect[1] & 0xFFFF) << 16) | (rect[0] & 0xFFFF)
    statuses = statuses or [GPUSTAT_READY_GP0]
    hardware = issue_hardware(
        statuses, rect_word0, rect_word1, prefix_words, blocks,
        u32(source + len(prefix_words) * 4)
    )
    machine = make_machine(
        exe, rect=rect, hardware=hardware,
        timeout_results=timeout_results, update_counter=update_counter,
        width_limit=width_limit, height_limit=height_limit,
        source=source, source_words=source_words
    )
    result = machine.run(RECT, source)
    machine.assert_scripts_consumed()
    require(result == 0, f"{name} result 0x{result:08X}")
    assert_wait_init(machine, 17,
                     sum(1 for value in (timeout_results or []) if
                         update_counter and value == 0))
    p, t, sign, adjusted, words, arithmetic_blocks = arithmetic
    assert_arithmetic(machine, product=p, plus_one=t, sign=sign,
                      adjusted=adjusted, words=words,
                      blocks=arithmetic_blocks)
    require(arithmetic_blocks == blocks, f"{name} block expectation split")
    assert_rect_clamp(machine, rect_word0,
                      rect_word1 & 0xFFFF, rect_word1 >> 16)
    require(source_reads(machine) == [
        (source + index * 4, value & MASK)
        for index, value in enumerate(prefix_words)
    ], f"{name} CPU-prefix reads {source_reads(machine)}")
    passed(name, detail)
    return machine


def scenario_cpu_and_dma_thresholds(exe: Exe) -> None:
    source = 0x80110000

    one = [0xBEEF1234]
    machine = run_success(
        exe, name="one-pixel-odd-padding", rect=(1023, 511, 1, 1),
        source=source, source_words=one, rect_word1=0x00010001,
        prefix_words=one, blocks=0,
        arithmetic=(1, 2, 0, 2, 1, 0),
        detail="one whole CPU word; padding half retained only in source word"
    )
    require([event.value for event in machine.hardware_observed
             if event.kind == "write" and event.pc == 0x80076828] == one,
            "1x1 did not emit exact packed source word")

    fifteen = [0x51000000 + index for index in range(15)]
    machine = run_success(
        exe, name="below-dma-threshold-30px", rect=(3, 4, 15, 2),
        source=source, source_words=fifteen, rect_word1=0x0002000F,
        prefix_words=fifteen, blocks=0,
        arithmetic=(30, 31, 0, 31, 15, 0),
        detail="15 CPU words; no DMA registers"
    )
    require(machine.pc_counts.get(0x80076810) == 15 and
            machine.pc_counts.get(0x80076828) == 15,
            "15-word loop/load-delay count mismatch")

    for pixels in (31, 32):
        run_success(
            exe, name=f"exact-dma-block-{pixels}px",
            rect=(9, 10, pixels, 1), source=source, source_words=[],
            rect_word1=(1 << 16) | pixels, prefix_words=[], blocks=1,
            arithmetic=(pixels, pixels + 1, 0, pixels + 1, 16, 1),
            detail="MADR=source BCR=00010010 CHCR=01000201"
        )

    for pixels in (33, 34):
        first = [0xA5000000 | pixels]
        run_success(
            exe, name=f"remainder-plus-dma-{pixels}px",
            rect=(11, 12, pixels, 1), source=source,
            source_words=first, rect_word1=(1 << 16) | pixels,
            prefix_words=first, blocks=1,
            arithmetic=(pixels, pixels + 1, 0, pixels + 1, 17, 1),
            detail="one CPU word then MADR=source+4; DMA not completed"
        )


def scenario_canonical_and_maximum(exe: Exe) -> None:
    canonical = run_success(
        exe, name="canonical-disc1", rect=(704, 64, 32, 64),
        source=0x8012A8B8, source_words=[], rect_word1=0x00400020,
        prefix_words=[], blocks=64,
        arithmetic=(2048, 2049, 0, 2049, 1024, 64),
        detail="exact GP0 words; MADR=8012A8B8 BCR=00400010"
    )
    expected = [
        ("read", 0x80076758, GPUSTAT_GP1, GPUSTAT_READY_GP0),
        ("write", 0x800767AC, GPUSTAT_GP1, 0x04000000),
        ("write", 0x800767BC, GP0, 0x01000000),
        ("write", 0x800767D4, GP0, 0xA0000000),
        ("write", 0x800767E8, GP0, 0x004002C0),
        ("write", 0x800767FC, GP0, 0x00400020),
        ("write", 0x80076840, GPUSTAT_GP1, 0x04000002),
        ("write", 0x80076850, DMA2_MADR, 0x8012A8B8),
        ("write", 0x80076864, DMA2_BCR, 0x00400010),
        ("write", 0x80076874, DMA2_CHCR, 0x01000201),
    ]
    observed = [(event.kind, event.pc, event.address, event.value)
                for event in canonical.hardware_observed]
    require(observed == expected, f"canonical event order {observed}")
    require(WAIT_TIMEOUT not in calls(canonical),
            "canonical ready path called timeout helper")

    maximum = run_success(
        exe, name="clamped-maximum-1024x512",
        rect=(1023, 511, 2000, 600), source=0x80080000,
        source_words=[], rect_word1=0x02000400, prefix_words=[],
        blocks=0x4000,
        arithmetic=(0x00080000, 0x00080001, 0, 0x00080001,
                    0x00040000, 0x00004000),
        detail="clamp=1024x512 span=1MiB BCR=40000010"
    )
    require(source_span_safe(0x80080000, 0x40000),
            "maximum accepted source span safety")
    require(not source_reads(maximum), "maximum rem0 read source on CPU")


def scenario_readiness_boundary(exe: Exe) -> None:
    source = 0x80110000
    machine = make_machine(
        exe, rect=(1, 2, 1, 1),
        hardware=[read_status(0x80076758, 0)],
        source=source, source_words=[0x12345678],
        stop_at={WAIT_TIMEOUT}
    )
    try:
        machine.run(RECT, source)
    except DependencyStop as stop:
        require(stop.call.pc == 0x80076770 and
                stop.call.target == WAIT_TIMEOUT,
                "not-ready boundary stopped at wrong call")
        require(stop.call.args[0] == 0xA0000000,
                "not-ready helper ABI was not exact A0 register state")
    else:
        raise SystemExit("FATAL: not-ready path fabricated helper result")
    machine.assert_scripts_consumed()
    assert_wait_init(machine, 17)
    assert_arithmetic(machine, product=1, plus_one=2, sign=0,
                      adjusted=2, words=1, blocks=0)
    assert_no_issue(machine)
    require(machine.pc_counts.get(0x80076774) == 1,
            "func_80077404 delay slot count")
    passed("not-ready-honest-boundary",
           "stop=func_80077404 before GP1/GP0; no result invented")


def scenario_readiness_withheld(exe: Exe) -> None:
    source = 0x80110000
    machine = make_machine(
        exe, rect=(1, 2, 1, 1), source=source,
        source_words=[0x12345678],
        hardware=[read_status(0x80076758, 0),
                  read_status(0x8007678C, 0)],
        timeout_results=[0, 0], update_counter=True
    )
    try:
        machine.run(RECT, source)
    except HardwareStop as stop:
        require(stop.event.kind == "read" and
                stop.event.pc == 0x8007678C and
                stop.event.address == GPUSTAT_GP1,
                f"withheld event stop {stop.event}")
    else:
        raise SystemExit("FATAL: exhausted status script made progress")
    machine.assert_scripts_consumed()
    assert_wait_init(machine, 17, 2)
    assert_no_issue(machine)
    require(calls(machine) == [WAIT_INIT, VSYNC, WAIT_TIMEOUT, WAIT_TIMEOUT],
            f"withheld call order {calls(machine)}")
    passed("not-ready-withheld",
           "two clear polls stable; third read needs explicit event")


def scenario_readiness_transition(exe: Exe) -> None:
    word = [0x89ABCDEF]
    machine = run_success(
        exe, name="explicit-not-ready-to-ready",
        rect=(7, 8, 1, 1), source=0x80110000, source_words=word,
        rect_word1=0x00010001, prefix_words=word, blocks=0,
        arithmetic=(1, 2, 0, 2, 1, 0), statuses=[0, 0, 0x04000000],
        timeout_results=[0, 0], update_counter=True,
        detail="status script [clear,clear,ready]; helper never changes status"
    )
    status_reads = [event for event in machine.hardware_observed
                    if event.kind == "read"]
    require([(event.pc, event.value) for event in status_reads] == [
        (0x80076758, 0), (0x8007678C, 0),
        (0x8007678C, GPUSTAT_READY_GP0),
    ], f"explicit status trace {status_reads}")
    require(calls(machine) == [WAIT_INIT, VSYNC, WAIT_TIMEOUT, WAIT_TIMEOUT],
            f"transition helper calls {calls(machine)}")


def scenario_timeout_result(exe: Exe) -> None:
    source = 0x80110000
    machine = make_machine(
        exe, rect=(1, 2, 1, 1), source=source,
        source_words=[0x12345678],
        hardware=[read_status(0x80076758, 0)],
        timeout_results=[7]
    )
    result = machine.run(RECT, source)
    machine.assert_scripts_consumed()
    require(result == MASK, "nonzero timeout result was not collapsed to -1")
    assert_wait_init(machine, 17)
    assert_no_issue(machine)
    require(machine.pc_counts.get(0x8007677C) == 1 and
            regular_value(machine, 0x8007677C, 2) == MASK,
            "timeout branch delay did not force -1")
    passed("controlled-timeout", "callee result=7 -> worker -1; no issue")


def scenario_source_safety(exe: Exe) -> None:
    vectors = [
        ("exact-last-word", RAM_END - 4, 1, True),
        ("cross-end", RAM_END - 4, 2, False),
        ("at-end", RAM_END, 1, False),
        ("unaligned", RAM_END - 5, 1, False),
        ("zero", 0, 1, False),
        ("low-address", 0x00100000, 1, False),
        ("uint32-high", 0xFFFFFFFC, 1, False),
        ("address-add-overflow", 0xFFFFFFFC, 2, False),
        ("byte-count-overflow", RAM_BASE, 0x40000000, False),
        ("canonical", 0x8012A8B8, 1024, True),
        ("maximum", 0x80080000, 0x40000, True),
    ]
    for name, address, words, expected in vectors:
        require(source_span_safe(address, words) == expected,
                f"source safety vector {name}")
    passed("source-preflight-vectors",
           f"{len(vectors)} exact alignment/range/overflow decisions")

    # The exact valid RAM endpoint is executable for the one-word CPU path.
    run_success(
        exe, name="source-exact-ram-end-accepted", rect=(0, 0, 1, 1),
        source=RAM_END - 4, source_words=[0xCAFEBABE],
        rect_word1=0x00010001, prefix_words=[0xCAFEBABE], blocks=0,
        arithmetic=(1, 2, 0, 2, 1, 0),
        detail="last aligned word read exactly once"
    )

    # Retail emits the header before its first lw faults.  The native safety
    # envelope must reject this before invoking the literal issue sequence.
    for name, bad_source, reason in [
        ("literal-unaligned-source-fault", 0x801FFFFB, "unaligned"),
        ("literal-zero-source-fault", 0, "outside modeled RAM"),
    ]:
        header = issue_hardware(
            [GPUSTAT_READY_GP0], 0, 0x00010001, [], 0, bad_source
        )
        # issue_hardware has no pixel event when prefix_words is empty; retain
        # only status/header, because the bad lw faults before the GP0 store.
        machine = make_machine(
            exe, rect=(0, 0, 1, 1), hardware=header,
            source=None
        )
        try:
            machine.run(RECT, bad_source)
        except GuestFault as fault:
            require(fault.pc == 0x80076810 and fault.address == bad_source and
                    fault.reason == reason,
                    f"{name} fault {fault}")
        else:
            raise SystemExit(f"FATAL: {name} did not fault literally")
        machine.assert_scripts_consumed()
        require(not source_span_safe(bad_source, 1),
                f"{name} safety envelope accepted")
        passed(name, "retail fault kept explicit; native preflight rejects")

    # Two CPU words cross the end: first word is visibly sent, second lw is
    # the literal fault.  This proves why full-span validation must precede
    # *all* native GPU mutations rather than relying on per-word loads.
    cross_source = RAM_END - 4
    first = 0x0BADF00D
    hardware = issue_hardware(
        [GPUSTAT_READY_GP0], 0, 0x00010003, [first], 0,
        cross_source + 4
    )
    machine = make_machine(
        exe, rect=(0, 0, 3, 1), hardware=hardware,
        source=cross_source, source_words=[first]
    )
    try:
        machine.run(RECT, cross_source)
    except GuestFault as fault:
        require(fault.pc == 0x80076810 and fault.address == RAM_END,
                f"cross-end literal fault {fault}")
    else:
        raise SystemExit("FATAL: cross-end literal source did not fault")
    machine.assert_scripts_consumed()
    require(source_reads(machine) == [(cross_source, first)],
            "cross-end first word was not exact")
    require(not source_span_safe(cross_source, 2),
            "cross-end safety envelope accepted")
    passed("literal-cross-end-second-word-fault",
           "first retail word visible; native full-span preflight rejects all")

    # With remainder zero, retail never dereferences the bad source and writes
    # it verbatim to MADR.  This is evidence for, not a substitute for, the
    # stricter native preflight contract.
    bad_dma_source = 0xFFFFFFFC
    machine = run_success(
        exe, name="literal-dma-address-remains-32bit",
        rect=(0, 0, 31, 1), source=bad_dma_source, source_words=[],
        rect_word1=0x0001001F, prefix_words=[], blocks=1,
        arithmetic=(31, 32, 0, 32, 16, 1),
        detail="retail MADR=FFFFFFFC; no low mirror/no host pointer"
    )
    require(not source_span_safe(bad_dma_source, 16),
            "bad DMA source accepted by native envelope")
    require(not source_reads(machine), "DMA-only path dereferenced source")


def scenario_corrupt_double_negative_limits(exe: Exe) -> None:
    # Both signed limits are -32768, but their selected lhu bit patterns are
    # multiplied as signed halfwords after storage.  The positive low32
    # product demonstrates the literal arithmetic without attempting the
    # enormous unsafe transfer in native code.
    source = 0x80000000
    blocks = 0x02000000
    machine = run_success(
        exe, name="corrupt-double-negative-limits-literal",
        rect=(0x2222, 0x3333, 5, 5), source=source, source_words=[],
        rect_word1=0x80008000, prefix_words=[], blocks=blocks,
        arithmetic=(0x40000000, 0x40000001, 0, 0x40000001,
                    0x20000000, blocks),
        width_limit=0x8000, height_limit=0x8000,
        detail="literal BCR truncates to 00000010; safety span rejects"
    )
    require(not source_span_safe(source, 0x20000000),
            "corrupt enormous span passed safety")
    bcr = [event.value for event in machine.hardware_observed
           if event.address == DMA2_BCR]
    require(bcr == [0x00000010], f"corrupt block BCR truncation {bcr}")


def main() -> int:
    if len(sys.argv) > 2:
        raise SystemExit(f"usage: {sys.argv[0]} [SHA-exact-SLUS_006.62]")
    path = pathlib.Path(sys.argv[1] if len(sys.argv) == 2
                        else "build/extracted/disc1/SLUS_006.62")
    exe = Exe(path)
    verify_static(exe)

    scenario_nonpositive_and_signed_limits(exe)
    scenario_cpu_and_dma_thresholds(exe)
    scenario_canonical_and_maximum(exe)
    scenario_readiness_boundary(exe)
    scenario_readiness_withheld(exe)
    scenario_readiness_transition(exe)
    scenario_timeout_result(exe)
    scenario_source_safety(exe)
    scenario_corrupt_double_negative_limits(exe)

    print(f"B53E ORACLE: PASS — {SCENARIO_COUNT} scenarios; literal worker "
          "executed; hardware/callees explicit; no automatic progress")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
