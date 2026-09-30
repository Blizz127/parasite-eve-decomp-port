#!/usr/bin/env python3
"""PE-BTL93 — 16910 opcode 0xED key 0xA29 stores actor+0x27D."""
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
    require(load_u32(blob, 0x800910A0 + 0xED * 4) == 0x80016910, "table[0xED]")
    require(load_u32(blob, 0x800169C4) == 0x24020A29, "li 0xA29")
    require(load_u32(blob, 0x800169C8) == 0x106200AB, "beq 16C78")
    require(load_u32(blob, 0x80016C80) == 0x8C63D2F0, "lw D2F0")
    require(load_u32(blob, 0x80016C8C) == 0xA062027D, "sb +0x27D")
    require(load_u32(blob, 0x80016DE0) == 0x24020001, "v0=1")
    print("PASS: 0xED key 0xA29 sb *arg1 to actor+0x27D")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
