#!/usr/bin/env python3
"""PE-BTL26 independent oracle: type-3 0x77 / 14DA0 / 1CAB0."""
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

    require((0x80014E30 - 0x80014DA0) // 4 == 36, "14DA0 36w")
    require(
        window_sha(data, 0x80014DA0, 0x80014E30)
        == "e66738b1c528724e908e2251f3afb49b26461e5c9615ebb454fb82744b598201",
        "14DA0 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x77 * 4) == 0x80014DA0, "table[0x77]")
    require(jal_target(load_u32(data, 0x80014E04)) == 0x8001CAB0, "jal 1CAB0")
    require(load_u32(data, 0x80014E08) == 0x24070004, "a3=4")
    require(load_u32(data, 0x80014E18) == 0x24020001, "0x77 v0=1")

    require((0x8001CBA0 - 0x8001CAB0) // 4 == 60, "1CAB0 60w")
    require(
        window_sha(data, 0x8001CAB0, 0x8001CBA0)
        == "e011a7b85252158b0bfe62fab414365b0e3172acd3e0f36d3f22b05478099ebc",
        "1CAB0 sha",
    )
    require(load_u32(data, 0x8001CAB0) == 0x00042403, "sra a0 16")
    require(load_u32(data, 0x8001CB80) == 0x2D6B0001, "toggle sltiu")
    require(load_u32(data, 0x8001CB9C) == 0x01601021, "v0=t3")

    print("PASS: 0x77 14DA0 jal 1CAB0 n=4; 1CAB0 60w sra16 edge-cross")
    return 0


if __name__ == "__main__":
    sys.exit(main())
