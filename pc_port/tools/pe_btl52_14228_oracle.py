#!/usr/bin/env python3
"""PE-BTL52 independent oracle: 0x0E / 14228 actor-field read."""
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

    require((0x800143B0 - 0x80014228) // 4 == 98, "14228 98w")
    require(
        window_sha(data, 0x80014228, 0x800143B0)
        == "e26b9d26bb81e8400fadc7037c6b03483631d3e0832f65ef69155c1ed927bfab",
        "14228 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x0E * 4) == 0x80014228, "table[0x0E]")
    require(load_u32(data, 0x8001430C) == 0x2403FFFF, "miss -1")
    require(load_u32(data, 0x80014368) == 0x90A2000E, "code0 lbu +0x0E")
    require(load_u32(data, 0x800143A4) == 0x24020001, "v0=1")

    print("PASS: 0x0E 14228 field read; miss -1; code0 +0x0E")
    return 0


if __name__ == "__main__":
    sys.exit(main())
