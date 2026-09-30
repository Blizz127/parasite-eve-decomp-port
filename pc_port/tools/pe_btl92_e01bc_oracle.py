#!/usr/bin/env python3
"""PE-BTL92 — 3F3C4 jals E01BC; live E21A4<=0 early-out."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
SHA256 = "87f953d4e09191ed132c0b36c5d9e6e35d8319945de829bf5aa778af8d657cd8"
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
    require(jal_target(load_u32(blob, 0x8003F578)) == 0x800E01BC, "3F578")
    require((0x800E026C - 0x800E01BC) // 4 == 44, "44w")
    require(load_u32(blob, 0x800E01BC) == 0x3C02800E, "lui 800E")
    require(load_u32(blob, 0x800E01C0) == 0x844221A4, "lh E21A4")
    require(load_u32(blob, 0x800E01D0) == 0x8E102800, "lw E2800")
    require(load_u32(blob, 0x800E01E0) == 0x1840001B, "blez")
    require(load_u32(blob, 0x800E0264) == 0x03E00008, "jr")
    window = blob[exe_off(0x800E01BC) : exe_off(0x800E026C)]
    require(hashlib.sha256(window).hexdigest() == SHA256, "sha")
    print("PASS: 3F578 E01BC 44w; live blez on E21A4<=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
