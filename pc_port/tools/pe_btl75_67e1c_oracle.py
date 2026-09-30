#!/usr/bin/env python3
"""PE-BTL75 — 67E1C camera-slot interpolate."""
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
    require((0x80068014 - 0x80067E1C) // 4 == 126, "67E1C 126w")
    require(jal_target(load_u32(blob, 0x80068CF8)) == 0x80067E1C, "68CF8")
    require(load_u32(blob, 0x80067E1C) == 0x3C02800A, "lui 800a")
    require(load_u32(blob, 0x80067E20) == 0x8C42D1A0, "lw D1A0")
    require(load_u32(blob, 0x80067E28) == 0x30420104, "andi 0x104")
    require(load_u32(blob, 0x80067E38) == 0x3C02800B, "lui B1624")
    require(load_u32(blob, 0x80067E3C) == 0x8C421624, "lw B1624")
    require(load_u32(blob, 0x80067E4C) == 0x944B0006, "lhu count+6")
    require(load_u32(blob, 0x80067E48) == 0x8C430014, "lw +0x14")
    require(load_u32(blob, 0x80067E70) == 0x30420004, "andi bit4")
    require(load_u32(blob, 0x80067F34) == 0x30420008, "andi bit8")
    require(load_u32(blob, 0x80067FBC) == 0x25080038, "stride 56")
    require(load_u32(blob, 0x80067FC4) == 0x24C6CF88, "addiu BCF88")
    require(load_u32(blob, 0x80067FD0) == 0x30A20080, "andi 0x80")
    require(load_u32(blob, 0x80067E60) == 0x254ACF8C, "addiu BCF8C")
    require(
        window_sha(blob, 0x80067E1C, 0x80068014)
        == "99f90f7f78d13d6ac4ff441b07eaf674192eee1c5a94f7d743107aae93c83d6f",
        "67E1C sha",
    )
    print("PASS: 67E1C 126w D1A0&0x104 B1624 stride56 BCF88&0x80")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
