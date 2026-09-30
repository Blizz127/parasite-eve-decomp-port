#!/usr/bin/env python3
"""PE-BTL84 — 6A0E8 is a D1A0&0x10 early-out."""
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
    require((0x8006A258 - 0x8006A0E8) // 4 == 92, "6A0E8 92w")
    require(jal_target(load_u32(blob, 0x8003F640)) == 0x8006A0E8, "3F640")
    require(load_u32(blob, 0x8006A108) == 0x30420200, "andi 0x200")
    require(load_u32(blob, 0x8006A120) == 0x30420010, "andi 0x10")
    require(jal_target(load_u32(blob, 0x8003F5EC)) == 0x80073A44, "3F5EC VSync")
    require(
        window_sha(blob, 0x8006A0E8, 0x8006A258)
        == "aa84251c2bc4cb1321173132edf028b70ff6cbb4a19028a7fba179b0d117bb86",
        "6A0E8 sha",
    )
    print("PASS: 6A0E8 92w D1A0&0x10 out; sole jal 3F640")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
