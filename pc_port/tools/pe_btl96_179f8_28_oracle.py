#!/usr/bin/env python3
"""PE-BTL96 — opcode 0x28 bit-clear 179F8."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
SHA256 = "cb5775ebacad81ba452adcb1c22bccf6e0d7efcb3f7a41f25b0f73be55071aeb"
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
    require(load_u32(blob, 0x800910A0 + 0x28 * 4) == 0x800179F8, "table[0x28]")
    require((0x80017A24 - 0x800179F8) // 4 == 11, "11w")
    require(load_u32(blob, 0x800179FC) == 0x24030001, "li 1")
    require(load_u32(blob, 0x80017A08) == 0x00431804, "sllv")
    require(load_u32(blob, 0x80017A10) == 0x00031827, "nor")
    require(load_u32(blob, 0x80017A14) == 0x00431024, "and")
    require(load_u32(blob, 0x80017A1C) == 0x03E00008, "jr")
    require(load_u32(blob, 0x80017A20) == 0x24020001, "v0=1")
    window = blob[exe_off(0x800179F8) : exe_off(0x80017A24)]
    require(hashlib.sha256(window).hexdigest() == SHA256, "sha")
    print("PASS: 0x28 179F8 11w *arg0 &= ~(1<<*arg1)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
