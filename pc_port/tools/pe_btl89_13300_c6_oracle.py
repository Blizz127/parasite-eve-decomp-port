#!/usr/bin/env python3
"""PE-BTL89 — 0xC6 / 13300 command-wait."""
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
    require(load_u32(blob, 0x800910A0 + 0xC6 * 4) == 0x80013300, "table[0xC6]")
    require((0x800133E8 - 0x80013300) // 4 == 58, "13300 58w")
    require(jal_target(load_u32(blob, 0x80013334)) == 0x8001A680, "1A680")
    require(load_u32(blob, 0x80013314) == 0x30A20020, "andi 0x20")
    require(load_u32(blob, 0x800133C8) == 0x2463FFF4, "CE00-0xC")
    require(
        window_sha(blob, 0x80013300, 0x800133E8)
        == "27e1042f5797b7ce7c0a4a03616a8f83490851fb76652daf2ae5a30b3e8b81a9",
        "13300 sha",
    )
    print("PASS: 0xC6 13300 58w jal 1A680; bit 0x20 wait")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
