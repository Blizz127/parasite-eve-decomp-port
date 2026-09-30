#!/usr/bin/env python3
"""PE-BTL6 independent oracle: 29810 remainder at 0x80029854.

Pins SHA-1-exact EXE. After jal 20EFC, 43 words before lui a0 for
71A64: sb/lw/and/sw D_8009D1AC (mask 0xFFFFFCFF), sw 0 D_8009D1A8,
sb 0 D_8009D1CE / D_8009D235, sw 0 gp+0x594 (D_8009D304), sh 0
gp+0x4AC (D_8009D21C), 10-wide {0,-1} halfword pairs at 0x800A7FF0
(sltiu 10), 7 words at 0x800B8A90 (lui 0x800C + signed addiu 0x8A90,
NOT 0x800C8A90). Then already-wired 71A64 / 293F4(0). Does not
import production C. Does not claim mode 7, overlay jalr, or 0x55
completion.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
START = 0x80029854
END = 0x80029900
SHA = "98e643b29ceaf2324189b3f561b873a2fdc74bc7f881c5d878b4e151a3776300"
GP = 0x8009CD70
FN_71A64 = 0x80071A64
FN_293F4 = 0x800293F4


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def window_sha(data: bytes, start: int, end: int) -> str:
    return hashlib.sha256(data[exe_off(start) : exe_off(end)]).hexdigest()


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")

    require((END - START) // 4 == 43, "43 words")
    require(window_sha(data, START, END) == SHA, "29854 sha")
    require(load_u32(data, START) == 0x00002021, "a0=0")
    require(load_u32(data, START + 4) == 0x2405FFFF, "a1=-1")
    require(load_u32(data, 0x80029860) == 0xA020D1AC, "sb D_8009D1AC")
    require(0x800A0000 - 0x2E54 == 0x8009D1AC, "D1AC")
    require(load_u32(data, 0x8002986C) == 0x2402FCFF, "addiu 0xFCFF")
    require(load_u32(data, 0x80029874) == 0xAC20D1A8, "sw D_8009D1A8")
    require(0x800A0000 - 0x2E58 == 0x8009D1A8, "D1A8")
    require(load_u32(data, 0x8002987C) == 0xA020D1CE, "sb D_8009D1CE")
    require(0x800A0000 - 0x2E32 == 0x8009D1CE, "D1CE")
    require(load_u32(data, 0x80029884) == 0xA020D235, "sb D_8009D235")
    require(0x800A0000 - 0x2DCB == 0x8009D235, "D235")
    require(load_u32(data, 0x80029888) == 0xAF800594, "sw gp+0x594")
    require(GP + 0x594 == 0x8009D304, "D304")
    require(load_u32(data, 0x8002988C) == 0xA78004AC, "sh gp+0x4AC")
    require(GP + 0x4AC == 0x8009D21C, "D21C")
    require(load_u32(data, 0x80029890) == 0x00621824, "and D1AC")
    require(load_u32(data, 0x80029898) == 0xAC23D1AC, "sw D1AC masked")
    require(load_u32(data, 0x800298B0) == 0xA4207FF0, "sh 0 0x800A7FF0")
    require(load_u32(data, 0x800298BC) == 0xA4257FF2, "sh -1 0x800A7FF2")
    require(load_u32(data, 0x800298C4) == 0x2C42000A, "sltiu 10")
    require(load_u32(data, 0x800298D4) == 0x3C03800C, "lui 0x800C")
    require(load_u32(data, 0x800298D8) == 0x24638A90, "addiu 0x8A90")
    base = 0x800C0000 + (0x8A90 - 0x10000)
    require(base == 0x800B8A90, f"7-word base {base:#x} not 0x800C8A90")
    require(load_u32(data, 0x800298E8) == 0xAC400000, "sw 0 table")
    require(load_u32(data, 0x800298F4) == 0x2C420007, "sltiu 7")
    require(jal_target(load_u32(data, 0x80029908)) == FN_71A64, "then 71A64")
    require(jal_target(load_u32(data, 0x80029910)) == FN_293F4, "then 293F4")
    require(load_u32(data, 0x80029914) == 0x00002021, "293F4 a0=0")

    print(
        "PASS: 29854 43w sha 98e643b2…6300; D1AC&=~0x300; "
        "A7FF0 x10 {0,-1}; B8A90 x7 zero; not C8A90; no mode7/0x55"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
