#!/usr/bin/env python3
"""PE-BTL77 — 3F3C4 jals 661A4 then 661CC."""
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
    require(jal_target(load_u32(blob, 0x8003F570)) == 0x800661A4, "3F570")
    require(jal_target(load_u32(blob, 0x8003F578)) == 0x800E01BC, "3F578")
    require(jal_target(load_u32(blob, 0x8003F580)) == 0x800661CC, "3F580")
    require((0x800661CC - 0x800661A4) // 4 == 10, "661A4 10w")
    require((0x800661EC - 0x800661CC) // 4 == 8, "661CC 8w")
    require(
        window_sha(blob, 0x800661A4, 0x800661CC)
        == "79eb7a676b370467b30a5e04726833bf40c3cfe844e484c4071945353386de66",
        "661A4 sha",
    )
    print("PASS: 3F570 661A4; 3F578 E01BC; 3F580 661CC")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
