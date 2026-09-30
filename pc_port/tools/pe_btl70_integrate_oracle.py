#!/usr/bin/env python3
"""PE-BTL70 — 35C84/35E04 bit1 only gates +0x88, not pose."""
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


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require(load_u32(blob, 0x80035D24) == 0x30420002, "35C84 andi 2")
    require(load_u32(blob, 0x80035D28) == 0x1040000D, "beq +13 -> 35D60")
    require(load_u32(blob, 0x80035D60) == 0x8E020068, "skip still lw +0x68")
    require(load_u32(blob, 0x80035E74) == 0x30420002, "35E04 andi 2")
    require(load_u32(blob, 0x80035E78) == 0x1040000D, "beq +13 -> 35EB0")
    require(load_u32(blob, 0x80035EB0) == 0x8E020068, "35E04 skip still lw +0x68")
    print("PASS: bit1 skips +0x88 only; pose += +0x68 always")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
