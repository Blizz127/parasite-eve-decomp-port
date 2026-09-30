#!/usr/bin/env python3
"""PE-BTL19 independent oracle: VM opcode 0x40 sets D_8009D2E8 bit 0.

Pins SHA-1-exact EXE. 8 words. This is the live type-1 setter that
makes 35C84 skip 3999C. Does not import production C. Does not force
the bit in production.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FN = 0x80017D7C
END = 0x80017D9C
SHA = "ce839661081cb10287fab695111cb68082a1c27bb99b3ac418c4143cf95f9b09"


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
    require((END - FN) // 4 == 8, "17D7C 8 words")
    require(window_sha(data, FN, END) == SHA, "17D7C sha")
    require(load_u32(data, 0x800910A0 + 0x40 * 4) == FN, "table[0x40]")
    require(load_u32(data, 0x80017D80) == 0x8C42D2E8, "lw D2E8")
    require(load_u32(data, 0x80017D88) == 0x34420001, "ori 1")
    require(load_u32(data, 0x80017D90) == 0xAC22D2E8, "sw D2E8")
    require(load_u32(data, 0x80017D98) == 0x24020001, "v0=1")
    require(load_u32(data, 0x800910A0 + 0x3F * 4) == 0x80017D5C, "table[0x3F]")
    require((0x80017D7C - 0x80017D5C) // 4 == 8, "17D5C 8 words")
    require(
        window_sha(data, 0x80017D5C, 0x80017D7C)
        == "e8c3e4f826f98d8639376458d0f1992aedfa690ea2140081668c9f7893d95e05",
        "17D5C sha",
    )
    require(load_u32(data, 0x80017D64) == 0x2403FFFE, "17D5C li ~1")
    require(load_u32(data, 0x80017D70) == 0xAC22D2E8, "17D5C sw D2E8")
    print(
        "PASS: 17D7C 8w op 0x40 D_8009D2E8|=1; 17D5C 8w op 0x3F &= ~1; "
        "live type-1 0x40 is the 35C84/3999C skip setter"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
