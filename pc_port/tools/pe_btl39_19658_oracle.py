#!/usr/bin/env python3
"""PE-BTL39 independent oracle: 0x1E / 19658 actor+0x98 |= 0x80."""
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

    require((0x8001967C - 0x80019658) // 4 == 9, "19658 9w")
    require(
        window_sha(data, 0x80019658, 0x8001967C)
        == "cadffdfc008ac07594524717f0f28c24dca01c8e459f9d0b7f54c1891ecf6fcd",
        "19658 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x1E * 4) == 0x80019658, "table[0x1E]")
    require(load_u32(data, 0x8001965C) == 0x8C63D2F0, "lw D2F0")
    require(load_u32(data, 0x8001966C) == 0x34420080, "ori 0x80")
    require(load_u32(data, 0x80019670) == 0xAC620098, "sw +0x98")
    require(load_u32(data, 0x80019678) == 0x24020001, "v0=1")

    print("PASS: 0x1E 19658 D2F0+0x98 |= 0x80; v0=1")
    return 0


if __name__ == "__main__":
    sys.exit(main())
