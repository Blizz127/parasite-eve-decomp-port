#!/usr/bin/env python3
"""PE-BTL85 — 3DFC8 is a 2-word nop; 3F6DC dest-change jal."""
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
    require((0x8003DFD0 - 0x8003DFC8) // 4 == 2, "3DFC8 2w")
    require(load_u32(blob, 0x8003DFC8) == 0x03E00008, "jr ra")
    require(load_u32(blob, 0x8003DFCC) == 0x00000000, "nop")
    require(jal_target(load_u32(blob, 0x8003F6DC)) == 0x8003DFC8, "3F6DC")
    require(jal_target(load_u32(blob, 0x8003F6CC)) == 0x80074DC0, "3F6CC")
    require(jal_target(load_u32(blob, 0x8003F6D4)) == 0x80087024, "3F6D4")
    require(
        window_sha(blob, 0x8003DFC8, 0x8003DFD0)
        == "6d64edf91449c1b17746c1ef18afa2eb25c70bdf1322ab3df5a2630993b7e2f1",
        "3DFC8 sha",
    )
    print("PASS: 3DFC8 2w nop; 3F6DC dest-change jal")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
