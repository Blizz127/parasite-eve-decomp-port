#!/usr/bin/env python3
"""PE-BTL40 independent oracle: 0x79 / 18BEC actor+0x98 |= 0x20."""
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

    require((0x80018C10 - 0x80018BEC) // 4 == 9, "18BEC 9w")
    require(
        window_sha(data, 0x80018BEC, 0x80018C10)
        == "d373eee2614c31ce91cbafd9d12402458cff5c59075a95634193320d02e8000c",
        "18BEC sha",
    )
    require(load_u32(data, 0x800910A0 + 0x79 * 4) == 0x80018BEC, "table[0x79]")
    require(load_u32(data, 0x80018BF0) == 0x8C63D2F0, "lw D2F0")
    require(load_u32(data, 0x80018C00) == 0x34420020, "ori 0x20")
    require(load_u32(data, 0x80018C04) == 0xAC620098, "sw +0x98")
    require(load_u32(data, 0x80018C0C) == 0x24020001, "v0=1")

    print("PASS: 0x79 18BEC D2F0+0x98 |= 0x20; v0=1")
    return 0


if __name__ == "__main__":
    sys.exit(main())
