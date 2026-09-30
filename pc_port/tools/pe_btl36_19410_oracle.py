#!/usr/bin/env python3
"""PE-BTL36 independent oracle: 0x9C / 19410 fade-wait."""
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

    require((0x80019450 - 0x80019410) // 4 == 16, "19410 16w")
    require(
        window_sha(data, 0x80019410, 0x80019450)
        == "ab0d665bb7e9e59e7726bede5108afe7c5cd444b614ad80b4c8e072c2bfe1ce8",
        "19410 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x9C * 4) == 0x80019410, "table[0x9C]")
    require(load_u32(data, 0x80019410) == 0x3C02800C, "lui CFEE")
    require(load_u32(data, 0x80019414) == 0x9042CFEE, "lbu CFEE")
    require(load_u32(data, 0x8001941C) == 0x30420003, "andi 3")
    require(load_u32(data, 0x80019420) == 0x2C420002, "sltiu 2")
    require(load_u32(data, 0x80019438) == 0x2463FFF8, "CE00-8")
    require(load_u32(data, 0x80019444) == 0xAC830010, "task+0x10=1")

    print("PASS: 0x9C 19410 waits while (CFEE&3)>=2; rewinds CE00")
    return 0


if __name__ == "__main__":
    sys.exit(main())
