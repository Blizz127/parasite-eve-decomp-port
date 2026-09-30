#!/usr/bin/env python3
"""PE-BTL64 — 0x03/17C54 and 661EC ROM contract."""
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


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def window_sha(blob: bytes, start: int, end: int) -> str:
    return hashlib.sha256(blob[exe_off(start) : exe_off(end)]).hexdigest()


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require(load_u32(blob, 0x800910A0 + 0x03 * 4) == 0x80017C54, "table[0x03]")
    require((0x80017C8C - 0x80017C54) // 4 == 14, "17C54 14w")
    require(
        window_sha(blob, 0x80017C54, 0x80017C8C)
        == "4d53aeb570e4a1a69243d19089312f97ae255ebe4f0e9d5c15023ab01d9afa24",
        "17C54 sha",
    )
    require(jal_target(load_u32(blob, 0x80017C74)) == 0x800661EC, "jal 661EC")
    require(load_u32(blob, 0x80017C78) == 0x00003821, "a3=0")
    require((0x80066268 - 0x800661EC) // 4 == 31, "661EC 31w")
    require(
        window_sha(blob, 0x800661EC, 0x80066268)
        == "095ed0474107c1b7124d63d5dc36c7fbc97f82ac409ee06746741734bc0bba28",
        "661EC sha",
    )
    require(load_u32(blob, 0x800661FC) == 0x31020040, "andi 0x40")
    require(load_u32(blob, 0x8006620C) == 0x2402FFED, "return -19")
    print("PASS: 0x03/17C54 jal 661EC; bit40 else -19")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
