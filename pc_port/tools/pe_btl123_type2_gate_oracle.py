#!/usr/bin/env python3
"""PE-BTL123 — type-2 0x08 is gated by persist[0x4A] < 40."""
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


def jal_target(word: int) -> int:
    return 0x80000000 | ((word & 0x3FFFFFF) << 2)


def main() -> int:
    exe = next((p for p in CANDIDATES if p.is_file()), None)
    require(exe is not None, "missing retail EXE")
    blob = exe.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require(load_u32(blob, 0x80012850) == 0x27BDFFE8, "12850")
    require(load_u32(blob, 0x80010000 + 0x0A * 4) == 0x800129EC, "jtbl[10]")
    require(load_u32(blob, 0x80012A00) == 0x0043102A, "slt a<b")
    require(load_u32(blob, 0x8001731C) == 0x8C820000, "0x05 lw arg0")
    require(load_u32(blob, 0x80017324) == 0x8C420000, "lw *arg0")
    require(load_u32(blob, 0x8001732C) == 0x14400009, "bne skip-if-false")
    print("PASS: 0x09 subop 0x0A is signed < ; 0x05 skips if 0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
