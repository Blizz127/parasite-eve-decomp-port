#!/usr/bin/env python3
"""PE-BTL21 independent oracle: type-1 0xE1 / 0x84 / 0x88.

Pins SHA-1-exact EXE. Three small v0=1 stores. Does not import
production C. 0x08 spawn is not this cut.
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


def window_sha(data: bytes, start: int, end: int) -> str:
    return hashlib.sha256(data[exe_off(start) : exe_off(end)]).hexdigest()


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")
    require((0x8001A390 - 0x8001A374) // 4 == 7, "1A374 7w")
    require(
        window_sha(data, 0x8001A374, 0x8001A390)
        == "13a283233d7a467d759906c1df11d1f18ddc98062b368d5d5d2bde15485c808b",
        "1A374 sha",
    )
    require(load_u32(data, 0x800910A0 + 0xE1 * 4) == 0x8001A374, "table[0xE1]")
    require(load_u32(data, 0x8001A384) == 0xA022CFFC, "sb BCFFC")
    require((0x80018EB4 - 0x80018E84) // 4 == 12, "18E84 12w")
    require(
        window_sha(data, 0x80018E84, 0x80018EB4)
        == "92fa802d36600aa688f2389bad556472d32ef21df099834d15e1d85efc954695",
        "18E84 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x84 * 4) == 0x80018E84, "table[0x84]")
    require(load_u32(data, 0x80018E94) == 0xA422D020, "sh D020")
    require(load_u32(data, 0x80018EA8) == 0xA422D022, "sh D022")
    for lui_pc, store_pc, expected in (
        (0x80018E90, 0x80018E94, 0x800BD020),
        (0x80018EA4, 0x80018EA8, 0x800BD022),
    ):
        lui, store = load_u32(data, lui_pc), load_u32(data, store_pc)
        require(lui >> 16 == 0x3C01, "yaw address LUI $at")
        low = store & 0xFFFF
        signed_low = low - 0x10000 if low & 0x8000 else low
        address = (((lui & 0xFFFF) << 16) + signed_low) & 0xFFFFFFFF
        require(address == expected, "yaw store includes signed low address half")
    require((0x80018F74 - 0x80018F54) // 4 == 8, "18F54 8w")
    require(
        window_sha(data, 0x80018F54, 0x80018F74)
        == "00cff29cd5f4e67fe2723eb393ebe63e43bfc643aef947874834de68f40ecfd9",
        "18F54 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x88 * 4) == 0x80018F54, "table[0x88]")
    require(load_u32(data, 0x80018F64) == 0x304200BF, "andi 0xBF")
    print("PASS: 0xE1 sb BCFFC; 0x84 sh 800BD020/800BD022 (signed low half); 0x88 BCFEE&=~0x40; all v0=1")
    return 0


if __name__ == "__main__":
    sys.exit(main())
