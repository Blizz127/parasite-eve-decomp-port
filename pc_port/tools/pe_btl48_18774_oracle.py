#!/usr/bin/env python3
"""PE-BTL48 independent oracle: 0x6A / 18774, 6F39C, D4620, CE49C."""
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

    require((0x800187C0 - 0x80018774) // 4 == 19, "18774 19w")
    require(
        window_sha(data, 0x80018774, 0x800187C0)
        == "e32a287480c5a8288006e303911abbd07820569cbf1680e52e189a8b91e2d822",
        "18774 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x6A * 4) == 0x80018774, "table[0x6A]")
    require(jal_target(load_u32(data, 0x80018794)) == 0x8006F39C, "jal 6F39C")
    require(load_u32(data, 0x800187A8) == 0x24020001, "0x6A v0=1")

    require((0x8006F6D4 - 0x8006F39C) // 4 == 206, "6F39C 206w")
    require(
        window_sha(data, 0x8006F39C, 0x8006F6D4)
        == "ee236f21452e26032a9440c812ba083c7d3ee2d4462d055459392c8b0d488337",
        "6F39C sha",
    )
    require(load_u32(data, 0x8006F3B8) == 0x2E4200C0, "sltiu 0xC0")
    require(jal_target(load_u32(data, 0x8006F4F8)) == 0x8006914C, "jal 6914C")
    require(load_u32(data, 0x800942E0) == 0x80094188, "942E0 table")
    require(load_u32(data, 0x80094188 + 0x55 * 4) == 0x800E13D4, "table[0x55]")
    require(load_u32(data, 0x800E13D8) == 0x800D4620, "entry+4 D4620")
    require(jal_target(load_u32(data, 0x8006F67C)) == 0x800CE49C, "jal CE49C")

    require((0x800D4698 - 0x800D4620) // 4 == 30, "D4620 30w")
    require(
        window_sha(data, 0x800D4620, 0x800D4698)
        == "52e49e7de4b044d35b3180855136124f95cac4542bd163a9a67d6ef2182e10a5",
        "D4620 sha",
    )
    require((0x800CE4F8 - 0x800CE49C) // 4 == 23, "CE49C 23w")
    require(
        window_sha(data, 0x800CE49C, 0x800CE4F8)
        == "65cc4cd5691e74dcbcd7bba11570cd4ca80b40c6b2dbde7ddd3ec2529ca963bc",
        "CE49C sha",
    )
    require(load_u32(data, 0x800E1044 + 0x20 * 4) == 0, "E1044[0x20] empty")

    print("PASS: 0x6A 18774 jal 6F39C; 0x75 → table 0x55 / D4620")
    return 0


if __name__ == "__main__":
    sys.exit(main())
