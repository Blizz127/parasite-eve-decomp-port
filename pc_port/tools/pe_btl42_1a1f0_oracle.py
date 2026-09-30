#!/usr/bin/env python3
"""PE-BTL42 independent oracle: 0xDC / 1A1F0 actor+0x98 |= 0x01000000."""
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

    require((0x8001A214 - 0x8001A1F0) // 4 == 9, "1A1F0 9w")
    require(
        window_sha(data, 0x8001A1F0, 0x8001A214)
        == "65cffdf80b045c888fde1b209476a70434ea6cd85c070435bf83fa14337bd872",
        "1A1F0 sha",
    )
    require(load_u32(data, 0x800910A0 + 0xDC * 4) == 0x8001A1F0, "table[0xDC]")
    require(load_u32(data, 0x8001A1F4) == 0x8C42D2F0, "lw D2F0")
    require(load_u32(data, 0x8001A200) == 0x3C040100, "lui 0x01000000")
    require(load_u32(data, 0x8001A204) == 0x00641825, "or")
    require(load_u32(data, 0x8001A208) == 0xAC430098, "sw +0x98")
    require(load_u32(data, 0x8001A210) == 0x24020001, "v0=1")

    print("PASS: 0xDC 1A1F0 D2F0+0x98 |= 0x01000000; v0=1")
    return 0


if __name__ == "__main__":
    sys.exit(main())
