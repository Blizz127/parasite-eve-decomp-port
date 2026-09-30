#!/usr/bin/env python3
"""PE-BTL87 — 3F3C4 dest-change D1A0/B0CD8 stores."""
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


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require(jal_target(load_u32(blob, 0x8003F6E4)) == 0x800696F0, "3F6E4")
    require(load_u32(blob, 0x8003F6EC) == 0x2404C7FF, "li ~0x3800")
    require(load_u32(blob, 0x8003F6F0) == 0x8F820430, "lw gp+0x430")
    require(load_u32(blob, 0x8003F6FC) == 0x34420040, "ori 0x40")
    require(load_u32(blob, 0x8003F700) == 0x00441024, "and ~0x3800")
    require(load_u32(blob, 0x8003F704) == 0x34630002, "B0CD8|2")
    require(load_u32(blob, 0x8003F70C) == 0x2402F7FF, "li ~0x800")
    require(load_u32(blob, 0x8003F714) == 0x30630200, "andi 0x200")
    require(load_u32(blob, 0x8003F728) == 0x34637DFF, "ori 0x7DFF")
    print("PASS: 3F3C4 dest-change D1A0|=0x40 B0CD8|=2")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
