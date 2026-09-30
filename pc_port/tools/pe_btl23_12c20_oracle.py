#!/usr/bin/env python3
"""PE-BTL23 independent oracle: type-5 0x0B / 12C20 and 0x41 / 17D9C.

Pins SHA-1-exact EXE. Does not import production C.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def window_sha(data: bytes, start: int, end: int) -> str:
    return hashlib.sha256(data[exe_off(start) : exe_off(end)]).hexdigest()


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")

    require((0x80012E7C - 0x80012C20) // 4 == 151, "12C20 151w")
    require(
        window_sha(data, 0x80012C20, 0x80012E7C)
        == "18e047de75127c2966de09761932087e77850f5d429053afe64f3f09d0eb22a8",
        "12C20 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x0B * 4) == 0x80012C20, "table[0x0B]")
    require(load_u32(data, 0x80012C3C) == 0x2C620007, "sltiu 7")
    require(load_u32(data, 0x80010060) == 0x80012C64, "jtbl[0]")
    require(load_u32(data, 0x80010074) == 0x80012DEC, "jtbl[5]")
    require(jal_target(load_u32(data, 0x80012C9C)) == 0x8001AA78, "code0 jal 1AA78")
    require(load_u32(data, 0x80012C78) == 0xAC820028, "code0 +0x28")
    require(load_u32(data, 0x80012CC0) == 0xAC430040, "snap +0x40")
    require(load_u32(data, 0x80012E00) == 0xA4620038, "code5 sh +0x38")
    require(load_u32(data, 0x80012E70) == 0x24020001, "v0=1")

    require((0x80017DC0 - 0x80017D9C) // 4 == 9, "17D9C 9w")
    require(
        window_sha(data, 0x80017D9C, 0x80017DC0)
        == "77759305b9ddd30fa820c94b1cdf19706a13214d8878eb28c78ca4f6fd0a9549",
        "17D9C sha",
    )
    require(load_u32(data, 0x800910A0 + 0x41 * 4) == 0x80017D9C, "table[0x41]")
    require(load_u32(data, 0x80017DB0) == 0x34420040, "ori +0x98 0x40")
    require(load_u32(data, 0x80017DBC) == 0x24020001, "0x41 v0=1")

    print("PASS: 0x0B=12C20 151w sha 18e047de… 7 codes jal 1AA78; 0x41=17D9C 9w +0x98|=0x40")
    return 0


if __name__ == "__main__":
    sys.exit(main())
