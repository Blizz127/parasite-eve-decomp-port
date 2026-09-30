#!/usr/bin/env python3
"""PE-BTL56 independent oracle: 3EB04 pad-edge sites."""
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

    require((0x8003F074 - 0x8003EB04) // 4 == 348, "3EB04 348w")
    require(
        window_sha(data, 0x8003EB04, 0x8003F074)
        == "c6a27aa96c180ee441866ac86dfc995683fb48c2366dfd864f0bbd03aeb4660b",
        "3EB04 sha",
    )
    require(load_u32(data, 0x8003F40C) == 0x0C00FAC1, "3F3C4 jal 3EB04")
    require(load_u32(data, 0x8003EC6C) == 0xAF8204C8, "sw previous")
    require(load_u32(data, 0x8003ED00) == 0x2C420020, "sltiu 32")
    require(load_u32(data, 0x8003F03C) == 0x00441826, "xor held^prev")
    require(load_u32(data, 0x8003F040) == 0x00621024, "and edge")
    require(load_u32(data, 0x8003F048) == 0xAF820484, "sw D1F4")
    require(load_u32(data, 0x8003EC54) == 0x9484E9A2, "lhu BE9A2")

    print("PASS: 3EB04 348w + 3F40C jal + D1F4 edge")
    return 0


if __name__ == "__main__":
    sys.exit(main())
