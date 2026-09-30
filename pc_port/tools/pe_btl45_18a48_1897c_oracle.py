#!/usr/bin/env python3
"""PE-BTL45 independent oracle: 0xB7 / 18A48 and 0x70 / 1897C."""
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

    require((0x80018A9C - 0x80018A48) // 4 == 21, "18A48 21w")
    require(
        window_sha(data, 0x80018A48, 0x80018A9C)
        == "9218163e5d2548a4818a88b42990c69a41aa277d893c9d51aceb313acea3d862",
        "18A48 sha",
    )
    require(load_u32(data, 0x800910A0 + 0xB7 * 4) == 0x80018A48, "table[0xB7]")
    require(jal_target(load_u32(data, 0x80018A84)) == 0x8002FAA4, "jal 2FAA4")
    require(load_u32(data, 0x80018A90) == 0x24020001, "0xB7 v0=1")

    require((0x80018A48 - 0x8001897C) // 4 == 51, "1897C 51w")
    require(
        window_sha(data, 0x8001897C, 0x80018A48)
        == "64cd42b0fc4be024ba3bcdad5b717ab8377585e71e3f821c33b72c2c06fd0ff3",
        "1897C sha",
    )
    require(load_u32(data, 0x800910A0 + 0x70 * 4) == 0x8001897C, "table[0x70]")
    require(load_u32(data, 0x800189C4) == 0x80420000, "lb arg5")
    require(jal_target(load_u32(data, 0x80018A30)) == 0x8002FA10, "jal 2FA10")
    require(load_u32(data, 0x80018A3C) == 0x24020001, "0x70 v0=1")

    print("PASS: 0xB7 18A48 jal 2FAA4; 0x70 1897C jal 2FA10")
    return 0


if __name__ == "__main__":
    sys.exit(main())
