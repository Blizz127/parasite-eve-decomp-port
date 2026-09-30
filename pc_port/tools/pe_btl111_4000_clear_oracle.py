#!/usr/bin/env python3
"""PE-BTL111 — 27D14 0x4000 path clears body 0x6000 at 28088."""
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


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, va - TADDR + HDR)[0]


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require(load_u32(blob, 0x80027FB0) >> 26 == 5, "bne 0x4000")
    require(load_u32(blob, 0x80028078) == 0x24039FFF, "mask ~0x6000")
    require(load_u32(blob, 0x8002808C) == 0xAE420000, "sw body")
    print("PASS: 27D14 0x4000 clears 0x6000")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
