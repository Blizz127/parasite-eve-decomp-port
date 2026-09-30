#!/usr/bin/env python3
"""PE-BTL25 independent oracle: type-3 0x5E pose-copy 14694."""
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

    require((0x800148E0 - 0x80014694) // 4 == 147, "14694 147w")
    require(
        window_sha(data, 0x80014694, 0x800148E0)
        == "97c3ff9a4d8dfb7dded030d56219c06a611f04637cb4d50f29ef7e6b5874059e",
        "14694 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x5E * 4) == 0x80014694, "table[0x5E]")
    require(load_u32(data, 0x800146A4) == 0x14400008, "bne *arg1")
    require(load_u32(data, 0x800146B0) == 0x8C42D254, "lw D254")
    require(load_u32(data, 0x800146CC) == 0x8CA5D20C, "lw D20C")
    require(load_u32(data, 0x8001470C) == 0x30420010, "andi +0x98 0x10")
    require(load_u32(data, 0x80014740) == 0xAC830000, "miss sw -1")
    require(load_u32(data, 0x80010190) == 0x80014784, "jtbl[0] +0x28")
    require(load_u32(data, 0x800101A4) == 0x80014874, "jtbl[5] lh +0x38")
    require(load_u32(data, 0x800148D4) == 0x24020001, "v0=1")

    print("PASS: 0x5E 14694 D254/walk pose-copy; miss stores -1; v0=1")
    return 0


if __name__ == "__main__":
    sys.exit(main())
