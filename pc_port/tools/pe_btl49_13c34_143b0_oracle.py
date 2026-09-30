#!/usr/bin/env python3
"""PE-BTL49 independent oracle: 0x4B / 13C34 and 0x54 / 143B0."""
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

    require((0x80013E84 - 0x80013C34) // 4 == 148, "13C34 148w")
    require(
        window_sha(data, 0x80013C34, 0x80013E84)
        == "0c50833455e6f657ea99916003152888e8fe71f1e4c93be8b0016638d6015de2",
        "13C34 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x4B * 4) == 0x80013C34, "table[0x4B]")
    require(load_u32(data, 0x80013C4C) == 0x30A20020, "andi bit 0x20")
    require(jal_target(load_u32(data, 0x80013D68)) == 0x80079FB4, "jal 79FB4")
    require(load_u32(data, 0x80013E44) == 0x2463FFEC, "CE00-0x14")

    require((0x800144FC - 0x800143B0) // 4 == 83, "143B0 83w")
    require(
        window_sha(data, 0x800143B0, 0x800144FC)
        == "53cb113139a8862dfd200dd94fbf4f18e5eaec26144f7cc64530a39e3613a629",
        "143B0 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x54 * 4) == 0x800143B0, "table[0x54]")
    require(load_u32(data, 0x80014454) == 0x2403FFFF, "miss -1")
    require(load_u32(data, 0x800144F0) == 0x24020001, "0x54 v0=1")

    print("PASS: 0x4B 13C34 jal 79FB4 turn; 0x54 143B0 pose distance")
    return 0


if __name__ == "__main__":
    sys.exit(main())
