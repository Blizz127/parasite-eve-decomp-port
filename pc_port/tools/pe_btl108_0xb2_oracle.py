#!/usr/bin/env python3
"""PE-BTL108 — opcode 0xB2 / 19798 jals 392EC; table slot 0xB2."""
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
    require(load_u32(blob, 0x800910A0 + 0xB2 * 4) == 0x80019798, "table")
    require((0x800197CC - 0x80019798) // 4 == 13, "19798 13w")
    require(jal_target(load_u32(blob, 0x800197A4)) == 0x800392EC, "jal")
    require(load_u32(blob, 0x800197C8) == 0x03E00008, "19798 jr")
    require((0x8003930C - 0x800392EC) // 4 == 8, "392EC 8w")
    require(load_u32(blob, 0x800392EC) == 0x3C028009, "lui 8009")
    require(load_u32(blob, 0x800392F0) == 0x90421A1C, "lbu 1A1C")
    require(load_u32(blob, 0x80039304) == 0x90421A1D, "lbu 1A1D")
    print("PASS: 0xB2 → 19798 → 392EC 1A1C/1A1D")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
