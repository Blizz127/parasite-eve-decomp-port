#!/usr/bin/env python3
"""PE-BTL71 — 3EB04 Up/Right through A76F0, not planted D26C."""
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


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require(jal_target(load_u32(blob, 0x8003F40C)) == 0x8003EB04, "3F3C4 3EB04")
    require(jal_target(load_u32(blob, 0x8003E9D8)) == 0x8003EAC8, "3EAC8 Up")
    require(load_u32(blob, 0x8003E9D4) == 0x24040008, "a0=bit3")
    require(load_u32(blob, 0x8003E9DC) == 0x24050010, "a1=Up 0x10")
    require(jal_target(load_u32(blob, 0x8003E9FC)) == 0x8003EAC8, "3EAC8 Right")
    require(load_u32(blob, 0x8003E9F8) == 0x24040010, "a0=bit4")
    require(load_u32(blob, 0x8003EA00) == 0x24050020, "a1=Right 0x20")
    require(load_u32(blob, 0x800943C0 + 21 * 4) == 0x800942F0, "row21")
    require(load_u32(blob, 0x800943C0 + 22 * 4) == 0x80094320, "row22")
    require(load_u32(blob, 0x8009430C) == 0x800710A4, "row21 710A4")
    require(load_u32(blob, 0x8009433C) == 0x800710A4, "row22 710A4")
    print("PASS: 3EB04 via BE9A2; A76F0 Up=0x10 Right=0x20; row21/22 710A4")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
