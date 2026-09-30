#!/usr/bin/env python3
"""PE-BTL91 — 0x55 / 144FC park rewind."""
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
    require(load_u32(blob, 0x800910A0 + 0x55 * 4) == 0x800144FC, "table[0x55]")
    require(load_u32(blob, 0x80014660) == 0x00001021, "v0=0 park")
    require(load_u32(blob, 0x8001466C) == 0x2463FFF4, "CE00-0xC")
    require(load_u32(blob, 0x80014658) == 0x0800519F, "complete j")
    require(load_u32(blob, 0x8001465C) == 0x24020001, "v0=1")
    print("PASS: 0x55 144FC park rewinds CE00 by 0xC")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
