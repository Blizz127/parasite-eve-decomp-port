#!/usr/bin/env python3
"""PE-BTL63 — 6B4F8 dest-load and M0367I table contract."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
TOKEN = 0xA80663C8
TABLE = 0x80093378
CHARSET = 0x800930B4
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


def decode(blob: bytes, token: int) -> str:
    out = []
    for i in range(6):
        idx = (token >> ((5 - i) * 5 + 2)) & 0x1F
        ch = blob[exe_off(CHARSET) + idx]
        if 97 <= ch < 123:
            ch -= 32
        out.append(chr(ch))
    return "".join(out)


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require(decode(blob, TOKEN) == "M0367I", "token name")
    require(load_u32(blob, TABLE + 366 * 8) == 0x15050, "rel")
    packed = load_u32(blob, TABLE + 366 * 8 + 4)
    require(packed == 0x04E0AA21, f"packed {packed:#x}")
    require((packed & 0xFF) == 33, "sec0")
    require(((packed >> 8) & 0xFFF) == 170, "sec1")
    require((packed >> 20) == 78, "sec2")
    require(jal_target(load_u32(blob, 0x8003F088)) == 0x8006B4F8, "3F074 jal")
    require(jal_target(load_u32(blob, 0x8006B594)) == 0x8006E6A8, "chunk0")
    require(jal_target(load_u32(blob, 0x8006B608)) == 0x8006E6A8, "chunk1")
    require(jal_target(load_u32(blob, 0x8006B6EC)) == 0x8006E6A8, "chunk2")
    require(load_u32(blob, 0x8006B578) == 0x8EC50194, "dest +0x194")
    require(load_u32(blob, 0x8006B5E0) == 0x8EC50168, "dest +0x168")
    require(load_u32(blob, 0x8006B6BC) == 0x8EC5018C, "dest +0x18C")
    print("PASS: M0367I table[366] 33+170+78; 6B4F8 three 6E6A8 dests")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
