#!/usr/bin/env python3
"""PE-BTL103 — 2B0E8 is mode 2; phase 3 stores mode 9, not -1."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
SHA_2B0E8 = "9eabc01299179e888714b69f69487bac38ecd50f1f1e40f6ddcb8e632bfefd67"
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


def window_sha(blob: bytes, lo: int, hi: int) -> str:
    return hashlib.sha256(blob[exe_off(lo) : exe_off(hi)]).hexdigest()


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require(jal_target(load_u32(blob, 0x8002A84C)) == 0x8002B0E8, "mode2 jal")
    require(window_sha(blob, 0x8002B0E8, 0x8002B29C) == SHA_2B0E8, "2B0E8 sha")
    require(jal_target(load_u32(blob, 0x8002B24C)) == 0x8006D60C, "phase3 6D60C")
    require(jal_target(load_u32(blob, 0x8002B25C)) == 0x800295E4, "phase3 295E4")
    require(load_u32(blob, 0x8002B278) == 0x24030009, "li 9")
    require(load_u32(blob, 0x8002B27C) == 0xAF83051C, "sw mode 9")
    require(load_u32(blob, 0x8002B264) == 0x3C05FFFF, "lui ~0x8000 hi")
    require(load_u32(blob, 0x8002B268) == 0x34A57FFF, "ori ~0x8000 lo")
    require(jal_target(load_u32(blob, 0x8002B22C)) == 0x8001A680, "phase2 1A680")
    require(load_u32(blob, 0x8002F56C) == 0x24020002, "2F300 li 2")
    require(load_u32(blob, 0x8002F570) == 0xAF82051C, "2F300 sw mode 2")
    require(jal_target(load_u32(blob, 0x80029360)) == 0x8002F300, "29360 jal 2F300")
    print("PASS: 2B0E8 mode2 → mode 9; 2F300 is the mode-2 producer")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
