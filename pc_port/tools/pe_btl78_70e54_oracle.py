#!/usr/bin/env python3
"""PE-BTL78 — 70E54 live prefix on 3F3C4."""
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
    require(jal_target(load_u32(blob, 0x8003F590)) == 0x80070E54, "3F590")
    require((0x80070FAC - 0x80070E54) // 4 == 86, "70E54 86w")
    require(jal_target(load_u32(blob, 0x80070E5C)) == 0x80074DC0, "DrawSync")
    require(jal_target(load_u32(blob, 0x80070E64)) == 0x80042FE8, "42FE8")
    require(load_u32(blob, 0x80070E78) == 0x30420200, "andi 0x200")
    require(jal_target(load_u32(blob, 0x80070EB8)) == 0x80073A44, "VSync")
    require(load_u32(blob, 0x80070EBC) == 0x24040002, "VSync(2)")
    require((0x80043038 - 0x80042FE8) // 4 == 20, "42FE8 20w")
    require(
        window_sha(blob, 0x80070E54, 0x80070FAC)
        == "64c882e893f33f306cbd7ea90d1c2b6d43933bd6d5312f6079090ccd35310537",
        "70E54 sha",
    )
    print("PASS: 70E54 86w DrawSync 42FE8 VSync(2) on bit200 clear")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
