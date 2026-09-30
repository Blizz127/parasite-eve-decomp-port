#!/usr/bin/env python3
"""PE-BTL105 — 27D14 death gate jals 28E94; 2A53C walks into 27D14."""
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


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require((0x80028570 - 0x80027D14) // 4 == 535, "27D14 window")
    require(load_u32(blob, 0x8002856C) == 0x03E00008, "27D14 jr")
    require(jal_target(load_u32(blob, 0x8002A53C)) == 0x80027D14, "2A53C")
    require(jal_target(load_u32(blob, 0x80028464)) == 0x80028E94, "28464")
    require(jal_target(load_u32(blob, 0x800284DC)) == 0x80028E94, "284DC")
    require(load_u32(blob, 0x800283C4) == 0x1C400062, "bgtz body+0x10")
    require(jal_target(load_u32(blob, 0x80029178)) == 0x8002F970, "phase3 2F970")
    require(jal_target(load_u32(blob, 0x80029360)) == 0x8002F300, "tail 2F300")
    print("PASS: 27D14 → 28E94 → 2F970 → 292EC; 2A53C walk")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
