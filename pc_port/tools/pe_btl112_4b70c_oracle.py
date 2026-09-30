#!/usr/bin/env python3
"""PE-BTL112 — 4B70C installs 4BB80; 4BB80 reaches 512AC(10)."""
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
    require(jal_target(load_u32(blob, 0x8002B194)) == 0x8004B70C, "2B0E8 jal 4B70C")
    require(jal_target(load_u32(blob, 0x8004B8D0)) == 0x8004B90C, "4B70C jal 4B90C")
    require(load_u32(blob, 0x8004B934) == 0x3C028005, "lui 8005")
    require(load_u32(blob, 0x8004B938) == 0x2442B970, "addiu → 4B970")
    require((0x80050000 + (0xB970 - 0x10000)) == 0x8004B970, "4B970 synth")
    require(load_u32(blob, 0x8004B940) == 0x3C028005, "lui 8005 cb")
    require(load_u32(blob, 0x8004B944) == 0x2442BB80, "addiu → 4BB80")
    require((0x80050000 + (0xBB80 - 0x10000)) == 0x8004BB80, "4BB80 synth")
    require(load_u32(blob, 0x8004B94C) == 0xAE02002C, "sw cb +0x2C")
    require(jal_target(load_u32(blob, 0x8004B948)) == 0x80062CB8, "62CB8")
    require(load_u32(blob, 0x80062CB8) == 0xAF8403EC, "62CB8 sw gp+0x3EC")
    require(load_u32(blob, 0x80062CC4) == 0x8F8203EC, "62CC4 lw gp+0x3EC")
    require(load_u32(blob, 0x8004BB84) == 0x3C020001, "4BB80 lui 0x10000")
    require(load_u32(blob, 0x8004BB88) == 0x00A22824, "4BB80 and a1")
    require(load_u32(blob, 0x8004BC38) == 0x2404000A, "addiu a0,10")
    require(jal_target(load_u32(blob, 0x8004BC4C)) == 0x800512AC, "4BB80 jal 512AC")
    require(load_u32(blob, 0x800514A0) == 0x08014533, "case 10 jump")
    require(load_u32(blob, 0x800514A4) == 0x240203E8, "li 1000")
    require(jal_target(load_u32(blob, 0x8005C4C4)) == 0x8005E6F0, "5C498 chain")
    require(jal_target(load_u32(blob, 0x8005C4D4)) == 0x8005E30C, "5C498 jal 5E30C")
    require(load_u32(blob, 0x8005E4AC) == 0x3C050001, "type4 a1=0x10000")
    require(load_u32(blob, 0x8002B1AC) == 0x87830534, "phase1 lh 534")
    require(load_u32(blob, 0x8002B1B0) == 0x240203E8, "phase1 li 1000")
    print("PASS: 4B70C → 4B90C 4BB80; 4BB80 → 512AC(10); 5E30C type4 a1=0x10000")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
