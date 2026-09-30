#!/usr/bin/env python3
"""PE-BTL65 — 0xB8/13514 ROM contract."""
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
    require(load_u32(blob, 0x800910A0 + 0xB8 * 4) == 0x80013514, "table[0xB8]")
    require((0x800136C0 - 0x80013514) // 4 == 107, "13514 107w")
    require(
        window_sha(blob, 0x80013514, 0x800136C0)
        == "573a82c682e9db1e5d04b7d674aa557c8cf84d097ed914f7ac3df80d31dc4ee7",
        "13514 sha",
    )
    require(load_u32(blob, 0x8001352C) == 0x30E20020, "andi bit 0x20")
    require(jal_target(load_u32(blob, 0x800135A4)) == 0x80079FB4, "jal 79FB4")
    require(load_u32(blob, 0x800135A8) == 0x00C02821, "a1=dx")
    require(load_u32(blob, 0x800135AC) == 0x24031400, "li 0x1400")
    require(load_u32(blob, 0x80013680) == 0x2463FFEC, "CE00-0x14")
    require(load_u32(blob, 0x80013674) == 0x00001021, "unfinished v0=0")
    require(load_u32(blob, 0x800136A4) == 0x3063FFDF, "clear bit 0x20")
    print("PASS: 0xB8/13514 jal 79FB4; on-point skip; unfinished v0=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
