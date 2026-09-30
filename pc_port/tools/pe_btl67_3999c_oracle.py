#!/usr/bin/env python3
"""PE-BTL67 — 3999C / 35C84 ROM contract."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
ROOT = pathlib.Path(__file__).resolve().parents[2]
EXE = ROOT / "build" / "disc1.candidate.exe"
TADDR = 0x80010000
HDR = 0x800


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(va: int) -> int:
    return va - TADDR + HDR


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, exe_off(va))[0]


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def window_sha(blob: bytes, start: int, end: int) -> str:
    return hashlib.sha256(blob[exe_off(start) : exe_off(end)]).hexdigest()


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require((0x80039B74 - 0x8003999C) // 4 == 118, "3999C 118w")
    require(
        window_sha(blob, 0x8003999C, 0x80039B74)
        == "9a5267b0b94ee61591e3b17e12616a656af83802fcc3af60a623e629c5fa3f6f",
        "3999C sha",
    )
    require(jal_target(load_u32(blob, 0x80035D14)) == 0x8003999C, "35C84 jal")
    require(load_u32(blob, 0x80035D10) == 0x24A543C0, "a1 0x43C0")
    require(load_u32(blob, 0x80035D18) == 0x27A60010, "a2 sp+0x10")
    require(load_u32(blob, 0x800399AC) == 0x00C09021, "s2=a2")
    require(load_u32(blob, 0x800399C0) == 0x8E420000, "lw *codep")
    require(load_u32(blob, 0x800399C8) == 0x00021080, "sll 2")
    require(load_u32(blob, 0x800943C0 + 5 * 4) == 0x80094350, "row5")
    require(load_u32(blob, 0x800943C0 + 21 * 4) == 0x800942F0, "row21")
    require(load_u32(blob, 0x80094350 + 0xC) == 0x8007136C, "row5 fn")
    print("PASS: 3999C 118w; 35C84 jal; table[5/21] → 7136C")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
