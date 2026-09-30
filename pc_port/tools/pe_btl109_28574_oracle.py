#!/usr/bin/env python3
"""PE-BTL109 — 236E8 arms body 0x2000; 28574 subtracts body+0x10."""
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
    require(jal_target(load_u32(blob, 0x8002A4D4)) == 0x800236E8, "299CC 236E8")
    require(jal_target(load_u32(blob, 0x80027F84)) == 0x80028574, "27D14 28574")
    require(jal_target(load_u32(blob, 0x800235AC)) == 0x80023008, "2312C 23008")
    require(load_u32(blob, 0x80028BE4) == 0x00501023, "subu HP")
    require(load_u32(blob, 0x80028BE8) == 0xAE820010, "sw body+0x10")
    require(load_u32(blob, 0x80021054) == 0x3C02800A, "21054 lui")
    require(load_u32(blob, 0x80021078) == 0x03E00008, "21054 jr")
    require(blob[exe_off(0x800108E4) : exe_off(0x800108E4) + 11] ==
            bytes([0, 100, 60, 41, 0, 25, 0, 18, 0, 0, 13]),
            "scale table")
    print("PASS: 236E8 / 28574 / 23008 / 21054 / 0x800108E4")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
