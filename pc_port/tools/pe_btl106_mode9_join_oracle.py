#!/usr/bin/env python3
"""PE-BTL106 — mode 9 falls to 2AA24; 2B0E8 stores 9; no dest write."""
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
    return 0x80000000 | ((word & 0x3FFFFFF) << 2)


def br_target(pc: int, word: int) -> int:
    imm = word & 0xFFFF
    if imm & 0x8000:
        imm -= 0x10000
    return pc + 4 + imm * 4


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require(load_u32(blob, 0x8002AA00) == 0x24020008, "li 8")
    require(br_target(0x8002AA14, load_u32(blob, 0x8002AA14)) == 0x8002AA24,
            "mode!=8 → 2AA24")
    require(jal_target(load_u32(blob, 0x8002AA1C)) == 0x8002DC58, "mode8 2DC58")
    require(load_u32(blob, 0x8002AA24) == 0x3C02800A, "2AA24 lui D1CE")
    require(load_u32(blob, 0x8002AA28) == 0x9042D1CE, "lbu D1CE")
    require(load_u32(blob, 0x8002AA50) == 0x938204D4, "lbu 4D4")
    require(load_u32(blob, 0x8002AA68) == 0x87820534, "lh 534")
    require(jal_target(load_u32(blob, 0x8002AA78)) == 0x80067CBC, "67CBC")
    require(load_u32(blob, 0x8002AA90) == 0x03E00008, "2AA90 jr")
    require(load_u32(blob, 0x8002B278) == 0x24030009, "2B0E8 li 9")
    require(load_u32(blob, 0x8002B27C) == 0xAF83051C, "2B0E8 sw 9")
    require(load_u32(blob, 0x80029A68) == 0xA7820534, "299CC sh 534")
    require(jal_target(load_u32(blob, 0x80029A5C)) == 0x8005C498, "5C498")
    print("PASS: mode 9 → 2AA24 join; 534 from 5C498; no dest store")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
