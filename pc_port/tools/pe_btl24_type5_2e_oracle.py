#!/usr/bin/env python3
"""PE-BTL24 independent oracle: type-5 0x2E / 0x4E / 0x2F / 0x30."""
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

    require((0x80017B34 - 0x80017AE8) // 4 == 19, "17AE8 19w")
    require(
        window_sha(data, 0x80017AE8, 0x80017B34)
        == "d9d0acb125c0ebe6c12b364f82377bf66ec42600f1db15399870c6123ccd53bc",
        "17AE8 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x2E * 4) == 0x80017AE8, "table[0x2E]")
    require(jal_target(load_u32(data, 0x80017B00)) == 0x8001A680, "jal 1A680")
    require(load_u32(data, 0x80017B18) == 0x2403FEFF, "andi ~0x100")
    require(load_u32(data, 0x80017B28) == 0x24020001, "0x2E v0=1")

    require((0x80017EFC - 0x80017EC4) // 4 == 14, "17EC4 14w")
    require(
        window_sha(data, 0x80017EC4, 0x80017EFC)
        == "2d212361116a4c0891cd92a52d89b052e82a55bdf43a78dd2639856d05bf74d2",
        "17EC4 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x4E * 4) == 0x80017EC4, "table[0x4E]")
    require(load_u32(data, 0x80017EF0) == 0xACA20014, "sw +0x14")

    require((0x80017B74 - 0x80017B34) // 4 == 16, "17B34 16w")
    require(
        window_sha(data, 0x80017B34, 0x80017B74)
        == "6c2e931e6bfac4b01d487ebe69506625be979e9c4bd0fcd71352a8607453bfa9",
        "17B34 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x2F * 4) == 0x80017B34, "table[0x2F]")
    require(load_u32(data, 0x80017B64) == 0x34420200, "ori 0x200")

    require((0x80017BB4 - 0x80017B74) // 4 == 16, "17B74 16w")
    require(
        window_sha(data, 0x80017B74, 0x80017BB4)
        == "0cbe81597ab308beb25b19c68d57d22da4593ad863dc3cc4762438d28fef1fce",
        "17B74 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x30 * 4) == 0x80017B74, "table[0x30]")
    require(load_u32(data, 0x80017B84) == 0xAC620010, "task+0x10=1")
    require(load_u32(data, 0x80017BA4) == 0x2463FFF8, "CE00-8")
    require(load_u32(data, 0x80017B98) == 0x00001021, "v0=0")

    print("PASS: 0x2E jal 1A680 +0x98&=~0x100; 0x4E +0x14; 0x2F +0x12|=0x200; 0x30 v0=0")
    return 0


if __name__ == "__main__":
    sys.exit(main())
