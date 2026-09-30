#!/usr/bin/env python3
"""PE-BTL86 — 696F0 live tail after D1A0&0x80 skip."""
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
    require((0x800698D4 - 0x800696F0) // 4 == 121, "696F0 121w")
    require(jal_target(load_u32(blob, 0x8003F6E4)) == 0x800696F0, "3F6E4")
    require(load_u32(blob, 0x80069710) == 0x30420080, "andi 0x80")
    require(load_u32(blob, 0x8006984C) == 0x24100008, "s0=8 tail")
    require(
        window_sha(blob, 0x800696F0, 0x800698D4)
        == "8ea0ad39f179aef98cb62db32e52ab5e44a9cbdf9ab92f0bbbe2ad6fc9fa8089",
        "696F0 sha",
    )
    print("PASS: 696F0 121w D1A0&0x80 skip then 6984C tail")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
