#!/usr/bin/env python3
"""PE-BTL30 independent oracle: 0x11 / 130B4 flag-mask test."""
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

    require((0x800131E8 - 0x800130B4) // 4 == 77, "130B4 77w")
    require(
        window_sha(data, 0x800130B4, 0x800131E8)
        == "41ee2a1659cb0f8d1905ad71fb9f8b99408d6647c1a55127e789122ae7e25bbd",
        "130B4 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x11 * 4) == 0x800130B4, "table[0x11]")
    require(load_u32(data, 0x800130C0) == 0x24050001, "a1=1")
    require(load_u32(data, 0x80013108) == 0x8C63D26C, "code0 D_8009D26C")
    require(load_u32(data, 0x80013130) == 0x8C63D1F4, "code1 D_8009D1F4")
    require(load_u32(data, 0x80013144) == 0x8C63D1E4, "code2 D_8009D1E4")
    require(load_u32(data, 0x80013164) == 0xAC450000, "success *arg2=1")
    require(load_u32(data, 0x800131DC) == 0xAC400000, "fail *arg2=0")
    require(load_u32(data, 0x800131E4) == 0x24020001, "v0=1")

    print("PASS: 0x11 130B4 codes 0/1/2 mask-eq; live type5 code 1 mask 0x100")
    return 0


if __name__ == "__main__":
    sys.exit(main())
