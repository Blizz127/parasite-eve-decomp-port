#!/usr/bin/env python3
"""PE-BTL27 independent oracle: 0x9B / 15240 / 39B74 / 362B8 / 3A6A8."""
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

    require((0x800155FC - 0x80015240) // 4 == 239, "15240 239w")
    require(
        window_sha(data, 0x80015240, 0x800155FC)
        == "fb261fcb8c8022fc71362da343edd64af03a70f1d632eefdbe81bd31b84dc9f8",
        "15240 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x9B * 4) == 0x80015240, "table[0x9B]")
    require(jal_target(load_u32(data, 0x8001528C)) == 0x800794C4, "jal 794C4")
    require(jal_target(load_u32(data, 0x800154A0)) == 0x8006698C, "jal 6698C")
    require(jal_target(load_u32(data, 0x800154BC)) == 0x80039B74, "jal 39B74")
    require(jal_target(load_u32(data, 0x800154CC)) == 0x8003A088, "jal 3A088")
    require(jal_target(load_u32(data, 0x800154E4)) == 0x8003A6A8, "jal 3A6A8")
    require(load_u32(data, 0x8001553C) == 0x10400019, "bit 0x10000000")
    require(load_u32(data, 0x800155A0) == 0x24020001, "v0=1")

    require((0x80039D24 - 0x80039B74) // 4 == 108, "39B74 108w")
    require(
        window_sha(data, 0x80039B74, 0x80039D24)
        == "8c66e3989b7b60284d8c1d70f7f89dd1826626c298406c4db2ff75d898cb84e7",
        "39B74 sha",
    )
    require(load_u32(data, 0x80039B8C) == 0x10A0005E, "beq a1,0")

    require((0x800363F4 - 0x800362B8) // 4 == 79, "362B8 79w")
    require(
        window_sha(data, 0x800362B8, 0x800363F4)
        == "c1d9429303ec09b5ebea35272182d10d64b061ede1dee1c34a637581827db78d",
        "362B8 sha",
    )
    require(load_u32(data, 0x8003A6C4) == 0x14800005, "3A6A8 bne dest+0")
    require(jal_target(load_u32(data, 0x8003A6CC)) == 0x8003E188, "jal 3E188")
    require((0x8003E474 - 0x8003E188) // 4 == 187, "3E188 187w")

    print("PASS: 0x9B 15240; 39B74 a1==0; 362B8; 3A6A8→3E188")
    return 0


if __name__ == "__main__":
    sys.exit(main())
