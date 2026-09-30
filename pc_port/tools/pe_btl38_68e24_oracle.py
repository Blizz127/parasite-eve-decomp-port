#!/usr/bin/env python3
"""PE-BTL38 independent oracle: 68E24 fade tick + 3F3C4 sites."""
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


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")

    require((0x8006914C - 0x80068E24) // 4 == 202, "68E24 202w")
    require(
        window_sha(data, 0x80068E24, 0x8006914C)
        == "bee869b9d9857c6d707e68201a813590713350f965c963eedde3c1673aa84267",
        "68E24 sha",
    )
    require(load_u32(data, 0x80068E2C) == 0x9042CFEE, "lbu CFEE")
    require(load_u32(data, 0x80068E38) == 0x304C0003, "andi 3")
    require(load_u32(data, 0x80069134) == 0xA1600066, "sb 0 CFEE")
    require(load_u32(data, 0x80069138) == 0xA16D0066, "sb 1 CFEE")
    require(jal_target(load_u32(data, 0x8003F4E8)) == 0x80065400, "3F3C4 jal 65400")
    require(jal_target(load_u32(data, 0x8003F4F0)) == 0x80035558, "3F3C4 jal 35558")
    require(jal_target(load_u32(data, 0x8003F588)) == 0x80068E24, "3F3C4 jal 68E24")
    require(jal_target(load_u32(data, 0x8006EB4C)) == 0x80068E24, "6E9A0 jal 68E24")
    require(load_u32(data, 0x8003F500) == 0x30420100, "B0CD8&0x100")
    require(load_u32(data, 0x8003F554) == 0x30420200, "B0CD8&0x200")

    print("PASS: 68E24 202w fade tick; 3F3C4 @ 3F588; 6E9A0 @ 6EB4C")
    return 0


if __name__ == "__main__":
    sys.exit(main())
