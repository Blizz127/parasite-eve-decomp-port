#!/usr/bin/env python3
"""PE-BTL101 — 2B29C[5] stores mode -1 and jals 6A25C dest restore."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
SHA_295E4 = "f8edd9429267dbf82ff1d03c3ef90dab43b72689829719528c2c06e60b0280e5"
SHA_6A25C = "09f71b1be185eafb93bbb498608abbf6ca53bc1cb9892993398145db5fa9ac86"
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


def window_sha(blob: bytes, lo: int, hi: int) -> str:
    return hashlib.sha256(blob[exe_off(lo) : exe_off(hi)]).hexdigest()


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require(load_u32(blob, 0x80010924) == 0x8002B8E8, "2B29C jtbl[5]")
    require(jal_target(load_u32(blob, 0x8002B8E8)) == 0x800295E4, "case5 jal 295E4")
    require(load_u32(blob, 0x8002B8F0) == 0x2402FFFF, "li -1")
    require(load_u32(blob, 0x8002B8F4) == 0xAF82051C, "sw mode")
    require(jal_target(load_u32(blob, 0x8002B8F8)) == 0x8006A25C, "jal 6A25C")
    require(window_sha(blob, 0x800295E4, 0x80029810) == SHA_295E4, "295E4 sha")
    require(window_sha(blob, 0x8006A25C, 0x8006A2E8) == SHA_6A25C, "6A25C sha")
    require(load_u32(blob, 0x8006A2C0) == 0x34A50048, "ori dest lo 0x48")
    require(load_u32(blob, 0x8006A29C) == 0x3C05A940, "lui dest hi 0xA940")
    require(load_u32(blob, 0x8006A2C8) == 0xAC25D280, "sw D280")
    require(jal_target(load_u32(blob, 0x80029794)) == 0x80021D4C, "295E4 jal 21D4C")
    print("PASS: 2B29C[5] 295E4 → mode=-1 → 6A25C dest 0xA9400048")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
