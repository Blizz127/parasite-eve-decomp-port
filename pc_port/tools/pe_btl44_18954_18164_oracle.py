#!/usr/bin/env python3
"""PE-BTL44 independent oracle: 0x6F / 18954 and 0x5A / 18164."""
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

    require((0x8001897C - 0x80018954) // 4 == 10, "18954 10w")
    require(
        window_sha(data, 0x80018954, 0x8001897C)
        == "9dc14475c81f4ab72ae29b688b942204b70d12ec2784e96ccc7455c60503a5ff",
        "18954 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x6F * 4) == 0x80018954, "table[0x6F]")
    require(load_u32(data, 0x80018958) == 0x8C84D2F0, "lw D2F0")
    require(jal_target(load_u32(data, 0x80018964)) == 0x8002F7D8, "jal 2F7D8")
    require(load_u32(data, 0x80018970) == 0x24020001, "0x6F v0=1")

    require((0x800181CC - 0x80018164) // 4 == 26, "18164 26w")
    require(
        window_sha(data, 0x80018164, 0x800181CC)
        == "b2d7e11cfea183f4b42cbaa8d347ca9fbc76ea4b2bfb00a877dc8ea32e5b9c8f",
        "18164 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x5A * 4) == 0x80018164, "table[0x5A]")
    require(load_u32(data, 0x80018174) == 0x90E2000C, "lbu type")
    require(jal_target(load_u32(data, 0x80018194)) == 0x8002FF78, "jal 2FF78")
    require(jal_target(load_u32(data, 0x800181B4)) == 0x80030220, "jal 30220")
    require(load_u32(data, 0x800181B8) == 0x00E02021, "a0=D2F0 delay")
    require(load_u32(data, 0x800181C0) == 0x24020001, "0x5A v0=1")

    print("PASS: 0x6F 18954 jal 2F7D8(D2F0); 0x5A 18164 2FF78/30220")
    return 0


if __name__ == "__main__":
    sys.exit(main())
