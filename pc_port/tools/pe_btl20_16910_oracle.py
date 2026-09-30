#!/usr/bin/env python3
"""PE-BTL20 independent oracle: 16910 opcode 0xED live key 2900.

Pins SHA-1-exact EXE. 314 words. Key 0xB54/2900 ORs overlay[0]
0x00400000. Does not import production C. Other keys not this cut.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FN = 0x80016910
END = 0x80016DF8
SHA = "273744b7a256a5b7dfbbef28350c46c0b2543fd2dd2bcf45c0fdbfc709b68c37"


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def window_sha(data: bytes, start: int, end: int) -> str:
    return hashlib.sha256(data[exe_off(start) : exe_off(end)]).hexdigest()


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")
    require((END - FN) // 4 == 314, "16910 314 words")
    require(window_sha(data, FN, END) == SHA, "16910 sha")
    require(load_u32(data, 0x800910A0 + 0xED * 4) == FN, "table[0xED]")
    require(load_u32(data, 0x80016A14) == 0x24020B54, "li 2900")
    require(load_u32(data, 0x80016A18) == 0x106200B9, "beq 16D00")
    require(load_u32(data, 0x80016D00) == 0x3C02800B, "case lui B0CD8")
    require(load_u32(data, 0x80016D10) == 0x3C040040, "lui 0x400000")
    require(load_u32(data, 0x80016DD8) == 0x00641825, "or overlay[0]")
    require(load_u32(data, 0x80016DE0) == 0x24020001, "v0=1")
    print("PASS: 16910 314w; live key 2900 D_800B0CD8|=0x00400000 v0=1")
    return 0


if __name__ == "__main__":
    sys.exit(main())
