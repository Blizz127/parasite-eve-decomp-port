#!/usr/bin/env python3
"""PE-BTL46 independent oracle: 0x59 / 18004, 2FE78, 3010C."""
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

    require((0x80018080 - 0x80018004) // 4 == 31, "18004 31w")
    require(
        window_sha(data, 0x80018004, 0x80018080)
        == "2993bcdd71ea84c61234eb3b0673a9b6fe014ccdc7172e4a5f40ed535ceef4ac",
        "18004 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x59 * 4) == 0x80018004, "table[0x59]")
    require(load_u32(data, 0x8001801C) == 0x9082000C, "lbu type")
    require(jal_target(load_u32(data, 0x80018038)) == 0x8002FE78, "jal 2FE78")
    require(jal_target(load_u32(data, 0x80018054)) == 0x8003010C, "jal 3010C")
    require(load_u32(data, 0x80018068) == 0x24020001, "0x59 v0=1")

    require((0x8002FF78 - 0x8002FE78) // 4 == 64, "2FE78 64w")
    require(
        window_sha(data, 0x8002FE78, 0x8002FF78)
        == "1094644057c987dfd92ddc6d474bb133380ebb678bcc501ca19d7986f710c3aa",
        "2FE78 sha",
    )
    require(load_u32(data, 0x8002FE80) == 0x8C42D254, "lw D254")
    require(load_u32(data, 0x8002FE88) == 0x8C450000, "lw *D254")
    require(load_u32(data, 0x8002FE90) == 0x2406FC18, "sentinel -1000")

    require((0x80030220 - 0x8003010C) // 4 == 69, "3010C 69w")
    require(
        window_sha(data, 0x8003010C, 0x80030220)
        == "10bca0574dde04e6cac660cbc064b249c8381ecd7fae4f09abd12be29f9c8787",
        "3010C sha",
    )
    require(load_u32(data, 0x8003010C) == 0x8C840000, "slot=*actor")
    require(load_u32(data, 0x80030114) == 0x24A5FFD7, "tag-41")
    require(load_u32(data, 0x80030120) == 0x2403FC18, "sentinel -1000")
    require(load_u32(data, 0x80010B28 + 3 * 4) == 0x80030158, "tag44 case")
    require(load_u32(data, 0x80030158) == 0x8C830010, "lw +0x10")

    print("PASS: 0x59 18004 jal 2FE78/3010C; tag 44 reads slot+0x10")
    return 0


if __name__ == "__main__":
    sys.exit(main())
