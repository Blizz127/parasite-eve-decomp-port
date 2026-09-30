#!/usr/bin/env python3
"""PE-BTL107 — 27D14 DoT at 27F24; 5C498 → 534; 512AC jtbl[10]=1000."""
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
    require(load_u32(blob, 0x80027EF8) == 0x30820010, "body bit 0x10")
    require(load_u32(blob, 0x80027F14) == 0x86230096, "lh body+0x96")
    require(load_u32(blob, 0x80027F24) == 0xAE220010, "sw body+0x10")
    require(jal_target(load_u32(blob, 0x80029A5C)) == 0x8005C498, "5C498")
    require(load_u32(blob, 0x80029A68) == 0xA7820534, "sh 534")
    require(load_u32(blob, 0x8005C498) == 0x27BDFFE8, "5C498 addiu")
    require(jal_target(load_u32(blob, 0x8005C4A4)) == 0x80042ED0, "42ED0")
    require(jal_target(load_u32(blob, 0x8005C4BC)) == 0x80051504, "51504")
    require(jal_target(load_u32(blob, 0x8005C548)) == 0x800514F8, "514F8")
    require(load_u32(blob, 0x80011198) == 0x800514A0, "jtbl[10]")
    require(load_u32(blob, 0x800514A4) == 0x240203E8, "li 1000")
    print("PASS: DoT body+0x10; 5C498→534; 512AC(10)=1000")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
