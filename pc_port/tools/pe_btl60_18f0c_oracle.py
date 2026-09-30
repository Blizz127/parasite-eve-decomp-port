#!/usr/bin/env python3
"""PE-BTL60 — opcode 0x87/66BD8 and 0x31 table contract."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
EXE = ROOT / "build" / "disc1.candidate.exe"
TADDR = 0x80010000
HDR = 0x800


def exe_off(va: int) -> int:
    return va - TADDR + HDR


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, exe_off(va))[0]


def main() -> int:
    blob = EXE.read_bytes()
    if load_u32(blob, 0x800910A0 + 0x87 * 4) != 0x80018F0C:
        print("FAIL: table[0x87]", file=sys.stderr)
        return 1
    if load_u32(blob, 0x800910A0 + 0x31 * 4) != 0x80017BB4:
        print("FAIL: table[0x31]", file=sys.stderr)
        return 1
    sha87 = hashlib.sha256(blob[exe_off(0x80018F0C) : exe_off(0x80018F54)]).hexdigest()
    if sha87 != "cb2c045fd48813b66f79f29a11c45d26c3394c02ac40601fb63740f02baed649":
        print(f"FAIL: 18F0C sha {sha87}", file=sys.stderr)
        return 1
    sha = hashlib.sha256(blob[exe_off(0x80066BD8) : exe_off(0x80066C7C)]).hexdigest()
    if sha != "ebcd3c1a1457163e25b0a72b07729e8b8bd418bfe342f8e7e3e97174c25930cc":
        print(f"FAIL: 66BD8 sha {sha}", file=sys.stderr)
        return 1
    if load_u32(blob, 0x80018F3C) != 0x0C019AF6:
        print("FAIL: 18F3C jal 66BD8", file=sys.stderr)
        return 1
    print("PASS: 0x87/18F0C 66BD8 + 0x31/17BB4 table")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
