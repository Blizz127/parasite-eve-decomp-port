#!/usr/bin/env python3
"""PE-BTL95 — 144FC jtbl[1..0x36] complete; state>=0x3C parks."""
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


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require(load_u32(blob, 0x80014520) == 0x2C62003C, "sltiu 0x3C")
    require(load_u32(blob, 0x800100A0) == 0x80014544, "jtbl[0]")
    for i in range(1, 0x37):
        require(load_u32(blob, 0x800100A0 + i * 4) == 0x80014658, f"jtbl[{i:02X}]")
    require(load_u32(blob, 0x800100A0 + 0x37 * 4) == 0x80014570, "jtbl[37]")
    require(load_u32(blob, 0x80014658) == 0x0800519F, "complete j")
    print("PASS: 144FC jtbl[1..0x36]=14658; sltiu 0x3C parks")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
