#!/usr/bin/env python3
"""PE-BTL6 independent oracle: 339A0 after 30640.

32 words, 0 jals. Copies 0x80010E38 (4 halfword pairs) onto $sp,
indexes (a0&0xFF)*4, sh to D_8009CE84 / D_8009CE86, sb a0 to
D_8009CE80. Does not import production C. Not ATB/AI/damage.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
START = 0x800339A0
END = 0x80033A20
SHA = "ea7edd1f3db4264b5bd3d116a27f2c9b8b8d841676404862d7fda8b2d8ea86fd"
GP = 0x8009CD70
SRC = 0x80010E38


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")
    require((END - START) // 4 == 32, "32 words")
    require(hashlib.sha256(data[exe_off(START) : exe_off(END)]).hexdigest() == SHA,
            "339A0 sha")
    require(load_u32(data, 0x800339A4) == 0x3C068001, "lui 0x8001")
    require(load_u32(data, 0x800339A8) == 0x24C60E38, "addiu 0x80010E38")
    require(data[exe_off(SRC) : exe_off(SRC) + 16]
            == bytes.fromhex("dd00b1000f00b1000f000f00dd000f00"),
            "4 pairs")
    require(load_u32(data, 0x800339F0) == 0x308200FF, "andi a0 0xFF")
    require(load_u32(data, 0x80033A04) == 0xA7820114, "sh CE84")
    require(GP + 0x114 == 0x8009CE84, "CE84")
    require(load_u32(data, 0x80033A0C) == 0xA3840110, "sb CE80")
    require(GP + 0x110 == 0x8009CE80, "CE80")
    require(load_u32(data, 0x80033A10) == 0xA7820116, "sh CE86")
    require(GP + 0x116 == 0x8009CE86, "CE86")
    for va in range(START, END, 4):
        require((load_u32(data, va) >> 26) != 3, f"no jal at {va:#x}")
    require(jal_target(load_u32(data, 0x80029998)) == START, "29810 jal 339A0")
    print("PASS: 339A0 32w sha ea7edd1f…86fd; 4 pairs; CE80/CE84/CE86; 0 jals")
    return 0


if __name__ == "__main__":
    sys.exit(main())
