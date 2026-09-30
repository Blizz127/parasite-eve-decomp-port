#!/usr/bin/env python3
"""PE-BTL55 independent oracle: 37870 message updater sites."""
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

    require((0x80038910 - 0x80037870) // 4 == 1064, "37870 1064w")
    require(
        window_sha(data, 0x80037870, 0x80038910)
        == "2b0eeaf2ed110c388b805b4cb3759bcf0161f1d35cc434a218d5b8f68d8c9d4d",
        "37870 sha",
    )
    require(load_u32(data, 0x8003F568) == 0x0C00DE1C, "3F3C4 jal 37870")
    require(load_u32(data, 0x8003F554) == 0x30420200, "andi 0x200")
    require(load_u32(data, 0x8003F500) == 0x30420100, "andi 0x100")
    require(load_u32(data, 0x8003798C) == 0x8F830120, "lw gp+0x120")
    require(load_u32(data, 0x80037974) == 0x240600F9, "addiu F9")
    require(load_u32(data, 0x80037978) == 0x240500FE, "addiu FE")
    require(load_u32(data, 0x80037C50) == 0xA2A00000, "F9 sb state 0")
    require(load_u32(data, 0x80037C98) == 0xA2A00000, "FF sb state 0")
    require(load_u32(data, 0x800388B4) == 0xA2A20000, "1→2 sb")
    require(load_u32(data, 0x80037C78) == 0x30420100, "FF andi D1F4 0x100")

    print("PASS: 37870 1064w + 3F568 jal + F9/FF/1→2 sites")
    return 0


if __name__ == "__main__":
    sys.exit(main())
