#!/usr/bin/env python3
"""PE-BTL33 independent oracle: 0xAA / 19618 overlay bit 0x2000."""
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

    require((0x80019638 - 0x80019618) // 4 == 8, "19618 8w")
    require(
        window_sha(data, 0x80019618, 0x80019638)
        == "5eb9bc4a95cb9c96c069401f5ce77e5a93f7a763bb4cca88d0e0ee65a168c537",
        "19618 sha",
    )
    require(load_u32(data, 0x800910A0 + 0xAA * 4) == 0x80019618, "table[0xAA]")
    require(load_u32(data, 0x8001961C) == 0x24630CD8, "addiu B0CD8")
    require(load_u32(data, 0x80019628) == 0x34422000, "ori 0x2000")
    require(load_u32(data, 0x80019634) == 0x24020001, "v0=1")

    print("PASS: 0xAA 19618 D_800B0CD8 |= 0x2000; v0=1")
    return 0


if __name__ == "__main__":
    sys.exit(main())
