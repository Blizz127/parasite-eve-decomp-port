#!/usr/bin/env python3
"""PE-BTL113 — 2B0E8 +0x16 is 1A4AC clip high-half, not a plant."""
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
    require(load_u32(blob, 0x8002B114) == 0x2402000A, "phase0 li 10")
    require(load_u32(blob, 0x8002B148) == 0x94830016, "lhu Aya+0x16")
    require(load_u32(blob, 0x8001A6D4) == 0xAE200014, "1A680 sw 0 +0x14")
    require(load_u32(blob, 0x8001A578) == 0x8E02001C, "1A4AC lw +0x1C")
    require(load_u32(blob, 0x8001A580) == 0x00E22821, "1A4AC addu cur+speed")
    require(load_u32(blob, 0x8001A574) == 0xAE020014, "1A4AC sw +0x14")
    require(load_u32(blob, 0x800351CC) == 0x3C020001, "35038 lui 0x10000")
    require(load_u32(blob, 0x800351D0) == 0xAE22001C, "35038 sw +0x1C")
    require(jal_target(load_u32(blob, 0x80035B84)) == 0x8001A4AC, "35558 1A4AC")
    print("PASS: +0x16 is 1A4AC(+0x1C) high half; 35038 plants 0x10000")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
