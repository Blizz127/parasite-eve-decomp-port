#!/usr/bin/env python3
"""PE-BG1 — 66F60 / 677FC / 68B94 tile-publish bodies."""
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
    require((0x800671C8 - 0x80066F60) // 4 == 154, "66F60 154w")
    require((0x800679C4 - 0x800677FC) // 4 == 114, "677FC 114w")
    require((0x80068CE0 - 0x80068B94) // 4 == 83, "68B94 83w")
    require(jal_target(load_u32(blob, 0x80067940)) == 0x80066F60, "67940")
    require(jal_target(load_u32(blob, 0x80068BBC)) == 0x800677FC, "68BBC")
    require(jal_target(load_u32(blob, 0x8003F1D8)) == 0x80068B94, "3F1D8")
    require(load_u32(blob, 0x80067028) == 0x2402007C, "SPRT_16 addiu 0x7C")
    require(load_u32(blob, 0x80067130) == 0x3C03E100, "DR_MODE E1 hi")
    require(
        window_sha(blob, 0x80066F60, 0x800671C8)
        == "8fb26adc5f5a9cefd4b5572447208f26e444bedc8b1c3ae44554a98a6e215442",
        "66F60 sha",
    )
    require(
        window_sha(blob, 0x800677FC, 0x800679C4)
        == "a3723245a927f92f7c42839b67e53783e9f18d0ddae20c6c432f031dc1ebbf7e",
        "677FC sha",
    )
    require(
        window_sha(blob, 0x80068B94, 0x80068CE0)
        == "e6e9bdb570efb92bfcaf43dfa7a1c06c4bcaf5afa643fd185a80308540ba073f",
        "68B94 sha",
    )
    print("PASS: 66F60 154w / 677FC 114w / 68B94 83w; 3F074 jals 68B94")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
