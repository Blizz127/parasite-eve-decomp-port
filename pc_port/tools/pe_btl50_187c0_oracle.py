#!/usr/bin/env python3
"""PE-BTL50 independent oracle: 0x6B / 187C0, 6F6D4, D4698."""
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

    require((0x80018818 - 0x800187C0) // 4 == 22, "187C0 22w")
    require(
        window_sha(data, 0x800187C0, 0x80018818)
        == "1e7332235b2aaa5689ec503658f0e072d2a8237ac38df8136337bc940e8dda40",
        "187C0 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x6B * 4) == 0x800187C0, "table[0x6B]")
    require(jal_target(load_u32(data, 0x80018800)) == 0x8006F6D4, "jal 6F6D4")
    require(load_u32(data, 0x80018804) == 0x00002821, "a1=0 delay")
    require(load_u32(data, 0x8001880C) == 0x24020001, "0x6B v0=1")

    require((0x8006F820 - 0x8006F6D4) // 4 == 83, "6F6D4 83w")
    require(
        window_sha(data, 0x8006F6D4, 0x8006F820)
        == "c38426e24571b4d0a03d6896c0356be4a5309221a9a27cc29fde229703411389",
        "6F6D4 sha",
    )
    require(load_u32(data, 0x8006F6E0) == 0x2C820016, "sltiu 0x16")
    require(load_u32(data, 0x8006F6F0) == 0x2402FFF6, "OOB -10")
    require(load_u32(data, 0x800E13DC) == 0x800D4698, "entry+8 D4698")

    require((0x800D4704 - 0x800D4698) // 4 == 27, "D4698 27w")
    require(
        window_sha(data, 0x800D4698, 0x800D4704)
        == "c33ca3b98d2a0fbcc7b8063612a91d0ccf9cc1f7020dc3fd57c34932ad556991",
        "D4698 sha",
    )
    require(load_u32(data, 0x800D46C4) == 0xAC3032D0, "sw slot F32D0")
    require(load_u32(data, 0x800D46CC) == 0xAC232368, "sw slot+0xC E2368")

    print("PASS: 0x6B 187C0 jal 6F6D4 a1=0; table[0x55]+8 D4698")
    return 0


if __name__ == "__main__":
    sys.exit(main())
