#!/usr/bin/env python3
"""PE-BTL74 — 68CE0 sequencer + 65674 D1A0 mask audit."""
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
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def window_sha(blob: bytes, start: int, end: int) -> str:
    return hashlib.sha256(blob[exe_off(start) : exe_off(end)]).hexdigest()


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require((0x80068D28 - 0x80068CE0) // 4 == 18, "68CE0 18w")
    require(jal_target(load_u32(blob, 0x8003F560)) == 0x80068CE0, "3F560")
    require(jal_target(load_u32(blob, 0x80068CE8)) == 0x80066CE8, "66CE8")
    require(jal_target(load_u32(blob, 0x80068CF0)) == 0x80065674, "65674")
    require(jal_target(load_u32(blob, 0x80068CF8)) == 0x80067E1C, "67E1C")
    require(load_u32(blob, 0x80065674) == 0x3C02800A, "65674 lui")
    require(load_u32(blob, 0x80065678) == 0x8C42D1A0, "lw D1A0")
    require(load_u32(blob, 0x80065680) == 0x30420104, "andi D1A0, 0x104")
    require(load_u32(blob, 0x80065684) == 0x1440009D, "bne masked D1A0")
    require((0x8006590C - 0x80065674) // 4 == 166, "65674 166w")
    require(
        window_sha(blob, 0x80065674, 0x8006590C)
        == "9b325f335fa931be66fdb74c976a5679c43b6890248ba05c21cac965fc3f3c81",
        "65674 sha",
    )
    require(
        window_sha(blob, 0x80068CE0, 0x80068D28)
        == "55c7dbcc38c18c7b8b301364bb8b12c52bbc9767435489f34f2f3683bacb8e5b",
        "68CE0 sha",
    )
    print("PASS: 68CE0 18w; 65674 D1A0 & 0x104 gate; 67E1C next")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
