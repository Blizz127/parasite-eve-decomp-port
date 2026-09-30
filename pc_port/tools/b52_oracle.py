#!/usr/bin/env python3
"""Phase 6E-B52 retail oracle: LoadImage wrapper to GPU boundary.

The oracle verifies and executes the literal MIPS-I bodies of
func_8007506C and func_80074E28 from the SHA-exact retail executable.
func_80071A74 diagnostic calls are logged as read-only boundaries.  Execution
STOPS at the indirect func_80076C34 dispatch: no GPUSTAT, queue, GP0/GP1, or
DMA completion state is invented.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_words


SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
MASK = 0xFFFFFFFF
RAM_BASE = 0x80000000
RAM_SIZE = 0x200000

WRAPPER = 0x8007506C
WRAPPER_END = 0x800750CC
VALIDATOR = 0x80074E28
VALIDATOR_END = 0x80074F44
PRINT_FN = 0x80071A74
NAME = 0x800118D4
FMT_BAD = 0x80011898
FMT_RECT = 0x800118A4
FMT_NAME = 0x800118B8
DEBUG = 0x8009574E
LIMIT_W = 0x80095750
LIMIT_H = 0x80095752
JTB = 0x80095704
JTB_PTR = 0x80095744
DISPATCH = 0x80076C34
WORKER = 0x80076664
SP = 0x801FFF00
RA_SENTINEL = 0xDEADBEEC


WRAPPER_WORDS = exe_words(0x8007506C, 24)

VALIDATOR_WORDS = exe_words(0x80074E28, 71)

JTB_WORDS = exe_words(0x80095704, 16)


def u32(x: int) -> int:
    return x & MASK


def s32(x: int) -> int:
    x &= MASK
    return x - 0x100000000 if x & 0x80000000 else x


def sx16(x: int) -> int:
    x &= 0xFFFF
    return x - 0x10000 if x & 0x8000 else x


class Exe:
    def __init__(self, path: pathlib.Path):
        data = path.read_bytes()
        got = hashlib.sha1(data).hexdigest()
        if got != SHA1:
            raise SystemExit(f"FATAL: {path} SHA-1 {got} != {SHA1}")
        if data[:8] != b"PS-X EXE":
            raise SystemExit("FATAL: not a PS-X EXE")
        self.taddr, self.tsize = struct.unpack_from("<II", data, 0x18)
        if len(data) != 0x800 + self.tsize:
            raise SystemExit("FATAL: PS-X EXE size/header mismatch")
        self.image = data[0x800:]

    def word(self, addr: int) -> int:
        return struct.unpack_from("<I", self.image, addr - self.taddr)[0]


class StopAtGpu(Exception):
    pass


class Machine:
    def __init__(self, exe: Exe):
        self.ram = bytearray(RAM_SIZE)
        off = exe.taddr - RAM_BASE
        self.ram[off:off + exe.tsize] = exe.image
        self.events: list[tuple] = []
        self.writes: list[tuple[int, int]] = []
        self.pc_counts: dict[int, int] = {}
        self.call_stack: list[int] = []
        self.boundary: tuple | None = None

    @staticmethod
    def off(addr: int, size: int) -> int:
        off = addr - RAM_BASE
        if off < 0 or off + size > RAM_SIZE:
            raise SystemExit(f"FATAL: guest access 0x{addr:08X}+{size}")
        return off

    def fetch(self, addr: int) -> int:
        return struct.unpack_from("<I", self.ram, self.off(addr, 4))[0]

    def load(self, addr: int, size: int, signed: bool = False) -> int:
        off = self.off(addr, size)
        value = int.from_bytes(self.ram[off:off + size], "little",
                               signed=signed)
        value = u32(value)
        self.events.append(("read", addr, size, value))
        return value

    def store(self, addr: int, size: int, value: int) -> None:
        off = self.off(addr, size)
        self.ram[off:off + size] = (value & ((1 << (size * 8)) - 1)).to_bytes(
            size, "little")
        self.events.append(("write", addr, size, value & MASK))
        self.writes.append((addr, size))

    def one(self, r: list[int], pc: int) -> int | None:
        self.pc_counts[pc] = self.pc_counts.get(pc, 0) + 1
        w = self.fetch(pc)
        op, rs, rt = (w >> 26) & 0x3F, (w >> 21) & 31, (w >> 16) & 31
        rd, sa, imm = (w >> 11) & 31, (w >> 6) & 31, w & 0xFFFF
        simm = sx16(imm)
        if w == 0:
            return None
        if op == 0:
            fn = w & 0x3F
            if fn == 0x00:
                r[rd] = u32(r[rt] << sa)
            elif fn == 0x08:
                return r[rs]
            elif fn == 0x09:
                r[rd] = u32(pc + 8)
                return r[rs]
            elif fn == 0x21:
                r[rd] = u32(r[rs] + r[rt])
            elif fn == 0x2A:
                r[rd] = int(s32(r[rs]) < s32(r[rt]))
            else:
                raise SystemExit(f"FATAL: SPECIAL {fn:02X} @0x{pc:08X}")
            return None
        if op == 1:
            if rt != 0:
                raise SystemExit(f"FATAL: REGIMM rt={rt} @0x{pc:08X}")
            return u32(pc + 4 + (simm << 2)) if s32(r[rs]) < 0 else None
        if op == 2:
            return u32(((pc + 4) & 0xF0000000) | ((w & 0x3FFFFFF) << 2))
        if op == 3:
            r[31] = u32(pc + 8)
            return u32(((pc + 4) & 0xF0000000) | ((w & 0x3FFFFFF) << 2))
        if op == 4:
            return u32(pc + 4 + (simm << 2)) if r[rs] == r[rt] else None
        if op == 5:
            return u32(pc + 4 + (simm << 2)) if r[rs] != r[rt] else None
        if op == 6:
            return u32(pc + 4 + (simm << 2)) if s32(r[rs]) <= 0 else None
        if op == 7:
            return u32(pc + 4 + (simm << 2)) if s32(r[rs]) > 0 else None
        if op == 9:
            r[rt] = u32(r[rs] + simm)
        elif op == 0x0F:
            r[rt] = u32(imm << 16)
        elif op == 0x23:
            r[rt] = self.load(u32(r[rs] + simm), 4)
        elif op == 0x24:
            r[rt] = self.load(u32(r[rs] + simm), 1)
        elif op == 0x21:
            r[rt] = self.load(u32(r[rs] + simm), 2, signed=True)
        elif op == 0x2B:
            self.store(u32(r[rs] + simm), 4, r[rt])
        else:
            raise SystemExit(f"FATAL: opcode {op:02X} @0x{pc:08X}")
        return None

    def run_wrapper(self, rect: int, source: int) -> tuple:
        r = [0] * 32
        r[4], r[5], r[29], r[31] = rect, source, SP, RA_SENTINEL
        pc = WRAPPER
        for _ in range(2000):
            w = self.fetch(pc)
            op, fn = (w >> 26) & 0x3F, w & 0x3F
            control = op in (1, 2, 3, 4, 5, 6, 7) or (op == 0 and fn in (8, 9))
            if not control:
                self.one(r, pc)
                r[0] = 0
                pc = u32(pc + 4)
                continue
            old_ra = r[31]
            target = self.one(r, pc)       # predicate/target sampled first
            linked = r[31]
            self.one(r, u32(pc + 4))       # architectural delay slot once
            r[0] = 0
            if op == 3 and target == VALIDATOR:
                self.events.append(("call", WRAPPER, VALIDATOR, r[4], r[5]))
                self.call_stack.append(linked)
                pc = target
                continue
            if op == 0 and fn == 9 and target == PRINT_FN:
                fifth = self.load(u32(r[29] + 0x10), 4)
                self.events.append(("print", r[4], r[5], r[6], r[7], fifth))
                r[2] = 0
                pc = u32(pc + 8)
                continue
            if op == 0 and fn == 9 and target == DISPATCH:
                rect_words = tuple(sx16(self.load(r[5] + i * 2, 2))
                                   for i in range(4))
                self.boundary = ("boundary", target, r[4], r[5], r[6],
                                 r[7], rect_words)
                self.events.append(self.boundary)
                raise StopAtGpu
            if op == 0 and fn == 8 and self.call_stack and target == old_ra:
                self.events.append(("return", VALIDATOR, r[2]))
                self.call_stack.pop()
            if target is None:
                pc = u32(pc + 8 if control else pc + 4)
            elif target == RA_SENTINEL:
                self.events.append(("return", WRAPPER, r[2]))
                return self.events[-1]
            else:
                pc = target
        raise SystemExit("FATAL: interpreter did not terminate")


def verify_static(exe: Exe) -> None:
    for base, words, name in (
        (WRAPPER, WRAPPER_WORDS, "func_8007506C"),
        (VALIDATOR, VALIDATOR_WORDS, "func_80074E28"),
    ):
        for i, want in enumerate(words):
            got = exe.word(base + i * 4)
            if got != want:
                raise SystemExit(f"FATAL: {name} word {i}: {got:08X} != {want:08X}")
    if len(WRAPPER_WORDS) != 24 or WRAPPER_END - WRAPPER != 0x60:
        raise SystemExit("FATAL: wrapper range/count")
    if len(VALIDATOR_WORDS) != 71 or VALIDATOR_END - VALIDATOR != 0x11C:
        raise SystemExit("FATAL: validator range/count")
    for i, want in enumerate(JTB_WORDS):
        if exe.word(JTB + i * 4) != want:
            raise SystemExit(f"FATAL: jtb[{i}] mismatch")
    if exe.word(JTB_PTR) != JTB or exe.word(JTB_PTR + 4) != PRINT_FN:
        raise SystemExit("FATAL: jtb/printf pointer initialization")
    for i, w in enumerate(VALIDATOR_WORDS):
        if (w >> 26) & 0x3F in (0x28, 0x29, 0x2A, 0x2B, 0x2E):
            if (w >> 21) & 31 != 29:
                raise SystemExit(f"FATAL: persistent validator store at word {i}")
    raw = exe.image
    calls = {VALIDATOR: [], WRAPPER: []}
    for off in range(0, len(raw) - 3, 4):
        w = struct.unpack_from("<I", raw, off)[0]
        if w >> 26 != 3:
            continue
        pc = exe.taddr + off
        target = ((pc + 4) & 0xF0000000) | ((w & 0x3FFFFFF) << 2)
        if target in calls:
            calls[target].append(pc)
    if len(calls[VALIDATOR]) != 8 or len(calls[WRAPPER]) != 14:
        raise SystemExit("FATAL: caller census changed")
    name_off = NAME - exe.taddr
    if raw[name_off:raw.index(b"\0", name_off)] != b"LoadImage":
        raise SystemExit("FATAL: LoadImage string evidence")
    print(f"SHA-1 verified: {SHA1}")
    print("func_8007506C: 24 words, 0x8007506C..0x800750CC, file 0x6586C")
    print("func_80074E28: 71 words, 0x80074E28..0x80074F44, stack stores only")
    print("caller census: validator=8 direct sites, LoadImage=14 direct sites")
    print("jtb[0..15] verified; D_80095744=0x80095704, "
          "jtb[2]=0x80076C34, jtb[8]=0x80076664")


def run_case(exe: Exe, label: str, rect_values: tuple[int, int, int, int],
             source: int, debug: int, limits: tuple[int, int],
             expected_prints: list[tuple[int, int, int, int, int]]) -> None:
    m = Machine(exe)
    rect = 0x80102000
    for i, value in enumerate(rect_values):
        m.store(rect + i * 2, 2, value)
    m.store(DEBUG, 1, debug)
    m.store(LIMIT_W, 2, limits[0])
    m.store(LIMIT_H, 2, limits[1])
    m.events.clear()
    m.writes.clear()
    before = bytes(m.ram)
    try:
        m.run_wrapper(rect, source)
    except StopAtGpu:
        pass
    else:
        raise SystemExit(f"FATAL [{label}]: did not stop at GPU boundary")
    if m.boundary != ("boundary", DISPATCH, WORKER, rect, 8,
                      source & MASK, rect_values):
        raise SystemExit(f"FATAL [{label}]: boundary {m.boundary}")
    prints = [e[1:] for e in m.events if e[0] == "print"]
    if prints != expected_prints:
        raise SystemExit(f"FATAL [{label}]: prints {prints} != {expected_prints}")
    flow = [e[0:2] for e in m.events if e[0] in ("call", "return", "boundary")]
    if flow != [("call", WRAPPER), ("return", VALIDATOR),
                ("boundary", DISPATCH)]:
        raise SystemExit(f"FATAL [{label}]: ordered call/return flow {flow}")
    reads = [e[1] for e in m.events if e[0] == "read"]
    dispatch_reads = [a for a in reads if a in (JTB_PTR, JTB + 0x20, JTB + 8)]
    if dispatch_reads != [JTB_PTR, JTB + 0x20, JTB + 8]:
        raise SystemExit(f"FATAL [{label}]: dispatch reads {dispatch_reads}")
    if m.pc_counts.get(0x80075090) != 1 or m.pc_counts.get(0x800750B4) != 1:
        raise SystemExit(f"FATAL [{label}]: delay-slot count")
    for off in range(0, RAM_SIZE, 4):
        addr = RAM_BASE + off
        if SP - 0x40 <= addr < SP:
            continue
        if m.ram[off:off + 4] != before[off:off + 4]:
            raise SystemExit(f"FATAL [{label}]: persistent write 0x{addr:08X}")
    print(f"scenario {label}: PASS rect={rect_values} source=0x{source & MASK:08X} "
          f"prints={len(prints)} boundary=0x{DISPATCH:08X}")


def main() -> int:
    path = pathlib.Path(sys.argv[1] if len(sys.argv) > 1
                        else "build/extracted/disc1/SLUS_006.62")
    exe = Exe(path)
    verify_static(exe)
    run_case(exe, "valid-image", (0, 0, 320, 240), 0x80123456,
             0, (1024, 512), [])
    run_case(exe, "alternate-limit-edge", (0, 0, 1024, 512), 0x80000000,
             1, (1024, 512), [])
    run_case(exe, "invalid-validator", (-1, 2, 0, 600), 0x81234567,
             1, (1024, 512), [
                 (FMT_BAD, NAME, 600, u32(-1), 0),
                 (FMT_RECT, u32(-1), 2, 0, 600),
             ])
    run_case(exe, "signed-coordinate-edge", (-32768, 32767, -1, 1),
             0xFFFFFFFF, 2, (1024, 512), [
                 (FMT_NAME, NAME, 0, 0, 0),
                 (FMT_RECT, u32(-32768), 32767, u32(-1), 1),
             ])
    run_case(exe, "source-address-edge", (1, 2, 3, 4), 0xFFFFFFFF,
             0, (1024, 512), [])
    print("B52 ORACLE: PASS — literal wrapper+validator executed; "
          "STOP at func_80076C34; no GPU/DMA state modeled")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
