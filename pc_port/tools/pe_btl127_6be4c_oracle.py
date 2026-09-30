#!/usr/bin/env python3
"""PE-BTL127 — 6BE4C dest-enter CE2/CE3 gate after 1266C."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
WIN = "fb4091da59b98ce270dd0a50c7edf8574fe5bf2117fa6a1e0cb3f5dc9e71306b"
ROOT = pathlib.Path(__file__).resolve().parents[2]
TADDR = 0x80010000
HDR = 0x800


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, va - TADDR + HDR)[0]


def jal_target(word: int) -> int:
    return 0x80000000 | ((word & 0x3FFFFFF) << 2)


def main() -> int:
    exe = next(
        (
            p
            for p in (
                ROOT / "build" / "disc1.candidate.exe",
                ROOT / "build" / "extracted" / "disc1" / "SLUS_006.62",
            )
            if p.is_file()
        ),
        None,
    )
    require(exe is not None, "missing retail EXE")
    blob = exe.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    window = blob[0x8006BE4C - TADDR + HDR : 0x8006BECC - TADDR + HDR]
    require(hashlib.sha256(window).hexdigest() == WIN, "6BE4C window")
    require(jal_target(load_u32(blob, 0x8003F204)) == 0x8006BE4C, "3F074 jal 6BE4C")
    require(jal_target(load_u32(blob, 0x8003F0B8)) == 0x8001266C, "1266C before 6BE4C")
    require(jal_target(load_u32(blob, 0x8003F20C)) == 0x8006BECC, "6BECC after 6BE4C")
    require(load_u32(blob, 0x8006BE50) == 0x90630CE2, "lbu CE2")
    require(load_u32(blob, 0x8006BE70) == 0x90840CE3, "lbu CE3")
    require(load_u32(blob, 0x8006BE7C) == 0x3C030020, "lui 0x200000")
    require(load_u32(blob, 0x8006BEB8) == 0x34420004, "ori CE6 4")
    require(load_u32(blob, 0x8006BEC0) == 0xA0220CE6, "sb CE6")
    require(load_u32(blob, 0x8006BEC4) == 0x03E00008, "jr ra")
    print("PASS: 6BE4C is post-1266C CE2/CE3 overlay gate, not B6A80")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
