#!/usr/bin/env python3
"""PE-BTL43 independent oracle: 0x1A / 176FC 70D6C/70DD0 write."""
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

    require((0x80017764 - 0x800176FC) // 4 == 26, "176FC 26w")
    require(
        window_sha(data, 0x800176FC, 0x80017764)
        == "ae4816ac6f1800cfcd48a3a3beb4b37cac8bb1cad2d679ad51302215739e2ab2",
        "176FC sha",
    )
    require(load_u32(data, 0x800910A0 + 0x1A * 4) == 0x800176FC, "table[0x1A]")
    require(load_u32(data, 0x8001770C) == 0x8E020004, "lw arg1")
    require(load_u32(data, 0x80017710) == 0x8E030008, "lw arg2")
    require(load_u32(data, 0x80017720) == 0x14850005, "bne *arg1,*arg2")
    require(jal_target(load_u32(data, 0x80017728)) == 0x80070D6C, "jal 70D6C")
    require(jal_target(load_u32(data, 0x80017738)) == 0x80070DD0, "jal 70DD0")
    require(load_u32(data, 0x80017748) == 0xAC620000, "sw *arg0")
    require(load_u32(data, 0x8001774C) == 0x24020001, "v0=1")

    print("PASS: 0x1A 176FC *arg0 = 70D6C or 70DD0(*arg1,*arg2); v0=1")
    return 0


if __name__ == "__main__":
    sys.exit(main())
