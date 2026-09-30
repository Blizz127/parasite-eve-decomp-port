#!/usr/bin/env python3
"""Phase 6E-B53C retail oracle: dispatcher prefix and dependency contract.

This standalone oracle verifies and executes the literal MIPS-I body of
func_80076C34 and its tiny inseparable helper func_800773D0 from the
SHA-exact retail executable.  Hardware and untranslated retail callees are
controlled boundaries: reads never make hardware progress, and a pump can
change queue state only when a scenario explicitly supplies that mutation.

The planned B53C production prefix ends at func_80077404 when the ring is
full and at func_80073E10 when it is not.  Full-body oracle scenarios exist
only to prove the already-recovered state machine, copy rules, delay slots,
and return values; they do not import or call production C.
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
DISPATCH_SHA256 = (
    "b3686b34851b08fa3bb0097263caf59519056417b593606b7e7a59c155b1e508"
)
WAIT_INIT_SHA256 = (
    "3b0637e8b6472855bf28115f4712ea1f43c3d3ad130c388ab71c8101d3d00cb5"
)

MASK = 0xFFFFFFFF
RAM_BASE = 0x80000000
RAM_SIZE = 0x200000
SP = 0x801FFF00
RA_SENTINEL = 0xDEADBEEC

DISPATCH = 0x80076C34
DISPATCH_END = 0x80076EE4
WAIT_INIT = 0x800773D0
WAIT_INIT_END = 0x80077404
WAIT_TIMEOUT = 0x80077404
QUEUE_PUMP = 0x80076EE4
EXCHANGE_IMASK = 0x80073E10
SET_DMA_CALLBACK = 0x80073CF4
VSYNC = 0x80073A44
LOADIMAGE_WORKER = 0x80076664

GPUSTAT = 0x1F801814
DMA2_CHCR = 0x1F8010A8
GPUSTAT_READY_GP0 = 0x04000000
DMA2_BUSY = 0x01000000

GPU_INIT = 0x8009574D
WORK_MARKER = 0x80095754
DRAW_SYNC_CALLBACK = 0x80095758
GPUSTAT_POINTER = 0x80095854
DMA2_CHCR_POINTER = 0x80095860
PRODUCER = 0x80095874
CONSUMER = 0x80095878
SAVED_IMASK = 0x8009587C
WAIT_DEADLINE = 0x80095888
WAIT_COUNTER = 0x8009588C

RING_BASE = 0x800BD030
RING_ENTRIES = 64
RING_ENTRY_SIZE = 0x60
RING_SIZE = RING_ENTRIES * RING_ENTRY_SIZE


# All 172 literal words at 0x80076C34..0x80076EE4 exclusive.  These are
# constants, not extracted expectations; verify_static compares every one
# to the independently loaded executable body.
DISPATCH_WORDS = exe_words(0x80076C34, 172)


# Complete 13-word func_800773D0 helper.  Its sole external dependency is
# the explicitly supplied VSync(-1) query value.
WAIT_INIT_WORDS = exe_words(0x800773D0, 13)


# Independent control-transfer fingerprint: PC, kind, decoded target (None
# for register targets), and the exact instruction in its delay slot.
DISPATCH_CONTROL = [
    (0x80076C58, "jal", 0x800773D0, 0x00E09021),
    (0x80076C60, "j", 0x80076C80, 0x00000000),
    (0x80076C68, "jal", 0x80077404, 0x00000000),
    (0x80076C70, "bne", 0x80076EC8, 0x2402FFFF),
    (0x80076C78, "jal", 0x80076EE4, 0x00000000),
    (0x80076C98, "beq", 0x80076C68, 0x00000000),
    (0x80076CA0, "jal", 0x80073E10, 0x00002021),
    (0x80076CC0, "beq", 0x80076D14, 0xAC820008),
    (0x80076CDC, "bne", 0x80076D58, 0x00000000),
    (0x80076CFC, "bne", 0x80076D58, 0x00000000),
    (0x80076D0C, "bne", 0x80076D58, 0x00000000),
    (0x80076D2C, "beq", 0x80076D20, 0x00000000),
    (0x80076D38, "jalr", None, 0x02402821),
    (0x80076D48, "jal", 0x80073E10, 0x00000000),
    (0x80076D50, "j", 0x80076EC8, 0x00001021),
    (0x80076D60, "jal", 0x80073CF4, 0x24040002),
    (0x80076D68, "beq", 0x80076E14, 0x00003021),
    (0x80076D80, "bgez", 0x80076D8C, 0x00000000),
    (0x80076D94, "beq", 0x80076DD0, 0x00062080),
    (0x80076DC8, "j", 0x80076D80, 0x02201021),
    (0x80076E0C, "j", 0x80076E38, 0xAC22D034),
    (0x80076E9C, "jal", 0x80073E10, 0xAC225874),
    (0x80076EA4, "jal", 0x80076EE4, 0x00000000),
    (0x80076EDC, "jr", None, 0x27BD0028),
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
class Call:
    pc: int
    target: int
    args: tuple[int, int, int, int]


@dataclass(frozen=True)
class Action:
    result: int = 0
    # Callee effects are allowed only when listed explicitly by the case.
    writes: tuple[tuple[int, int, int], ...] = ()


class DependencyStop(Exception):
    def __init__(self, call: Call):
        super().__init__(f"dependency 0x{call.target:08X}")
        self.call = call


class Machine:
    def __init__(self, exe: Exe, *, vsync: int, gpu_status: int,
                 dma2_chcr: int,
                 actions: dict[int, list[Action]] | None = None,
                 stop_at: set[int] | None = None):
        self.ram = bytearray(RAM_SIZE)
        offset = exe.taddr - RAM_BASE
        require(offset >= 0 and offset + exe.tsize <= RAM_SIZE,
                "executable does not fit guest RAM")
        self.ram[offset:offset + exe.tsize] = exe.image
        self.vsync = u32(vsync)
        self.gpu_status = u32(gpu_status)
        self.dma2_chcr = u32(dma2_chcr)
        self.actions = {
            target: deque(items) for target, items in (actions or {}).items()
        }
        self.stop_at = set(stop_at or ())
        self.calls: list[Call] = []
        self.mmio_reads: list[tuple[int, int]] = []
        self.memory_reads: list[tuple[int, int, int]] = []
        self.retail_writes: list[tuple[int, int, int, int]] = []
        self.controlled_writes: list[tuple[int, int, int]] = []
        self.pc_counts: dict[int, int] = {}
        self.vsync_queries = 0
        self.current_pc = 0

    @staticmethod
    def offset(address: int, size: int) -> int:
        offset = address - RAM_BASE
        require(offset >= 0 and offset + size <= RAM_SIZE,
                f"guest access 0x{address:08X}+{size}")
        return offset

    def seed(self, address: int, size: int, value: int) -> None:
        offset = self.offset(address, size)
        mask = (1 << (size * 8)) - 1
        self.ram[offset:offset + size] = (value & mask).to_bytes(size, "little")

    def seed_bytes(self, address: int, data: bytes) -> None:
        offset = self.offset(address, len(data))
        self.ram[offset:offset + len(data)] = data

    def peek(self, address: int, size: int) -> int:
        offset = self.offset(address, size)
        return int.from_bytes(self.ram[offset:offset + size], "little")

    def bytes(self, address: int, size: int) -> bytes:
        offset = self.offset(address, size)
        return bytes(self.ram[offset:offset + size])

    def fetch(self, address: int) -> int:
        return self.peek(address, 4)

    def load(self, address: int, size: int) -> int:
        if size == 4 and address == GPUSTAT:
            self.mmio_reads.append((address, self.gpu_status))
            return self.gpu_status
        if size == 4 and address == DMA2_CHCR:
            self.mmio_reads.append((address, self.dma2_chcr))
            return self.dma2_chcr
        self.memory_reads.append((self.current_pc, address, size))
        return self.peek(address, size)

    def store(self, address: int, size: int, value: int) -> None:
        self.seed(address, size, value)
        self.retail_writes.append(
            (self.current_pc, address, size,
             value & ((1 << (size * 8)) - 1))
        )

    def controlled_store(self, address: int, size: int, value: int) -> None:
        self.seed(address, size, value)
        self.controlled_writes.append(
            (address, size, value & ((1 << (size * 8)) - 1))
        )

    @staticmethod
    def is_control(word: int) -> bool:
        op = (word >> 26) & 0x3F
        fn = word & 0x3F
        return op in (1, 2, 3, 4, 5) or (op == 0 and fn in (8, 9))

    def one(self, registers: list[int], pc: int) -> int | None:
        self.current_pc = pc
        self.pc_counts[pc] = self.pc_counts.get(pc, 0) + 1
        word = self.fetch(pc)
        op = (word >> 26) & 0x3F
        rs = (word >> 21) & 31
        rt = (word >> 16) & 31
        rd = (word >> 11) & 31
        shift = (word >> 6) & 31
        immediate = word & 0xFFFF
        signed_immediate = sx16(immediate)

        if word == 0:
            return None
        if op == 0:
            function = word & 0x3F
            if function == 0x00:       # sll
                registers[rd] = u32(registers[rt] << shift)
            elif function == 0x03:     # sra
                registers[rd] = u32(s32(registers[rt]) >> shift)
            elif function == 0x08:     # jr
                return registers[rs]
            elif function == 0x09:     # jalr
                registers[rd] = u32(pc + 8)
                return registers[rs]
            elif function == 0x21:     # addu
                registers[rd] = u32(registers[rs] + registers[rt])
            elif function == 0x23:     # subu
                registers[rd] = u32(registers[rs] - registers[rt])
            elif function == 0x24:     # and
                registers[rd] = registers[rs] & registers[rt]
            elif function == 0x2A:     # slt
                registers[rd] = int(s32(registers[rs]) < s32(registers[rt]))
            else:
                raise SystemExit(
                    f"FATAL: SPECIAL {function:02X} @0x{pc:08X}"
                )
            return None
        if op == 1:                    # bgez used by signed /4 sequence
            require(rt == 1, f"REGIMM rt={rt} @0x{pc:08X}")
            if s32(registers[rs]) >= 0:
                return u32(pc + 4 + (signed_immediate << 2))
            return None
        if op == 2:                    # j
            return u32(((pc + 4) & 0xF0000000) |
                       ((word & 0x03FFFFFF) << 2))
        if op == 3:                    # jal
            registers[31] = u32(pc + 8)
            return u32(((pc + 4) & 0xF0000000) |
                       ((word & 0x03FFFFFF) << 2))
        if op == 4:                    # beq
            if registers[rs] == registers[rt]:
                return u32(pc + 4 + (signed_immediate << 2))
            return None
        if op == 5:                    # bne
            if registers[rs] != registers[rt]:
                return u32(pc + 4 + (signed_immediate << 2))
            return None
        if op == 9:                    # addiu
            registers[rt] = u32(registers[rs] + signed_immediate)
        elif op == 0x0C:               # andi
            registers[rt] = registers[rs] & immediate
        elif op == 0x0F:               # lui
            registers[rt] = u32(immediate << 16)
        elif op == 0x23:               # lw
            registers[rt] = self.load(u32(registers[rs] + signed_immediate), 4)
        elif op == 0x24:               # lbu
            registers[rt] = self.load(u32(registers[rs] + signed_immediate), 1)
        elif op == 0x2B:               # sw
            self.store(u32(registers[rs] + signed_immediate), 4, registers[rt])
        else:
            raise SystemExit(f"FATAL: opcode {op:02X} @0x{pc:08X}")
        return None

    def handle_call(self, call: Call, registers: list[int], resume_pc: int) -> int:
        self.calls.append(call)
        if call.target == WAIT_INIT:
            return WAIT_INIT
        if call.target == VSYNC:
            require(call.args[0] == MASK,
                    f"VSync argument 0x{call.args[0]:08X} is not -1")
            self.vsync_queries += 1
            registers[2] = self.vsync
            return resume_pc
        if call.target in self.stop_at:
            raise DependencyStop(call)
        queue = self.actions.get(call.target)
        require(queue is not None and queue,
                f"uncontrolled call 0x{call.target:08X} from 0x{call.pc:08X}")
        action = queue.popleft()
        for address, size, value in action.writes:
            self.controlled_store(address, size, value)
        registers[2] = u32(action.result)
        return resume_pc

    def run(self, worker: int, argument: int, copy_bytes: int,
            auxiliary: int, max_steps: int = 20000) -> int:
        registers = [0] * 32
        registers[4] = u32(worker)
        registers[5] = u32(argument)
        registers[6] = u32(copy_bytes)
        registers[7] = u32(auxiliary)
        registers[29] = SP
        registers[31] = RA_SENTINEL
        pc = DISPATCH

        for _ in range(max_steps):
            word = self.fetch(pc)
            if not self.is_control(word):
                self.one(registers, pc)
                registers[0] = 0
                pc = u32(pc + 4)
                continue

            op = (word >> 26) & 0x3F
            function = word & 0x3F
            target = self.one(registers, pc)
            delay_word = self.fetch(u32(pc + 4))
            require(not self.is_control(delay_word),
                    f"control instruction in delay slot at 0x{pc + 4:08X}")
            self.one(registers, u32(pc + 4))
            registers[0] = 0

            is_call = op == 3 or (op == 0 and function == 9)
            if is_call:
                require(target is not None, f"null call target at 0x{pc:08X}")
                call = Call(pc, target,
                            tuple(registers[i] for i in range(4, 8)))
                pc = self.handle_call(call, registers, u32(pc + 8))
                continue

            if op == 0 and function == 8:       # jr
                require(target is not None, f"null jr target at 0x{pc:08X}")
                if target == RA_SENTINEL:
                    self.registers = registers
                    return registers[2]
                pc = target
                continue

            pc = u32(pc + 8) if target is None else target

        raise SystemExit(
            "FATAL: interpreter step limit; hardware/dependency did not progress"
        )

    def assert_actions_consumed(self) -> None:
        pending = {target: len(queue) for target, queue in self.actions.items()
                   if queue}
        require(not pending, f"unused controlled call actions: {pending}")


def decode_control(pc: int, word: int) -> tuple[str, int | None] | None:
    op = (word >> 26) & 0x3F
    function = word & 0x3F
    immediate = sx16(word)
    if op == 2:
        return "j", ((pc + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)
    if op == 3:
        return "jal", ((pc + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)
    if op == 4:
        return "beq", u32(pc + 4 + (immediate << 2))
    if op == 5:
        return "bne", u32(pc + 4 + (immediate << 2))
    if op == 1 and ((word >> 16) & 31) == 1:
        return "bgez", u32(pc + 4 + (immediate << 2))
    if op == 0 and function == 8:
        return "jr", None
    if op == 0 and function == 9:
        return "jalr", None
    return None


def control_fingerprint(base: int, words: list[int]) -> list[tuple]:
    result = []
    for index, word in enumerate(words):
        pc = base + index * 4
        decoded = decode_control(pc, word)
        if decoded is not None:
            kind, target = decoded
            require(index + 1 < len(words), f"missing delay slot at 0x{pc:08X}")
            result.append((pc, kind, target, words[index + 1]))
    return result


def verify_static(exe: Exe) -> None:
    require(len(DISPATCH_WORDS) == 172, "dispatcher literal count is not 172")
    require(DISPATCH_END - DISPATCH == 0x2B0, "dispatcher range is not 0x2B0")
    require(len(WAIT_INIT_WORDS) == 13, "wait-init literal count is not 13")
    require(WAIT_INIT_END - WAIT_INIT == 0x34, "wait-init range is not 0x34")

    dispatch_literal = words_bytes(DISPATCH_WORDS)
    wait_literal = words_bytes(WAIT_INIT_WORDS)
    dispatch_body = exe.body(DISPATCH, DISPATCH_END - DISPATCH)
    wait_body = exe.body(WAIT_INIT, WAIT_INIT_END - WAIT_INIT)
    require(dispatch_literal == dispatch_body,
            "one or more of 172 dispatcher words differ from executable")
    require(wait_literal == wait_body,
            "one or more wait-init words differ from executable")
    require(hashlib.sha256(dispatch_body).hexdigest() == DISPATCH_SHA256,
            "dispatcher body SHA-256 mismatch")
    require(hashlib.sha256(wait_body).hexdigest() == WAIT_INIT_SHA256,
            "wait-init body SHA-256 mismatch")
    require(control_fingerprint(DISPATCH, DISPATCH_WORDS) == DISPATCH_CONTROL,
            "dispatcher control-transfer/delay-slot fingerprint mismatch")
    require(control_fingerprint(WAIT_INIT, WAIT_INIT_WORDS) == WAIT_INIT_CONTROL,
            "wait-init control-transfer/delay-slot fingerprint mismatch")
    require(exe.word(GPUSTAT_POINTER) == GPUSTAT,
            "retail GPUSTAT pointer changed")
    require(exe.word(DMA2_CHCR_POINTER) == DMA2_CHCR,
            "retail DMA2 CHCR pointer changed")

    print(f"SHA-1 verified: {EXE_SHA1}")
    print("func_80076C34: 172 literal words EXACT, "
          "0x80076C34..0x80076EE4, file 0x67434")
    print(f"body SHA-256 verified: {DISPATCH_SHA256}")
    print("control flow verified: 24 transfers with exact delay slots")
    print("func_800773D0: 13 literal words EXACT; VSync(-1) is controlled")


def ring_pattern() -> bytes:
    return bytes(((index * 37 + 0x5B) & 0xFF) for index in range(RING_SIZE))


def make_machine(exe: Exe, *, producer: int, consumer: int,
                 initialized: int = 1, callback: int = 0,
                 vsync: int = 7, gpu_status: int = GPUSTAT_READY_GP0,
                 dma2_chcr: int = 0x00000401,
                 actions: dict[int, list[Action]] | None = None,
                 stop_at: set[int] | None = None) -> Machine:
    machine = Machine(exe, vsync=vsync, gpu_status=gpu_status,
                      dma2_chcr=dma2_chcr, actions=actions, stop_at=stop_at)
    machine.seed_bytes(RING_BASE, ring_pattern())
    machine.seed(PRODUCER, 4, producer)
    machine.seed(CONSUMER, 4, consumer)
    machine.seed(GPU_INIT, 1, initialized)
    machine.seed(DRAW_SYNC_CALLBACK, 4, callback)
    machine.seed(WORK_MARKER, 4, 0)
    machine.seed(SAVED_IMASK, 4, 0xCCCCCCCC)
    machine.seed(WAIT_DEADLINE, 4, 0xDDDDDDDD)
    machine.seed(WAIT_COUNTER, 4, 0xEEEEEEEE)
    return machine


def calls(machine: Machine) -> list[int]:
    return [call.target for call in machine.calls]


def persistent_writes(machine: Machine) -> list[tuple[int, int, int, int]]:
    return [write for write in machine.retail_writes
            if not (SP - 0x80 <= write[1] < SP)]


def assert_wait_init(machine: Machine, initial_vsync: int) -> None:
    require(machine.vsync_queries == 1, "VSync query count is not exactly one")
    require(machine.peek(WAIT_DEADLINE, 4) == u32(initial_vsync + 0xF0),
            "wait deadline is not VSync(-1)+240 with 32-bit wrap")
    require(machine.peek(WAIT_COUNTER, 4) == 0,
            "wait poll counter was not reset")
    query = [call for call in machine.calls if call.target == VSYNC]
    require(len(query) == 1 and query[0].pc == 0x800773D8 and
            query[0].args[0] == MASK,
            "wait helper did not issue exact VSync(-1) call")
    require(machine.pc_counts.get(0x800773DC) == 1,
            "VSync jal delay slot did not execute exactly once")


def assert_only_wait_globals(machine: Machine) -> None:
    writes = persistent_writes(machine)
    require([(pc, address) for pc, address, _, _ in writes] == [
        (0x800773E8, WAIT_DEADLINE),
        (0x800773F0, WAIT_COUNTER),
    ], f"unexpected prefix writes: {writes}")


def expected_ring(before: bytes, slot: int, worker: int, argument: int,
                  auxiliary: int, payload: bytes | None) -> bytes:
    expected = bytearray(before)
    offset = slot * RING_ENTRY_SIZE
    expected[offset:offset + 4] = struct.pack("<I", u32(worker))
    expected[offset + 4:offset + 8] = struct.pack("<I", u32(argument))
    expected[offset + 8:offset + 12] = struct.pack("<I", u32(auxiliary))
    if payload is not None:
        expected[offset + 12:offset + 12 + len(payload)] = payload
    return bytes(expected)


def assert_enqueue_write_order(machine: Machine, slot: int,
                               copy_bytes: int,
                               expected_copy_words: int) -> None:
    """Prove payload -> metadata -> producer publication in retail order."""
    require(expected_copy_words >= 0,
            f"negative expected copy count {expected_copy_words}")
    entry = RING_BASE + slot * RING_ENTRY_SIZE
    expected = [
        (0x80076DC4, entry + 0x0C + index * 4)
        for index in range(expected_copy_words)
    ]
    expected.extend([
        (0x80076E34 if copy_bytes == 0 else 0x80076E10, entry + 0x04),
        (0x80076E58, entry + 0x08),
        (0x80076E7C, entry + 0x00),
        (0x80076EA0, PRODUCER),
    ])
    actual = [
        (pc, address)
        for pc, address, _, _ in persistent_writes(machine)
        if RING_BASE <= address < RING_BASE + RING_SIZE or address == PRODUCER
    ]
    require(actual == expected,
            f"enqueue write ordering {actual} != {expected}")


def scenario_nonfull_dependency(exe: Exe) -> None:
    vsync = 23
    machine = make_machine(exe, producer=0, consumer=0, vsync=vsync,
                           stop_at={EXCHANGE_IMASK})
    before_ring = machine.bytes(RING_BASE, RING_SIZE)
    try:
        machine.run(LOADIMAGE_WORKER, 0x80102000, 8, 0x80110000)
    except DependencyStop as stop:
        require(stop.call.pc == 0x80076CA0 and stop.call.target == EXCHANGE_IMASK,
                "nonfull prefix stopped at wrong dependency")
        require(stop.call.args[0] == 0,
                "func_80073E10 delay-slot a0 was not zero")
    else:
        raise SystemExit("FATAL: nonfull prefix did not stop at func_80073E10")
    assert_wait_init(machine, vsync)
    assert_only_wait_globals(machine)
    require(calls(machine) == [WAIT_INIT, VSYNC, EXCHANGE_IMASK],
            f"nonfull dependency call order {calls(machine)}")
    require(machine.bytes(RING_BASE, RING_SIZE) == before_ring,
            "nonfull prefix wrote the ring")
    require(machine.peek(PRODUCER, 4) == 0 and machine.peek(CONSUMER, 4) == 0,
            "nonfull prefix changed queue indices")
    require(machine.pc_counts.get(0x80076CA4) == 1,
            "func_80073E10 jal delay slot count")
    print("scenario dependency-nonfull: PASS stop=func_80073E10 ring-writes=0")


def scenario_full_dependency(exe: Exe) -> None:
    # producer 10 is full exactly when consumer is 11.
    vsync = 0xFFFFFF80
    machine = make_machine(exe, producer=10, consumer=11, vsync=vsync,
                           stop_at={WAIT_TIMEOUT})
    before_ring = machine.bytes(RING_BASE, RING_SIZE)
    try:
        machine.run(LOADIMAGE_WORKER, 0x80102000, 8, 0x80110000)
    except DependencyStop as stop:
        require(stop.call.pc == 0x80076C68 and stop.call.target == WAIT_TIMEOUT,
                "full prefix stopped at wrong dependency")
    else:
        raise SystemExit("FATAL: full prefix did not stop at func_80077404")
    assert_wait_init(machine, vsync)
    assert_only_wait_globals(machine)
    require(machine.peek(WAIT_DEADLINE, 4) == 0x70,
            "wrapped wait deadline mismatch")
    require(calls(machine) == [WAIT_INIT, VSYNC, WAIT_TIMEOUT],
            f"full dependency call order {calls(machine)}")
    require(machine.bytes(RING_BASE, RING_SIZE) == before_ring,
            "full prefix wrote the ring")
    require(machine.peek(PRODUCER, 4) == 10 and machine.peek(CONSUMER, 4) == 11,
            "full prefix changed queue indices")
    require(machine.pc_counts.get(0x80076C6C) == 1,
            "func_80077404 jal delay slot count")
    print("scenario dependency-full: PASS stop=func_80077404 wrap-deadline=0x70")


def scenario_timeout(exe: Exe) -> None:
    vsync = 100
    machine = make_machine(
        exe, producer=10, consumer=11, vsync=vsync,
        actions={WAIT_TIMEOUT: [Action(MASK)]},
    )
    before_ring = machine.bytes(RING_BASE, RING_SIZE)
    result = machine.run(LOADIMAGE_WORKER, 0x80102000, 8, 0x80110000)
    machine.assert_actions_consumed()
    assert_wait_init(machine, vsync)
    assert_only_wait_globals(machine)
    require(result == MASK, f"timeout result 0x{result:08X} is not -1")
    require(calls(machine) == [WAIT_INIT, VSYNC, WAIT_TIMEOUT],
            f"timeout call order {calls(machine)}")
    require(QUEUE_PUMP not in calls(machine), "timeout incorrectly called pump")
    require(machine.pc_counts.get(0x80076C74) == 1,
            "timeout branch delay slot did not set -1 exactly once")
    require(machine.bytes(RING_BASE, RING_SIZE) == before_ring,
            "timeout wrote the ring")
    print("scenario controlled-timeout: PASS result=-1 pump=not-called")


def scenario_full_progress(exe: Exe) -> None:
    # This is the wrap-full state: (63+1)&63 == consumer 0.  Progress occurs
    # only because the supplied pump action explicitly changes consumer.
    vsync = 31
    machine = make_machine(
        exe, producer=63, consumer=0, vsync=vsync,
        actions={
            WAIT_TIMEOUT: [Action(0)],
            QUEUE_PUMP: [Action(0, ((CONSUMER, 4, 1),))],
        },
        stop_at={EXCHANGE_IMASK},
    )
    before_ring = machine.bytes(RING_BASE, RING_SIZE)
    try:
        machine.run(LOADIMAGE_WORKER, 0x80102000, 8, 0x80110000)
    except DependencyStop as stop:
        require(stop.call.target == EXCHANGE_IMASK,
                "full-progress case did not reach nonfull dependency")
    else:
        raise SystemExit("FATAL: full-progress case did not stop")
    machine.assert_actions_consumed()
    assert_wait_init(machine, vsync)
    assert_only_wait_globals(machine)
    require(calls(machine) == [WAIT_INIT, VSYNC, WAIT_TIMEOUT,
                               QUEUE_PUMP, EXCHANGE_IMASK],
            f"full-progress call order {calls(machine)}")
    require(machine.controlled_writes == [(CONSUMER, 4, 1)],
            "pump progress was not the sole controlled mutation")
    require(machine.peek(PRODUCER, 4) == 63 and machine.peek(CONSUMER, 4) == 1,
            "explicit wrap-full progress state mismatch")
    require(machine.pc_counts.get(0x80076C98) == 2,
            "full condition was not checked before and after pump")
    require(machine.bytes(RING_BASE, RING_SIZE) == before_ring,
            "full-progress prefix wrote the ring")
    print("scenario wrap-full-progress: PASS explicit-pump-only stop=func_80073E10")


def scenario_direct_worker_dependency(exe: Exe) -> None:
    """Stop before fabricating the command-issue worker's return/effects."""
    vsync = 8
    old_mask = 0x2468
    argument = 0x80102300
    auxiliary = 0x8ABCDEF0
    machine = make_machine(
        exe, producer=0, consumer=0, vsync=vsync,
        actions={EXCHANGE_IMASK: [Action(old_mask)]},
        stop_at={LOADIMAGE_WORKER},
    )
    before_ring = machine.bytes(RING_BASE, RING_SIZE)
    try:
        machine.run(LOADIMAGE_WORKER, argument, 8, auxiliary)
    except DependencyStop as stop:
        require(stop.call.pc == 0x80076D38 and
                stop.call.target == LOADIMAGE_WORKER,
                "direct worker prefix stopped at wrong dependency")
        require(stop.call.args[:2] == (argument, auxiliary),
                "direct worker boundary ABI/delay slot mismatch")
    else:
        raise SystemExit("FATAL: direct path fabricated a worker return")
    machine.assert_actions_consumed()
    assert_wait_init(machine, vsync)
    require(calls(machine) == [WAIT_INIT, VSYNC, EXCHANGE_IMASK,
                               LOADIMAGE_WORKER],
            f"direct worker dependency call order {calls(machine)}")
    require(LOADIMAGE_WORKER not in machine.actions,
            "direct worker boundary unexpectedly had a result action")
    require(machine.pc_counts.get(0x80076D3C) == 1,
            "direct worker jalr delay slot did not execute exactly once")
    require(machine.peek(SAVED_IMASK, 4) == old_mask and
            machine.peek(WORK_MARKER, 4) == 1,
            "direct worker prefix state mismatch")
    require(machine.bytes(RING_BASE, RING_SIZE) == before_ring,
            "direct worker prefix wrote the ring")
    require(machine.mmio_reads == [
        (DMA2_CHCR, 0x00000401),
        (GPUSTAT, GPUSTAT_READY_GP0),
    ], f"direct worker boundary MMIO reads {machine.mmio_reads}")
    writes = [(pc, address) for pc, address, _, _ in persistent_writes(machine)]
    require(writes == [
        (0x800773E8, WAIT_DEADLINE),
        (0x800773F0, WAIT_COUNTER),
        (0x80076CB4, SAVED_IMASK),
        (0x80076CC4, WORK_MARKER),
    ], f"direct worker prefix writes {writes}")
    print("scenario dependency-direct-worker: PASS exact-jalr-ABI no-result")


def scenario_direct(exe: Exe) -> None:
    vsync = 9
    old_mask = 0x0000A55A
    argument = 0x80102000
    auxiliary = 0x80123400
    machine = make_machine(
        exe, producer=0, consumer=0, vsync=vsync,
        actions={
            EXCHANGE_IMASK: [Action(old_mask), Action(0)],
            LOADIMAGE_WORKER: [Action(MASK)],
        },
    )
    before_ring = machine.bytes(RING_BASE, RING_SIZE)
    result = machine.run(LOADIMAGE_WORKER, argument, 8, auxiliary)
    machine.assert_actions_consumed()
    assert_wait_init(machine, vsync)
    require(result == 0, "direct path did not return zero")
    require(calls(machine) == [WAIT_INIT, VSYNC, EXCHANGE_IMASK,
                               LOADIMAGE_WORKER, EXCHANGE_IMASK],
            f"direct call order {calls(machine)}")
    first_mask, worker_call, restore = (
        machine.calls[2], machine.calls[3], machine.calls[4]
    )
    require(first_mask.args[0] == 0, "direct I_MASK acquire argument")
    require(worker_call.pc == 0x80076D38 and
            worker_call.args[:2] == (argument, auxiliary),
            "direct worker ABI/delay slot mismatch")
    require(restore.args[0] == old_mask, "direct I_MASK restore argument")
    require(machine.peek(SAVED_IMASK, 4) == old_mask,
            "direct saved I_MASK guest word mismatch")
    require(machine.peek(WORK_MARKER, 4) == 1,
            "direct marker delay-slot store missing")
    require(machine.peek(PRODUCER, 4) == 0 and machine.peek(CONSUMER, 4) == 0,
            "direct path changed queue indices")
    require(machine.bytes(RING_BASE, RING_SIZE) == before_ring,
            "direct path wrote the ring")
    require(machine.mmio_reads == [
        (DMA2_CHCR, 0x00000401),
        (GPUSTAT, GPUSTAT_READY_GP0),
    ], f"direct MMIO reads {machine.mmio_reads}")
    require(machine.pc_counts.get(0x80076D3C) == 1,
            "worker jalr delay slot count")
    writes = [(pc, address) for pc, address, _, _ in persistent_writes(machine)]
    require(writes == [
        (0x800773E8, WAIT_DEADLINE),
        (0x800773F0, WAIT_COUNTER),
        (0x80076CB4, SAVED_IMASK),
        (0x80076CC4, WORK_MARKER),
    ], f"direct persistent writes {writes}")
    print("scenario direct-worker: PASS worker-result-ignored return=0")


def enqueue_actions(old_mask: int) -> dict[int, list[Action]]:
    return {
        EXCHANGE_IMASK: [Action(old_mask), Action(0)],
        SET_DMA_CALLBACK: [Action(0)],
        QUEUE_PUMP: [Action(0)],
    }


def assert_enqueue_calls(machine: Machine, old_mask: int) -> None:
    require(calls(machine) == [WAIT_INIT, VSYNC, EXCHANGE_IMASK,
                               SET_DMA_CALLBACK, EXCHANGE_IMASK, QUEUE_PUMP],
            f"enqueue call order {calls(machine)}")
    setter = machine.calls[3]
    require(setter.pc == 0x80076D60 and
            setter.args[0] == 2 and setter.args[1] == QUEUE_PUMP,
            "DMA callback setter ABI/delay slot mismatch")
    require(machine.calls[4].args[0] == old_mask,
            "enqueue I_MASK restore argument")
    require(LOADIMAGE_WORKER not in calls(machine),
            "enqueue path invoked worker itself")


def scenario_enqueue_pump_dependency(exe: Exe) -> None:
    """Stop at pump only after the complete entry has been published."""
    vsync = 34
    old_mask = 0x1357
    slot = 12
    argument = 0x80102400
    auxiliary = 0x92345678
    payload = struct.pack("<II", 0x01020304, 0xA1B2C3D4)
    machine = make_machine(
        exe, producer=slot, consumer=slot, callback=0x80023450,
        vsync=vsync,
        actions={
            EXCHANGE_IMASK: [Action(old_mask), Action(0)],
            SET_DMA_CALLBACK: [Action(0)],
        },
        stop_at={QUEUE_PUMP},
    )
    machine.seed_bytes(argument, payload)
    before_ring = machine.bytes(RING_BASE, RING_SIZE)
    try:
        machine.run(LOADIMAGE_WORKER, argument, 8, auxiliary)
    except DependencyStop as stop:
        require(stop.call.pc == 0x80076EA4 and stop.call.target == QUEUE_PUMP,
                "enqueue prefix stopped at wrong pump dependency")
    else:
        raise SystemExit("FATAL: enqueue path fabricated a pump return")
    machine.assert_actions_consumed()
    assert_wait_init(machine, vsync)
    assert_enqueue_calls(machine, old_mask)
    require(QUEUE_PUMP not in machine.actions,
            "queue-pump boundary unexpectedly had a result action")
    entry = RING_BASE + slot * RING_ENTRY_SIZE
    want_ring = expected_ring(before_ring, slot, LOADIMAGE_WORKER,
                              entry + 0x0C, auxiliary, payload)
    require(machine.bytes(RING_BASE, RING_SIZE) == want_ring,
            "pump boundary entry was not fully written")
    require(machine.peek(PRODUCER, 4) == 13 and
            machine.peek(CONSUMER, 4) == 12,
            "pump boundary producer was not published")
    assert_enqueue_write_order(machine, slot, 8, 2)
    require(machine.pc_counts.get(0x80076EA0) == 1 and
            machine.pc_counts.get(0x80076EA8) == 1,
            "producer/pump delay-slot count mismatch")
    print("scenario dependency-enqueue-pump: PASS entry-published exact-order")


def scenario_empty_enqueue_copy8(exe: Exe) -> None:
    vsync = 41
    old_mask = 0x1234
    slot = 5
    argument = 0x80102000
    auxiliary = 0x80ABCDEF
    payload = struct.pack("<II", 0x11223344, 0xAABBCCDD)
    machine = make_machine(
        exe, producer=slot, consumer=slot, callback=0x80012344,
        vsync=vsync, actions=enqueue_actions(old_mask),
    )
    machine.seed_bytes(argument, payload)
    before_ring = machine.bytes(RING_BASE, RING_SIZE)
    result = machine.run(LOADIMAGE_WORKER, argument, 8, auxiliary)
    machine.assert_actions_consumed()
    assert_wait_init(machine, vsync)
    assert_enqueue_calls(machine, old_mask)
    entry = RING_BASE + slot * RING_ENTRY_SIZE
    want_ring = expected_ring(before_ring, slot, LOADIMAGE_WORKER,
                              entry + 0x0C, auxiliary, payload)
    require(machine.bytes(RING_BASE, RING_SIZE) == want_ring,
            "empty enqueue 8-byte ring image mismatch")
    require(machine.peek(PRODUCER, 4) == 6 and machine.peek(CONSUMER, 4) == 5,
            "empty enqueue indices mismatch")
    require(result == 1, f"empty enqueue pending result {result}")
    require(machine.mmio_reads == [(DMA2_CHCR, 0x00000401)],
            f"empty enqueue MMIO reads {machine.mmio_reads}")
    require(machine.peek(WORK_MARKER, 4) == 1,
            "empty enqueue marker missing")
    assert_enqueue_write_order(machine, slot, 8, 2)
    print("scenario empty-enqueue-copy8: PASS copied-inline pending=1")


def scenario_nonempty_enqueue_copy64(exe: Exe) -> None:
    vsync = 52
    old_mask = 0x5678
    slot = 2
    argument = 0x80102100
    auxiliary = 0xF1234567
    payload = b"".join(struct.pack("<I", 0xCA000000 + index)
                       for index in range(16))
    machine = make_machine(
        exe, producer=slot, consumer=0, vsync=vsync,
        actions=enqueue_actions(old_mask),
    )
    machine.seed_bytes(argument, payload)
    before_ring = machine.bytes(RING_BASE, RING_SIZE)
    result = machine.run(LOADIMAGE_WORKER, argument, 64, auxiliary)
    machine.assert_actions_consumed()
    assert_wait_init(machine, vsync)
    assert_enqueue_calls(machine, old_mask)
    entry = RING_BASE + slot * RING_ENTRY_SIZE
    want_ring = expected_ring(before_ring, slot, LOADIMAGE_WORKER,
                              entry + 0x0C, auxiliary, payload)
    require(machine.bytes(RING_BASE, RING_SIZE) == want_ring,
            "nonempty enqueue 64-byte ring image mismatch")
    require(machine.peek(PRODUCER, 4) == 3 and machine.peek(CONSUMER, 4) == 0,
            "nonempty enqueue indices mismatch")
    require(result == 3, f"nonempty enqueue pending result {result}")
    require(machine.mmio_reads == [],
            "nonempty queue should branch before DMA/GPU MMIO")
    require(machine.pc_counts.get(0x80076D94) == 17 and
            machine.pc_counts.get(0x80076D98) == 17,
            "64-byte copy loop/delay-slot count mismatch")
    assert_enqueue_write_order(machine, slot, 64, 16)
    print("scenario nonempty-enqueue-copy64: PASS words=16 pending=3")


def scenario_wrap_enqueue_copy0(exe: Exe) -> None:
    vsync = 63
    old_mask = 0x9ABC
    slot = 63
    argument = 0x80102200
    auxiliary = 0xFFFFFFFF
    machine = make_machine(
        exe, producer=slot, consumer=62, vsync=vsync,
        actions=enqueue_actions(old_mask),
    )
    before_ring = machine.bytes(RING_BASE, RING_SIZE)
    result = machine.run(LOADIMAGE_WORKER, argument, 0, auxiliary)
    machine.assert_actions_consumed()
    assert_wait_init(machine, vsync)
    assert_enqueue_calls(machine, old_mask)
    want_ring = expected_ring(before_ring, slot, LOADIMAGE_WORKER,
                              argument, auxiliary, None)
    require(machine.bytes(RING_BASE, RING_SIZE) == want_ring,
            "wrap enqueue no-copy ring image mismatch")
    require(machine.peek(PRODUCER, 4) == 0 and machine.peek(CONSUMER, 4) == 62,
            "producer did not wrap 63 -> 0")
    require(result == 2, f"wrapped pending result {result}")
    require(machine.pc_counts.get(0x80076D80, 0) == 0 and
            machine.pc_counts.get(0x80076D94, 0) == 0,
            "copy_bytes=0 entered copy loop")
    require(machine.mmio_reads == [],
            "wrap nonempty queue should not access DMA/GPU MMIO")
    assert_enqueue_write_order(machine, slot, 0, 0)
    print("scenario wrap-enqueue-copy0: PASS pointer-retained producer=0 pending=2")


def scenario_signed_copy_counts(exe: Exe) -> None:
    # Expected word counts are independently derived from the retail signed
    # adjust/SRA sequence, not Python's floor division:
    #   negative: (copy_bytes + 3, wrapping 32-bit) >> 2 arithmetically
    #   nonnegative: copy_bytes >> 2
    # Only a strictly positive quotient enters the slt-controlled loop.
    cases = [
        ("minus1", -1, 0),
        ("plus1", 1, 0),
        ("plus3", 3, 0),
        ("plus4", 4, 1),
        ("plus7", 7, 1),
        ("int32-min", -0x80000000, 0),
    ]

    for index, (label, copy_bytes, expected_words) in enumerate(cases):
        vsync = 70 + index
        old_mask = 0x4000 + index
        slot = 20 + index
        # An attempted lw for INT32_MIN must fail the oracle's guest-range
        # check. Passing with this address therefore proves zero source reads.
        argument = (0xFFFFFFFF if copy_bytes == -0x80000000
                    else 0x80103000 + index * 0x20)
        auxiliary = 0xA0000000 + index
        payload = struct.pack("<II", 0x51000000 + index,
                              0x62000000 + index)
        machine = make_machine(
            exe, producer=slot, consumer=slot, callback=0x80034560,
            vsync=vsync, actions=enqueue_actions(old_mask),
        )
        if argument != 0xFFFFFFFF:
            machine.seed_bytes(argument, payload)
        before_ring = machine.bytes(RING_BASE, RING_SIZE)
        result = machine.run(LOADIMAGE_WORKER, argument, copy_bytes, auxiliary)
        machine.assert_actions_consumed()
        assert_wait_init(machine, vsync)
        assert_enqueue_calls(machine, old_mask)

        entry = RING_BASE + slot * RING_ENTRY_SIZE
        copied = payload[:expected_words * 4]
        want_ring = expected_ring(before_ring, slot, LOADIMAGE_WORKER,
                                  entry + 0x0C, auxiliary, copied)
        require(machine.bytes(RING_BASE, RING_SIZE) == want_ring,
                f"signed copy {label} ring image mismatch")
        require(machine.peek(entry + 4, 4) == entry + 0x0C,
                f"signed copy {label} did not select inline argument")
        require(machine.peek(PRODUCER, 4) == slot + 1 and
                machine.peek(CONSUMER, 4) == slot,
                f"signed copy {label} publication/index mismatch")
        require(result == 1, f"signed copy {label} pending result {result}")
        assert_enqueue_write_order(machine, slot, copy_bytes, expected_words)

        copy_reads = [
            (address, size)
            for pc, address, size in machine.memory_reads
            if pc == 0x80076D9C
        ]
        expected_reads = ([(argument, 4)] if expected_words == 1 else [])
        require(copy_reads == expected_reads,
                f"signed copy {label} source reads {copy_reads} != "
                f"{expected_reads}")
        require(machine.pc_counts.get(0x80076D94) == expected_words + 1,
                f"signed copy {label} loop predicate count")
        print(f"scenario signed-copy-{label}: PASS bytes={copy_bytes} "
              f"words={expected_words} inline=1")


def main() -> int:
    if len(sys.argv) > 2:
        raise SystemExit(f"usage: {sys.argv[0]} [SHA-exact-SLUS_006.62]")
    path = pathlib.Path(sys.argv[1] if len(sys.argv) == 2
                        else "build/extracted/disc1/SLUS_006.62")
    exe = Exe(path)
    verify_static(exe)

    scenario_nonfull_dependency(exe)
    scenario_full_dependency(exe)
    scenario_timeout(exe)
    scenario_full_progress(exe)
    scenario_direct_worker_dependency(exe)
    scenario_direct(exe)
    scenario_enqueue_pump_dependency(exe)
    scenario_empty_enqueue_copy8(exe)
    scenario_nonempty_enqueue_copy64(exe)
    scenario_wrap_enqueue_copy0(exe)
    scenario_signed_copy_counts(exe)

    print("B53C ORACLE: PASS — literal dispatcher/helper executed; "
          "dependencies controlled; no hardware auto-progress")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
