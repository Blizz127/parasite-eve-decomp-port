#!/usr/bin/env python3
"""Read-only predecode oracle for func_80030894's next (L10) unit.

This deliberately imports no production implementation.  It checks the exact
retail window, derives its complete control-flow/call census, evaluates the
GetClut variant from its proven arithmetic, and independently models the
bank-zero guest-memory write footprint that a future native translation must
reproduce.
"""

import hashlib
import pathlib
import struct
import sys


EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BASE = 0x8000F800
START, END, WORDS = 0x800311EC, 0x80031320, 77
WINDOW_SHA256 = "b8eafebde2564d8c3315c39c6b7e37ae8fe04036c525f8bdd2d12a761d0e86e2"
MODEL_BYTES = 74
MODEL_SHA256 = "cad997d53fb5fd3a345a130431eda562573d8ae848cb0b90e2395664b3138da4"

JALS = [
    (0x80031208, 0x800370DC),
    (0x8003122C, 0x80077AA4),
    (0x80031268, 0x800370DC),
    (0x800312E0, 0x800370DC),
]

CRITICAL = {
    0x800311E8: 0xA2170006,  # preceding L9 delay-slot store
    0x800311EC: 0x3C10800A,  # L10 start: lui s0,0x800A
    0x80031208: 0x0C00DC37,  # jal func_800370DC
    0x80031210: 0x24040130,  # GetClut x = 0x130
    0x80031214: 0x240501F9,  # GetClut y = 0x1F9
    0x8003122C: 0x0C01DEA9,  # jal func_80077AA4
    0x80031268: 0x0C00DC37,  # jal func_800370DC
    0x80031274: 0x001398C0,  # bank*56 continuation
    0x800312E0: 0x0C00DC37,  # jal func_800370DC
    0x80031314: 0x2C420002,  # literal two-item bound
    0x80031318: 0x1440FFEC,  # back-edge to 0x800312CC
    0x8003131C: 0x32C200FF,  # delay-slot counter mask
    0x80031320: 0x3C12800A,  # next unit (L11) start
}


def need(ok, message):
    if not ok:
        raise SystemExit("FAIL: " + message)


def get_clut(x, y):
    """Retail func_80077AA4 arithmetic, independently restated."""
    return ((y << 6) | ((x >> 4) & 0x3F)) & 0xFFFF


class Model:
    def __init__(self):
        self.bytes = {}

    def p8(self, addr, value):
        self.bytes[addr] = value & 0xFF

    def p16(self, addr, value):
        self.p8(addr, value)
        self.p8(addr + 1, value >> 8)

    def p32(self, addr, value):
        for i in range(4):
            self.p8(addr + i, value >> (8 * i))

    def compound(self, head, clut, width, height, rgb=None, uv=None):
        # Final successful state of func_800370DC(head, tpage_sprt).
        self.p8(head + 3, 6)
        self.p32(head + 4, 0xE1000234)
        self.p32(head + 8, 0)
        self.p8(head + 15, 0x64)
        if rgb is not None:
            for off in (12, 13, 14):
                self.p8(head + off, rgb)
        if uv is not None:
            self.p8(head + 20, uv[0])
            self.p8(head + 21, uv[1])
        self.p16(head + 22, clut)
        self.p16(head + 24, width)
        self.p16(head + 26, height)


def expected_model():
    model = Model()
    model.compound(0x8009E730, get_clut(0x130, 0x1F9), 24, 4,
                   rgb=0x80, uv=(0x68, 0xF4))
    model.compound(0x8009E880, 0x7E13, 36, 5,
                   rgb=0x80, uv=(0x7C, 0xEF))
    for j in range(2):
        model.compound(0x8009E8B8 + j * 28, 0x7E13, 6, 6)
    return model


def run():
    root = pathlib.Path(__file__).resolve().parent.parent.parent
    path = root / "build/disc1.candidate.exe"
    data = path.read_bytes()
    need(hashlib.sha1(data).hexdigest() == EXE_SHA1, "retail executable SHA-1")

    word = lambda addr: struct.unpack_from("<I", data, addr - BASE)[0]
    window = data[START - BASE:END - BASE]
    need(len(window) == WORDS * 4, "L10 window size")
    need(hashlib.sha256(window).hexdigest() == WINDOW_SHA256,
         "L10 window SHA-256")
    print(f"OK window: {WORDS} words / {len(window):#x} bytes, SHA-256 exact")

    for addr, value in CRITICAL.items():
        need(word(addr) == value, f"critical word {addr:#x}")
    print(f"OK boundaries/constants: {len(CRITICAL)} instruction-exact words")

    calls, branches = [], []
    for addr in range(START, END, 4):
        insn = word(addr)
        opcode = insn >> 26
        if opcode == 3:
            calls.append((addr, 0x80000000 | ((insn & 0x03FFFFFF) << 2)))
        if opcode in (1, 2, 4, 5, 6, 7, 20, 21, 22, 23):
            disp = insn & 0xFFFF
            if disp & 0x8000:
                disp -= 0x10000
            branches.append((addr, addr + 4 + disp * 4))
    need(calls == JALS, "jal census/order")
    need(branches == [(0x80031318, 0x800312CC)], "branch census")
    print("OK control flow: four native jal sites; one two-item back-edge")

    clut = get_clut(0x130, 0x1F9)
    need(clut == 0x7E53, "GetClut(0x130,0x1F9)")
    print("OK computed value: GetClut(0x130,0x1F9) = 0x7E53")

    model = expected_model()
    packed = b"".join(struct.pack("<IB", addr, value)
                      for addr, value in sorted(model.bytes.items()))
    digest = hashlib.sha256(packed).hexdigest()
    print(f"model_unique_written_bytes={len(model.bytes)}")
    print(f"model_write_map_sha256={digest}")
    need(len(model.bytes) == MODEL_BYTES, "model byte count")
    need(digest == MODEL_SHA256, "model digest")

    ranges = ((0x8009E730, 0x8009E74C, 21),
              (0x8009E880, 0x8009E89C, 21),
              (0x8009E8B8, 0x8009E8F0, 32))
    for lo, hi, count in ranges:
        actual = sum(lo <= addr < hi for addr in model.bytes)
        need(actual == count, f"write count in {lo:#x}..{hi:#x}")
        need(hi not in model.bytes, f"crossed bank boundary {hi:#x}")
    print("OK write bounds: 21 + 21 + 32 bytes; all bank-1 bases untouched")
    return 0


if __name__ == "__main__":
    sys.exit(run())
