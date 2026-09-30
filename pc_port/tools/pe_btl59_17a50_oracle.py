#!/usr/bin/env python3
"""PE-BTL59 — opcode 0x2A ROM contract + 3F3C4 3F074 first jal."""
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
    if load_u32(blob, 0x800910A0 + 0x2A * 4) != 0x80017A50:
        print("FAIL: table[0x2A]", file=sys.stderr)
        return 1
    sha = hashlib.sha256(blob[exe_off(0x80017A50) : exe_off(0x80017A78)]).hexdigest()
    if sha != "2f58ffe89e4130506d67a6784ea268503a8d0a6dec26bbe01506e2cec926db68":
        print(f"FAIL: 17A50 sha {sha}", file=sys.stderr)
        return 1
    if load_u32(blob, 0x80017A74) != 0x24020001:
        print("FAIL: 17A74 v0=1", file=sys.stderr)
        return 1
    if load_u32(blob, 0x8003F3D4) != 0x0C00FC1D:
        print("FAIL: 3F3C4 first jal is not 3F074", file=sys.stderr)
        return 1
    print("PASS: 0x2A/17A50 bit-or + 3F3C4@3F3D4 jal 3F074")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
