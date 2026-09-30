#!/usr/bin/env python3
"""PE-BTL122 — non-Aya *actor body is 2F7D8 via 0x6F, not 35038."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
ROOT = pathlib.Path(__file__).resolve().parents[2]
CANDIDATES = (
    ROOT / "build" / "disc1.candidate.exe",
    ROOT / "build" / "extracted" / "disc1" / "SLUS_006.62",
)
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
    exe = next((p for p in CANDIDATES if p.is_file()), None)
    require(exe is not None, "missing retail EXE")
    blob = exe.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")

    require(jal_target(load_u32(blob, 0x80018964)) == 0x8002F7D8, "0x6F")
    require(load_u32(blob, 0x800910A0 + 0x6F * 4) == 0x80018954, "jtbl 6F")
    require(load_u32(blob, 0x8002F8E4) == 0xAD220000, "2F7D8 sw body")
    require(jal_target(load_u32(blob, 0x8003515C)) == 0x8002F76C, "type0 2F76C")
    require(load_u32(blob, 0x80035170) == 0xAE200000, "type!=0 sw 0")
    require(load_u32(blob, 0x80035294) == 0x106000A3, "1AC==0 skip tail")
    require(jal_target(load_u32(blob, 0x800352B0)) == 0x8001A680, "Aya 1A680")
    require(jal_target(load_u32(blob, 0x80035340)) == 0x800362B8, "362B8")
    require(jal_target(load_u32(blob, 0x800354B4)) == 0x8003D050, "3D050")
    print("PASS: 2F7D8 is the non-Aya body store; 35038 zeros *actor")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
