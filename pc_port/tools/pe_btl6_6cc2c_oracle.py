#!/usr/bin/env python3
"""PE-BTL6 independent oracle: 6C5BC andi 0xFC epilogue at 0x8006CC2C.

Pins SHA-1-exact EXE words: jal 3D834 is immediately before the
lbu/+0xEE/andi 0xFC/sb +0xE sequence. Does not import production C.
Does not claim 0x55 completion, 6914C success, or mode 7.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
JAL_3D834 = 0x8006CC18
FN_3D834 = 0x8003D834
CLEAR = 0x8006CC20
ANDI = 0x8006CC2C
SB_E = 0x8006CC34
JOIN = 0x8006CC3C


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def j_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")

    require(load_u32(data, JAL_3D834) >> 26 == 3, "CC18 is jal")
    require(jal_target(load_u32(data, JAL_3D834)) == FN_3D834, "jal 3D834")
    require(JAL_3D834 + 8 == CLEAR, "clear is delay+4 after jal")
    require(load_u32(data, 0x8006CC1C) == 0xAFA20010, "jal delay sw")
    require(load_u32(data, CLEAR) == 0x9283000E, "lbu +0xE")
    require(load_u32(data, 0x8006CC24) == 0x24020001, "li v0,1")
    require(load_u32(data, 0x8006CC28) == 0xA28000EE, "sb 0 +0xEE")
    require(load_u32(data, ANDI) == 0x306300FC, "andi 0xFC")
    require(load_u32(data, 0x8006CC30) >> 26 == 2, "j join")
    require(j_target(load_u32(data, 0x8006CC30)) == JOIN, "j 0x8006CC3C")
    require(load_u32(data, SB_E) == 0xA283000E, "delay sb +0xE")
    require(load_u32(data, 0x8006CC38) == 0x00001021, "v0=0 join skipped")

    for va in range(JAL_3D834 + 4, ANDI, 4):
        require(load_u32(data, va) >> 26 != 3, f"jal before andi @{va:#x}")

    print(
        "PASS: 6CC2C andi 0xFC immediately after jal 3D834; "
        "sb +0xEE=0; delay sb +0xE; v0=1"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
