#!/usr/bin/env python3
"""PE-BTL125 — type-6 +0x190 waits while scratch[0]&4 is clear."""
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

    require(load_u32(blob, 0x80012A0C) == 0x8E020008, "jtbl[11] ==")
    require(load_u32(blob, 0x800129A0) == 0x8E020008, "jtbl[7] NOT head")
    require(load_u32(blob, 0x8001731C) == 0x8C820000, "0x05 lw arg0")
    require(load_u32(blob, 0x8001732C) == 0x14400009, "0x05 bne skip-if-false")
    require(load_u32(blob, 0x80017A50) == 0x24030001, "0x2A li 1")
    require(load_u32(blob, 0x80017A68) == 0x00431025, "0x2A or dest")
    require(load_u32(blob, 0x800126D0) == 0x24636A80, "1266C addiu B6A80")
    require(load_u32(blob, 0x80034F3C) == 0x24636A80, "34F10 addiu B6A80")
    require(load_u32(blob, 0x800171F4) == 0x24426A80, "17018 kind4 B6A80")
    require(jal_target(load_u32(blob, 0x8003F40C)) == 0x8003EB04, "3F3C4 jal 3EB04")
    print("PASS: type-6 wait is scratch[0]&4; EXE B6A80 sites are 1266C/17018/34F10")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
