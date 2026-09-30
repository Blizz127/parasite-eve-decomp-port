#!/usr/bin/env python3
"""PE-BTL69 — 66CE8 / 68CE0 / rsin-rcos ROM contract."""
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
    require((0x80066F60 - 0x80066CE8) // 4 == 158, "66CE8 158w")
    require(
        window_sha(blob, 0x80066CE8, 0x80066F60)
        == "a015062e5fed0f88f604176665fce1713abb97b45ce1ec2ad1efddb31db6b0a9",
        "66CE8 sha",
    )
    require((0x80077D30 - 0x80077CF4) // 4 == 15, "77CF4 15w")
    require((0x80077DC4 - 0x80077D30) // 4 == 37, "77D30 37w")
    require((0x80077E64 - 0x80077DC4) // 4 == 40, "77DC4 40w")
    require(jal_target(load_u32(blob, 0x80068CE8)) == 0x80066CE8, "68CE0 jal")
    require(jal_target(load_u32(blob, 0x8003F560)) == 0x80068CE0, "3F3C4 jal")
    require(load_u32(blob, 0x80066D1C) == 0x9463D020, "lhu BD020")
    require(jal_target(load_u32(blob, 0x80066D6C)) == 0x80077DC4, "rcos")
    require(jal_target(load_u32(blob, 0x80066D78)) == 0x80077CF4, "rsin")
    print("PASS: 66CE8 158w; 68CE0; 3F560; rsin/rcos")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
