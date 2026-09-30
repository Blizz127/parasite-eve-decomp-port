#!/usr/bin/env python3
"""PE-BTL104 — 292EC remaining-enemy tail jals 2F300; 2F300 stores mode 2."""
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
    require(load_u32(blob, 0x800292EC) == 0x3C10800A, "lw D20C lui")
    require(load_u32(blob, 0x800292F0) == 0x8E10D20C, "lw D20C")
    require(load_u32(blob, 0x800292FC) == 0x24030001, "li remain 1")
    require(load_u32(blob, 0x80029324) == 0x00001821, "found body remain=0")
    require(load_u32(blob, 0x80029350) == 0x8442000C, "lh record+0xC")
    require(load_u32(blob, 0x80029358) == 0x18400003, "blez skip 2F300")
    require(jal_target(load_u32(blob, 0x80029360)) == 0x8002F300, "jal 2F300")
    require(jal_target(load_u32(blob, 0x8002F308)) == 0x800293F4, "2F300 293F4")
    require(load_u32(blob, 0x8002F56C) == 0x24020002, "li 2")
    require(load_u32(blob, 0x8002F570) == 0xAF82051C, "sw mode 2")
    print("PASS: 292EC remain-walk → 2F300 mode 2; Aya HP<=0 skips")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
