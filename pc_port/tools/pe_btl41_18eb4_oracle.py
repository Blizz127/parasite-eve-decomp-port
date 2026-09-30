#!/usr/bin/env python3
"""PE-BTL41 independent oracle: 0x85 / 18EB4 / 66B60."""
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

    require((0x80018EE0 - 0x80018EB4) // 4 == 11, "18EB4 11w")
    require(
        window_sha(data, 0x80018EB4, 0x80018EE0)
        == "1a18600c97dbc999f8a4058500aec97ad09269a3c6aa5a7f76c3f776e1319d43",
        "18EB4 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x85 * 4) == 0x80018EB4, "table[0x85]")
    require(load_u32(data, 0x80018EC4) == 0x94440000, "lhu a0")
    require(jal_target(load_u32(data, 0x80018EC8)) == 0x80066B60, "jal 66B60")
    require(load_u32(data, 0x80018ED4) == 0x24020001, "v0=1")

    require((0x80066BD8 - 0x80066B60) // 4 == 30, "66B60 30w")
    require(
        window_sha(data, 0x80066B60, 0x80066BD8)
        == "862ff6185fcd6ef374b96fb285a567cf59af62e7e30bdf89ad7f41f6ca0ee45e",
        "66B60 sha",
    )
    require(load_u32(data, 0x80066B80) == 0xA422CFE8, "sh 0xFF CFE8")
    require(load_u32(data, 0x80066B9C) == 0xA022CFEE, "sb 2 CFEE")
    require(load_u32(data, 0x80066BA4) == 0xA022CFEF, "sb 2 CFEF")
    require(load_u32(data, 0x80066BAC) == 0xA424CFF6, "sh a0 CFF6")
    require(load_u32(data, 0x80066BD4) == 0x00001021, "v0=0")

    print("PASS: 0x85 18EB4 jal 66B60(lhu); CFEE=2 CFE8=0xFF CFF6=a0")
    return 0


if __name__ == "__main__":
    sys.exit(main())
