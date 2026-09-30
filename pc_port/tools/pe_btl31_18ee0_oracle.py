#!/usr/bin/env python3
"""PE-BTL31 independent oracle: 0x86 / 18EE0 / 66C7C."""
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

    require((0x80018F0C - 0x80018EE0) // 4 == 11, "18EE0 11w")
    require(
        window_sha(data, 0x80018EE0, 0x80018F0C)
        == "d042e2ad1ba21c6b4f3824184c8c826226f8d689ee712dd72dcd349aad3c9fec",
        "18EE0 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x86 * 4) == 0x80018EE0, "table[0x86]")
    require(load_u32(data, 0x80018EF0) == 0x94440000, "lhu a0")
    require(jal_target(load_u32(data, 0x80018EF4)) == 0x80066C7C, "jal 66C7C")
    require(load_u32(data, 0x80018F00) == 0x24020001, "v0=1")

    require((0x80066CE8 - 0x80066C7C) // 4 == 27, "66C7C 27w")
    require(
        window_sha(data, 0x80066C7C, 0x80066CE8)
        == "ce26b53b3c92cae7ffdaa7c393586b80b323a6f53fc16853640d60cafba82ef0",
        "66C7C sha",
    )
    require(load_u32(data, 0x80066C9C) == 0xA022CFEE, "sb 6 CFEE")
    require(load_u32(data, 0x80066CBC) == 0xA424CFF6, "sh a0 CFF6")
    require(load_u32(data, 0x80066CE4) == 0x00001021, "v0=0")

    print("PASS: 0x86 18EE0 jal 66C7C(lhu); CFEE=6 CFF6=a0")
    return 0


if __name__ == "__main__":
    sys.exit(main())
