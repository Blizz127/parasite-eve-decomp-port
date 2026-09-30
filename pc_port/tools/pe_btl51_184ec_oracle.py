#!/usr/bin/env python3
"""PE-BTL51 independent oracle: 0x64 / 184EC and 2FAF8."""
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

    require((0x8001856C - 0x800184EC) // 4 == 32, "184EC 32w")
    require(
        window_sha(data, 0x800184EC, 0x8001856C)
        == "9b345fcb4cf481c5258e9136a68c73e9e6fb0064596872e3a5aab6b4f774631d",
        "184EC sha",
    )
    require(load_u32(data, 0x800910A0 + 0x64 * 4) == 0x800184EC, "table[0x64]")
    require(jal_target(load_u32(data, 0x8001850C)) == 0x8002FAF8, "jal 2FAF8")
    require(load_u32(data, 0x80018548) == 0x2484FFF0, "CE00-0x10")
    require(load_u32(data, 0x80018538) == 0x24020001, "v0=1 ready")

    require((0x8002FE78 - 0x8002FAF8) // 4 == 224, "2FAF8 224w")
    require(
        window_sha(data, 0x8002FAF8, 0x8002FE78)
        == "67f58732da924e27b525984b9c1207d40c8ccc24af628d2a4b093e119af54673",
        "2FAF8 sha",
    )
    require(load_u32(data, 0x8002FB34) == 0x30420002, "D1A0 & 2")
    require(load_u32(data, 0x80010A88 + 2 * 4) == 0x8002FD74, "JT[2] activate")
    require(load_u32(data, 0x8002FD74) == 0x324800FF, "activate andi code")

    print("PASS: 0x64 184EC jal 2FAF8; JT[2]=2FD74; wait until rec==4")
    return 0


if __name__ == "__main__":
    sys.exit(main())
