#!/usr/bin/env python3
"""PE-BTL32 independent oracle: 0x04 / 17988 task-flag walk."""
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

    require((0x800179F8 - 0x80017988) // 4 == 28, "17988 28w")
    require(
        window_sha(data, 0x80017988, 0x800179F8)
        == "2ddb182b06e615769757641f5c2d7a349120daa1cb2a66d52496abe984015c0a",
        "17988 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x04 * 4) == 0x80017988, "table[0x04]")
    require(load_u32(data, 0x80017990) == 0x8CC6D2F0, "lw D2F0")
    require(load_u32(data, 0x80017994) == 0x8F850590, "lw D300")
    require(load_u32(data, 0x800179A4) == 0x8C4300A0, "lw +0xA0")
    require(load_u32(data, 0x800179C4) == 0x34420010, "ori 0x10")
    require(load_u32(data, 0x800179E4) == 0x2C420003, "sltiu 3")
    require(load_u32(data, 0x800179F4) == 0x24020001, "v0=1")

    print("PASS: 0x04 17988 walks +0xA0[0..2]; +8|=0x10 except D300")
    return 0


if __name__ == "__main__":
    sys.exit(main())
