#!/usr/bin/env python3
"""PE-BTL29 independent oracle: 0x24 / 1784C task+0x18/+0x1C copy."""
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

    require((0x8001787C - 0x8001784C) // 4 == 12, "1784C 12w")
    require(
        window_sha(data, 0x8001784C, 0x8001787C)
        == "0a67c25da84a739e7c9b979dcc94cde06a7e6f3fb35d73ce3b2ec5b7406c17e9",
        "1784C sha",
    )
    require(load_u32(data, 0x800910A0 + 0x24 * 4) == 0x8001784C, "table[0x24]")
    require(load_u32(data, 0x8001784C) == 0x8F820590, "lw gp+0x590")
    require(load_u32(data, 0x80017854) == 0x8C420018, "lw +0x18")
    require(load_u32(data, 0x80017868) == 0x8C42001C, "lw +0x1C")
    require(load_u32(data, 0x80017874) == 0x03E00008, "jr ra")
    require(load_u32(data, 0x80017878) == 0x24020001, "v0=1")
    require(0x8009CD70 + 0x590 == 0x8009D300, "gp+0x590 is D300")

    print("PASS: 0x24 1784C copies D300+0x18/+0x1C; v0=1")
    return 0


if __name__ == "__main__":
    sys.exit(main())
