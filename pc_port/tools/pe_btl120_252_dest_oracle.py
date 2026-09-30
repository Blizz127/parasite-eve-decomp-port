#!/usr/bin/env python3
"""PE-BTL120 — Aya+0x252 is dest+0x9E; 3C818 sb $0 is the clearer."""
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

    require(0x1B4 + 0x9E == 0x252, "Aya+0x1B4+0x9E == +0x252")
    require(0x1B4 + 0x9C == 0x250, "Aya+0x1B4+0x9C == +0x250")

    require(load_u32(blob, 0x80035A80) == 0x263001B4, "s0=actor+0x1B4")
    require(jal_target(load_u32(blob, 0x80035AE0)) == 0x8003AF14, "35558 3AF14")
    require(load_u32(blob, 0x800359A0) == 0x8F91049C, "dest walk D20C")

    require((0x8003B144 - 0x8003AF14) // 4 == 140, "3AF14 140w")
    require(jal_target(load_u32(blob, 0x8003B038)) == 0x8003C818, "bit1 3C818")
    require(load_u32(blob, 0x8003B02C) == 0x30620002, "andi +0x9C 2")

    require((0x8003CAE0 - 0x8003C818) // 4 == 178, "3C818 178w")
    require(load_u32(blob, 0x8003C87C) == 0xA200009E, "sb $0 dest+0x9E")
    require(load_u32(blob, 0x8003C848) == 0x8203008C, "lb +0x8C")
    require(load_u32(blob, 0x8003CAD4) == 0xA203008C, "sb +0x8C--")

    require(load_u32(blob, 0x80024BD8) == 0xA0620252, "case2 sb +0x252")
    require(jal_target(load_u32(blob, 0x80024BF0)) == 0x8003C5D8, "case2 3C5D8")

    print("PASS: dest+0x9E is Aya+0x252; 3C818 sb $0 is the clearer")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
