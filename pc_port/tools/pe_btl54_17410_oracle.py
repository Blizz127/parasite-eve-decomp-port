#!/usr/bin/env python3
"""PE-BTL54 independent oracle: 0x0D / 17410 and 0x22 / 177C8."""
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

    require((0x80017444 - 0x80017410) // 4 == 13, "17410 13w")
    require(
        window_sha(data, 0x80017410, 0x80017444)
        == "f84d1f42908c9ba5b90a985acaaa36a54538d99d96ad731305bd82571f45518e",
        "17410 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x0D * 4) == 0x80017410, "table[0x0D]")
    require(load_u32(data, 0x80017428) == 0x84440000, "lh *arg0")
    require(load_u32(data, 0x8001742C) == 0x0C00DD78, "jal 375E0")
    require(load_u32(data, 0x80017438) == 0x24020001, "v0=1")

    require((0x80017820 - 0x800177C8) // 4 == 22, "177C8 22w")
    require(
        window_sha(data, 0x800177C8, 0x80017820)
        == "0da54c5533d32ed6461cf0469efe6d6cf8ae1997377602767ac40b95ede82432",
        "177C8 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x22 * 4) == 0x800177C8, "table[0x22]")
    require(load_u32(data, 0x800177D8) == 0x84440000, "lh *arg0")
    require(load_u32(data, 0x800177DC) == 0x0C00DD52, "jal 37548")
    require(load_u32(data, 0x80017800) == 0x2463FFF4, "CE00-0xC")

    require((0x800375B4 - 0x80037548) // 4 == 27, "37548 27w")
    require(
        window_sha(data, 0x80037548, 0x800375B4)
        == "0fae0d80ac29e0ed58249788b9102261230fedb4054c18c95eb5ab3bbbdf8fc3",
        "37548 sha",
    )
    require(load_u32(data, 0x80037570) == 0x8422CEB8, "lh +0x10")
    require(load_u32(data, 0x8003759C) == 0x2C420004, "sltiu 4")

    require((0x80037864 - 0x800375E0) // 4 == 161, "375E0 161w")
    require(
        window_sha(data, 0x800375E0, 0x80037864)
        == "d7f8b96cf54e7f4e98ec49c081059b5e47916ce9e25a5f9e97294fd1eb58d58a",
        "375E0 sha",
    )
    require(load_u32(data, 0x8003764C) == 0xA028CEA8, "sb state 1")
    require(load_u32(data, 0x80037664) == 0xA429CEB8, "sh id")
    require(load_u32(data, 0x80037694) == 0x10400021, "beqz a1")
    require(load_u32(data, 0x8003F568) == 0x0C00DE1C, "3F3C4 jal 37870")

    print("PASS: 0x0D/17410 + 0x22/177C8 + 375E0/37548; 3F568 jal 37870")
    return 0


if __name__ == "__main__":
    sys.exit(main())
