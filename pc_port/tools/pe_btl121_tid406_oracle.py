#!/usr/bin/env python3
"""PE-BTL121 — 26824 publishes D2A4 to BE830 slot+4; 512AC(1) is *a1+387."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
ROOT = pathlib.Path(__file__).resolve().parents[2]
CANDIDATES = (
    ROOT / "build" / "disc1.candidate.exe",
    ROOT / "build" / "extracted" / "disc1" / "SLUS_006.62",
)
TADDR = 0x80010000
HDR = 0x800
GP = 0x8009CD70


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(va: int) -> int:
    return va - TADDR + HDR


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, exe_off(va))[0]


def jal_target(word: int) -> int:
    return 0x80000000 | ((word & 0x3FFFFFF) << 2)


def main() -> int:
    exe = next((p for p in CANDIDATES if p.is_file()), None)
    require(exe is not None, "missing retail EXE")
    blob = exe.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")

    require(0x8009D2A4 - GP == 0x534, "D2A4 is gp+0x534")
    require(load_u32(blob, 0x80029A5C) >> 26 == 3, "299CC jal")
    require(jal_target(load_u32(blob, 0x80029A5C)) == 0x8005C498, "299CC 5C498")
    require(load_u32(blob, 0x80029A68) == 0xA7820534, "sh v0, 534(gp)")
    require(load_u32(blob, 0x80035684) == 0xA422D2A4, "35558 sh D2A4")

    require(load_u32(blob, 0x80020F7C) == 0xA420E834, "20F7C sh zero BE834")
    require(load_u32(blob, 0x80026FA0) == 0xA420E834, "26FA0 sh zero BE834")

    require(load_u32(blob, 0x80026824) == 0x27BDFFD0, "26824 prologue")
    require(load_u32(blob, 0x8002684C) == 0x8610D2A4, "lh D2A4")
    require(load_u32(blob, 0x8002685C) == 0x2A020183, "slti 387")
    require(load_u32(blob, 0x80026864) == 0x2A020197, "slti 407")
    require(load_u32(blob, 0x800108AC + 13 * 4) == 0x8002692C, "jtbl[13] 406")
    require(load_u32(blob, 0x80026934) == 0xA38004EC, "406 D25C=0")
    require(load_u32(blob, 0x800269CC) == 0xA6020004, "406 sh D2A4 slot+4")
    require(load_u32(blob, 0x80026A34) == 0xA4460004, "269F4 sh D2A4 slot+4")
    require(jal_target(load_u32(blob, 0x8002692C)) == 0x80026FD0, "406 26FD0")

    require(load_u32(blob, 0x80011170 + 4) == 0x8005130C, "512AC jtbl[1]")
    require(load_u32(blob, 0x80051314) == 0x24420183, "addiu 387")
    require(load_u32(blob, 0x800514CC) == 0xAF8202A0, "sw D010")
    require(load_u32(blob, 0x800514A4) == 0x240203E8, "case10 li 1000")

    require(jal_target(load_u32(blob, 0x800264C0)) == 0x80026824, "25EEC 26824")
    require(load_u32(blob, 0x80026230) == 0x83840480, "lb D1F0 a0")
    require(load_u32(blob, 0x80026238) == 0x148000A1, "D1F0!=0 → 264C0")

    print("PASS: 26824 copies D2A4 to BE830+4; 512AC(1) writes *a1+387")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
