#!/usr/bin/env python3
"""PE-BTL6 independent oracle: func_80087198 + 6CDA4 state 0 a0==0.

Pins SHA-1-exact EXE. 87198 is 5 words: D_8009D270=1, return 0.
6CDA4 state 0 table-fills first, then jal 87198 when a0==0, sb 7.
Live 6D60C F2=0x2F is 6CDA4(0, lb +0xE1=0x0D, …). Table a1=0x0D
halves 0x1F8/0x268 → remain 0x70. Does not import production C.
Does not claim 6E6D4/6914C success, mode 7, or 0x55 completion.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FN = 0x80087198
FN_END = 0x800871AC
WIN_SHA256 = (
    "cbcb73a8789f68035fa750d437ec12b385a5e999cd9d74973ce05f9bdb45b2de"
)
TWIN = 0x80087414
TWIN_END = 0x80087428
TWIN_SHA256 = (
    "acf05be3c1bbedd3bb0be1938bc0203ea8f6e5a4bedf4134b31d4443e8342ab3"
)
TABLE = 0x8009317C


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def load_u16(data: bytes, addr: int) -> int:
    return struct.unpack_from("<H", data, exe_off(addr))[0]


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

    require((FN_END - FN) // 4 == 5, "87198 5 words")
    require(window_sha(data, FN, FN_END) == WIN_SHA256, "87198 sha256")
    require(load_u32(data, FN) == 0x24020001, "li 1")
    require(load_u32(data, FN + 4) == 0x3C01800A, "lui $at,0x800a")
    require(load_u32(data, FN + 8) == 0xAC22D270, "sw v0, D_8009D270")
    require(load_u32(data, FN + 12) == 0x03E00008, "jr")
    require(load_u32(data, FN + 16) == 0x00001021, "v0=0")

    require((TWIN_END - TWIN) // 4 == 5, "87414 5 words")
    require(window_sha(data, TWIN, TWIN_END) == TWIN_SHA256, "87414 sha256")
    require(load_u32(data, TWIN) == 0x24020002, "87414 li 2")
    require(load_u32(data, TWIN + 8) == 0xAC22D270, "87414 sw D270")

    require(jal_target(load_u32(data, 0x8006CE84)) == FN, "state0 a0==0 jal 87198")
    require(load_u32(data, 0x8006CE7C) == 0x16800005, "bne s4,0 skip 87198")
    require(jal_target(load_u32(data, 0x8006CE9C)) == TWIN, "a0==3 jal 87414")
    require(load_u32(data, 0x8006CEC0) == 0xA24200F0, "state0 sb F0")
    require(load_u32(data, 0x8006CEAC) == 0x24020007, "state0 li 7")

    require(load_u32(data, 0x8006D860) == 0x00002021, "0x2F a0=0")
    require(load_u32(data, 0x8006D86C) == 0x822500E1, "0x2F lb +0xE1")
    require(jal_target(load_u32(data, 0x8006D878)) == 0x8006CDA4, "0x2F jal 6CDA4")
    require(load_u32(data, 0x8006A784) == 0x2402000D, "6A674 li 0x0D")
    require(load_u32(data, 0x8006A788) == 0xA0C200E1, "6A674 sb +0xE1")

    require(load_u32(data, TABLE) == 0x000008B0, "table word0")
    half0 = load_u16(data, TABLE + 4 + 0x1A)
    half6 = load_u16(data, TABLE + 0x1A + 6)
    require(half0 == 0x01F8, "a1=0x0D half0")
    require(half6 == 0x0268, "a1=0x0D half6")
    require(half6 - half0 == 0x70, "remain 0x70")

    require(load_u32(data, 0x800871B0) == 0x8C42D270, "871AC lw D270")
    require(load_u32(data, 0x800871D8) == 0x30420001, "871AC andi 1")

    print(
        "PASS: 87198 5w sha256 D_8009D270=1 ret0; twin 87414=2; "
        "6CDA4 state0 a0==0 jal after table; 0x2F a1=+0xE1=0x0D "
        "remain 0x70; no 6E6D4/6914C/0x55 claim"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
