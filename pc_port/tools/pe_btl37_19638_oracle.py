#!/usr/bin/env python3
"""PE-BTL37 independent oracle: 0xAB / 19638 overlay bit clear."""
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

    require((0x80019658 - 0x80019638) // 4 == 8, "19638 8w")
    require(
        window_sha(data, 0x80019638, 0x80019658)
        == "03ea97754f90ff447c65301cb95b4ca128a116d9f310b793a880ea5360897afc",
        "19638 sha",
    )
    require(load_u32(data, 0x800910A0 + 0xAB * 4) == 0x80019638, "table[0xAB]")
    require(load_u32(data, 0x8001963C) == 0x24420CD8, "addiu B0CD8")
    require(load_u32(data, 0x80019644) == 0x2404DFFF, "li ~0x2000")
    require(load_u32(data, 0x80019648) == 0x00641824, "and")
    require(load_u32(data, 0x80019654) == 0x24020001, "v0=1")

    print("PASS: 0xAB 19638 D_800B0CD8 &= ~0x2000; v0=1")
    return 0


if __name__ == "__main__":
    sys.exit(main())
