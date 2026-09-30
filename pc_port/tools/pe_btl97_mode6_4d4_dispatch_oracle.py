#!/usr/bin/env python3
"""PE-BTL97 — mode-6 2A7F8→2BC90→2CEE0 and opcode 0xCF 4D4 dispatch."""
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
SHA_19D24 = "4581b7eb41ed889ea40dfd016fe641654330966c556b2084c0404a6122ceba24"
SHA_33A2C = "2a210c6561918ef0c55cdf10d649e3a571d3591306f64ac34e79a08f19f25803"
SHA_693B4 = "815d707308120ff2391a4f27a9d7d9a3e1ddd15c2e594cee5dde7a8068036a35"
SHA_2A9E4 = "ecc6de79bdf9da594a747c27b9cee354b888df1cd50e9c73e8628ed36ac9368c"
SHA_2CEE0 = "bda2d942d0bd2fa5dbc753e66fab7b79cbd34c5eb6c3c5c0e1216511af3b7892"


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(va: int) -> int:
    return va - TADDR + HDR


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, exe_off(va))[0]


def window_sha(blob: bytes, lo: int, hi: int) -> str:
    return hashlib.sha256(blob[exe_off(lo) : exe_off(hi)]).hexdigest()


def jal_target(word: int) -> int:
    return 0x80000000 | ((word & 0x3FFFFFF) << 2)


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require(load_u32(blob, 0x800910A0 + 0xCF * 4) == 0x80019D24, "table[0xCF]")
    require(load_u32(blob, 0x800910A0 + 0x95 * 4) == 0x800192B8, "table[0x95]")
    require(load_u32(blob, 0x800192BC) == 0xAC20D28C, "192BC sw zero D28C")
    require(load_u32(blob, 0x8002CF24) == 0x24020007, "2CF24 li 7")
    require(load_u32(blob, 0x8002CF28) == 0xAF82051C, "2CF28 sw gp+0x51C")
    require(jal_target(load_u32(blob, 0x80019D2C)) == 0x80033A2C, "0xCF jal 33A2C")
    require(jal_target(load_u32(blob, 0x8002A9EC)) == 0x8002BC90, "mode6 jal 2BC90")
    require(jal_target(load_u32(blob, 0x8002CEE0)) == 0x8006914C, "2CEE0 jal 6914C")
    require(load_u32(blob, 0x800693C8) == 0x30420002, "6914C andi D1A0 2")
    require(window_sha(blob, 0x80019D24, 0x80019D44) == SHA_19D24, "19D24 sha")
    require(window_sha(blob, 0x80033A2C, 0x80033A40) == SHA_33A2C, "33A2C sha")
    require(window_sha(blob, 0x800693B4, 0x800693F8) == SHA_693B4, "693B4 sha")
    require(window_sha(blob, 0x8002A9E4, 0x8002A9F8) == SHA_2A9E4, "2A9E4 sha")
    require(window_sha(blob, 0x8002CEE0, 0x8002CF2C) == SHA_2CEE0, "2CEE0 sha")
    print("PASS: 0xCF→33A2C; 2A7F8 mode6→2BC90; 2CEE0 6914C(0) then 2CF24")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
