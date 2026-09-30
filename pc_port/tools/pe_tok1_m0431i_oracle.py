#!/usr/bin/env python3
"""Phase 6E-TOK1 — M0431I/M0353I token identities and package slots."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
ROOT = pathlib.Path(__file__).resolve().parents[2]
CANDIDATE = ROOT / "build" / "disc1.candidate.exe"
EXTRACTED = ROOT / "build" / "extracted" / "disc1" / "SLUS_006.62"
TADDR = 0x80010000
HDR = 0x800
TABLE = 0x80093378
CHARSET = 0x800930B4
ALPHA = b"0123456789abcdefghiklmnoprstuvwy"


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(va: int) -> int:
    return va - TADDR + HDR


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, exe_off(va))[0]


def main() -> int:
    options = ([pathlib.Path(sys.argv[1])] if len(sys.argv) > 1
               else [CANDIDATE, EXTRACTED])
    blob = None
    used = None
    for exe in options:
        if exe.is_file() and hashlib.sha1(exe.read_bytes()).hexdigest() == SHA1:
            blob = exe.read_bytes()
            used = exe
            break
    require(blob is not None, f"no SHA-1-exact EXE among {options}")
    print(f"authority: {used}")
    require(blob[exe_off(CHARSET) : exe_off(CHARSET) + 32] == ALPHA, "charset")
    # Slot 430 (M0431I): rel + packed; slot 352 (M0353I); slot 366 (M0367I).
    require(load_u32(blob, TABLE + 430 * 8) == 0x182CF, "m0431i rel")
    require(load_u32(blob, TABLE + 430 * 8 + 4) == 0x01200911, "m0431i pack")
    require(load_u32(blob, TABLE + 352 * 8) == 0x1483F, "m0353i rel")
    require(load_u32(blob, TABLE + 352 * 8 + 4) == 0x0E08921, "m0353i pack")
    require(load_u32(blob, TABLE + 366 * 8) == 0x15050, "m0367i rel")
    require(load_u32(blob, TABLE + 366 * 8 + 4) == 0x04E0AA21, "m0367i pack")
    print("PASS: token slots m0431i/m0353i/m0367i + charset")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
