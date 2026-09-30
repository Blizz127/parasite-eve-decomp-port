#!/usr/bin/env python3
"""PE-BTL124 — dest-ready m0005i type-1 ticks type-2; 35558 360B4 suffix."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
ROOT = pathlib.Path(__file__).resolve().parents[2]
CANDIDATES = (
    ROOT / "build" / "disc1.candidate.exe",
    ROOT / "build" / "extracted" / "disc1" / "SLUS_006.62",
)
TADDR = 0x80010000
HDR = 0x800


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, va - TADDR + HDR)[0]


def load_u16(blob: bytes, va: int) -> int:
    return struct.unpack_from("<H", blob, va - TADDR + HDR)[0]


def jal_target(word: int) -> int:
    return 0x80000000 | ((word & 0x3FFFFFF) << 2)


def main() -> int:
    exe = next((p for p in CANDIDATES if p.is_file()), None)
    require(exe is not None, "missing retail EXE")
    blob = exe.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")

    require(load_u32(blob, 0x80035BEC) == 0x0C00692B, "35558 jal 1A4AC")
    require(jal_target(load_u32(blob, 0x80035C2C)) == 0x800360B4, "35C2C jal 360B4")
    require(load_u32(blob, 0x800360CC) == 0x3C02FF7F, "360B4 lui 0xFF7F")
    require(load_u32(blob, 0x800360D0) == 0x3442FFFF, "360B4 ori 0xFFFF")
    require(load_u32(blob, 0x800360D4) == 0x8E040098, "360B4 lw +0x98")
    require(load_u32(blob, 0x800360E4) == 0xAE020098, "360B4 sw +0x98")
    require(load_u32(blob, 0x80035C40) == 0x3C03EFFF, "35C34 lui 0xEFFF")
    require(load_u32(blob, 0x80035C44) == 0x3463FFFF, "35C34 ori 0xFFFF")
    require(load_u32(blob, 0x80035C48) == 0x8E220098, "35C34 lw +0x98")
    require(load_u32(blob, 0x80035C54) == 0xAE220098, "35C34 sw +0x98")

    require(load_u32(blob, 0x8001A6EC) == 0xAE220098, "1A680 sw +0x98")
    require(load_u32(blob, 0x8001A6F0) == 0x90820002, "1A680 lbu 2(a0) no null")
    require(load_u16(blob, 0x800930D8 + 18 * 2) == 288, "CE2=10 start")
    require(load_u16(blob, 0x800930D8 + 19 * 2) == 316, "CE2=10 end")
    require(load_u16(blob, 0x800930D8 + 22 * 2) == 396, "CE2=14 start")
    require(load_u16(blob, 0x800930D8 + 23 * 2) == 428, "CE2=14 end")
    require(load_u32(blob, 0x800910C0) == 0x8001735C, "0x08 -> 1735C")
    require(jal_target(load_u32(blob, 0x80017394)) == 0x80035038, "0x08 jal 35038")

    print("PASS: 35558 360B4 clears 0x800000; 1A680 lbu 2(a0); D_800930D8 CE2=10/14")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
