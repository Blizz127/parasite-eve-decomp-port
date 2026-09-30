#!/usr/bin/env python3
"""PE-BTL35 independent oracle: 0x82 / 18E58 → 66800."""
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


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")

    require((0x80018E84 - 0x80018E58) // 4 == 11, "18E58 11w")
    require(
        window_sha(data, 0x80018E58, 0x80018E84)
        == "99b34ecf54facb304c194825efe205c4a02d01e09b2d3089cd50cdf45336def3",
        "18E58 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x82 * 4) == 0x80018E58, "table[0x82]")
    require(jal_target(load_u32(data, 0x80018E6C)) == 0x80066800, "jal 66800")
    require(load_u32(data, 0x80018E60) == 0x8C820000, "lw *arg0")
    require(load_u32(data, 0x80018E68) == 0x8C440000, "lw **arg0 → a0")
    require(load_u32(data, 0x80018E78) == 0x24020001, "v0=1")
    require((0x8006698C - 0x80066800) // 4 == 99, "66800 99w")
    require(load_u32(data, 0x80066800) == 0x3C02800B, "66800 lui B1624")
    require(load_u32(data, 0x80066804) == 0x8C421624, "66800 lw B1624")

    print("PASS: 0x82 18E58 jal 66800(*arg0); v0=1")
    return 0


if __name__ == "__main__":
    sys.exit(main())
