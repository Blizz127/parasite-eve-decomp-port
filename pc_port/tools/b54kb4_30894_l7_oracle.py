#!/usr/bin/env python3
"""Independent B54K-B4 oracle for func_80030894's complete L7 group."""

import hashlib
import pathlib
import struct
import sys

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BASE = 0x8000F800
START, END, WORDS = 0x80030F6C, 0x800310A4, 78
WIN_SHA = "0b3149236b8f26a7cc5614dd4179039f1f33824a4c3618fef4cf64eca551659d"
MODEL_BYTES = 87
MODEL_SHA = "db066c0811940babcc12af7bbc2d8759590686821f2cd05038bf2f8b49acdc87"
JALS = [
    (0x80030F88, 0x800370DC),
    (0x80030FD4, 0x80077C64),
    (0x80030FE4, 0x80077C64),
    (0x80031020, 0x80077B64),
    (0x80031058, 0x800370DC),
]
CRITICAL = {
    0x80030F68: 0x32D100FF, 0x80030F6C: 0x3C10800A,
    0x80030F70: 0x2610E460, 0x80030F7C: 0x001588C0,
    0x80030F80: 0x02358823, 0x80030F84: 0x00118880,
    0x80030F88: 0x0C00DC37, 0x80030F90: 0x00159140,
    0x80030FD4: 0x0C01DF19, 0x80030FE4: 0x0C01DF19,
    0x80030FEC: 0x00158080, 0x80030FF0: 0x02158021,
    0x80030FF4: 0x00108080, 0x80031020: 0x0C01DED9,
    0x80031028: 0x0000B021, 0x8003102C: 0x02158021,
    0x80031030: 0x00108880, 0x80031040: 0x32C200FF,
    0x80031044: 0x000280C0, 0x80031048: 0x02028023,
    0x8003104C: 0x00108080, 0x80031058: 0x0C00DC37,
    0x8003108C: 0x2C420003, 0x8003109C: 0x1440FFE8,
    0x800310A0: 0xA2170006, 0x800310A4: 0x0000B021,
}


def need(ok, message):
    if not ok:
        raise SystemExit("FAIL: " + message)


def find_exe():
    here = pathlib.Path(__file__).resolve()
    for path in (here.parent.parent.parent / "build/disc1.candidate.exe",
                 pathlib.Path("build/disc1.candidate.exe")):
        if path.is_file():
            return path
    raise SystemExit("FAIL: retail executable missing")


class Model:
    def __init__(self):
        self.b = {}

    def p8(self, a, v):
        self.b[a] = v & 0xFF

    def p16(self, a, v):
        self.p8(a, v); self.p8(a + 1, v >> 8)

    def p32(self, a, v):
        for i in range(4):
            self.p8(a + i, v >> (i * 8))

    def compound(self, h, width, height, rgb=None, u=None):
        self.p8(h + 3, 6); self.p32(h + 4, 0xE1000234)
        self.p32(h + 8, 0); self.p8(h + 15, 0x64)
        if rgb is not None:
            for o in (12, 13, 14): self.p8(h + o, rgb)
        if u is not None:
            self.p8(h + 20, u); self.p8(h + 21, 0xE0)
        self.p16(h + 22, 0x7E13)
        self.p16(h + 24, width); self.p16(h + 26, height)


def expected():
    m = Model()
    m.compound(0x8009E460, 24, 24, u=0xE8)
    for h, color in ((0x8009E498, 0xE0), (0x8009E4A8, 0x60)):
        m.p8(h + 3, 3); m.p8(h + 7, 0x40)
        for o in (4, 5, 6): m.p8(h + o, color)
    m.p8(0x8009E4D8 + 3, 4); m.p8(0x8009E4D8 + 7, 0x20)
    for j in range(3):
        m.compound(0x8009E3B8 + j * 28, 24, 8, rgb=0x80)
    return m


def run():
    data = find_exe().read_bytes()
    need(hashlib.sha1(data).hexdigest() == EXE_SHA1, "exe SHA-1")
    word = lambda a: struct.unpack_from("<I", data, a - BASE)[0]
    win = data[START - BASE:END - BASE]
    need(len(win) == WORDS * 4 and hashlib.sha256(win).hexdigest() == WIN_SHA,
         "full L7 window")
    print(f"OK window: {WORDS} words / {len(win):#x} bytes, SHA-256 exact")
    for a, value in CRITICAL.items():
        need(word(a) == value, f"word {a:#x}")
    print(f"OK boundaries/scales/calls: {len(CRITICAL)} instruction-exact words")
    calls, branches = [], []
    for a in range(START, END, 4):
        w, op = word(a), word(a) >> 26
        if op == 3:
            calls.append((a, 0x80000000 | ((w & 0x03FFFFFF) << 2)))
        if op in (1, 2, 4, 5, 6, 7, 20, 21, 22, 23):
            d = w & 0xFFFF
            if d & 0x8000: d -= 0x10000
            branches.append((a, a + 4 + d * 4))
    need(calls == JALS, "jal census/order")
    need(branches == [(0x8003109C, 0x80031040)], "branch census")
    print("OK control flow: five native jal sites; one three-item back-edge")
    model = expected()
    packed = b"".join(struct.pack("<IB", a, v)
                      for a, v in sorted(model.b.items()))
    digest = hashlib.sha256(packed).hexdigest()
    print(f"model_unique_written_bytes={len(model.b)}")
    print(f"model_write_map_sha256={digest}")
    need(len(model.b) == MODEL_BYTES and digest == MODEL_SHA, "model digest")
    need(0x8009E503 not in model.b, "crossed next group")
    print("OK independent model: compound/pair/PolyF3/three-item array")
    return 0


if __name__ == "__main__":
    sys.exit(run())
