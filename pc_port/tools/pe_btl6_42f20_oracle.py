#!/usr/bin/env python3
"""PE-BTL6 independent oracle: func_80042F20 + 144FC 0x38/0x39.

Pins SHA-1-exact EXE. 42F20 is 6 words: li 5 / sw gp+0x168,
li -1 / sw gp+0x174, jr+nop. 144FC 0x38 after 6D60C!=1 jals
42F20 when overlay bit 0x400000 is clear, then always sb 0x39.
0x39 jals 6914C(1). 6914C is 274 words; 0x34 jals 6E6A8, 0x35
jals 6E7E8. Does not import production C. Does not claim 6914C
success, mode 7, stream-complete, or 0x55 completion.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
JT = 0x800100A0
STATE38 = 0x8001459C
STATE39 = 0x800145DC
STATE3A = 0x800145F8
REDISPATCH = 0x80014518
PARK = 0x80014660
FN_42F20 = 0x80042F20
END_42F20 = 0x80042F38
SHA_42F20 = (
    "302158566c4097c4a50c3640932998257ac62504f2c6d96ee8529e1e1d96ed70"
)
FN_6914C = 0x8006914C
END_6914C = 0x80069594
SHA_6914C = (
    "cd016627ab58160bc8e04d9f7817b7e7413c22336087a38cf0a8de329a6ddc6b"
)
FN_6E6A8 = 0x8006E6A8
FN_6E7E8 = 0x8006E7E8
FN_6D60C = 0x8006D60C

FN_42F20_WORDS = [
    0x24020005,
    0xAF820168,
    0x2402FFFF,
    0xAF820174,
    0x03E00008,
    0x00000000,
]


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


def window_sha(data: bytes, start: int, end: int) -> str:
    return hashlib.sha256(data[exe_off(start) : exe_off(end)]).hexdigest()


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")

    require((END_42F20 - FN_42F20) // 4 == 6, "42F20 6 words")
    require(window_sha(data, FN_42F20, END_42F20) == SHA_42F20, "42F20 sha256")
    for i, want in enumerate(FN_42F20_WORDS):
        got = load_u32(data, FN_42F20 + i * 4)
        require(got == want, f"42F20 @{FN_42F20 + i * 4:#x}")
    require(load_u32(data, END_42F20) == 0xAF800168, "next leaf sw zero 168")

    jal = 0x0C000000 | ((FN_42F20 & 0x0FFFFFFF) >> 2)
    hits = [
        0x80010000 + offset - 0x800
        for offset in range(0, len(data) - 4, 4)
        if struct.unpack_from("<I", data, offset)[0] == jal
    ]
    require(hits == [0x800145C8, 0x8005D608], f"42F20 jals {hits}")

    require(load_u32(data, JT + 0x38 * 4) == STATE38, "JT[0x38]")
    require(load_u32(data, JT + 0x39 * 4) == STATE39, "JT[0x39]")
    require(load_u32(data, JT + 0x3A * 4) == STATE3A, "JT[0x3A]")
    require(STATE38 + 16 * 4 == STATE39, "0x38 then 0x39")
    require(STATE39 + 7 * 4 == STATE3A, "0x39 then 0x3A")

    require(jal_target(load_u32(data, STATE38)) == FN_6D60C, "0x38 jal 6D60C")
    require(load_u32(data, STATE38 + 4) == 0x24040001, "0x38 a0=1")
    require(load_u32(data, STATE38 + 8) == 0x24030001, "0x38 li 1")
    require(load_u32(data, STATE38 + 0xC) == 0x1043002D, "0x38 beq park")
    require(STATE38 + 0x10 + 0x2D * 4 == PARK, "0x38 park 14660")
    require(load_u32(data, STATE38 + 0x10) == 0x3C030040, "0x38 lui 0x40")
    require(jal_target(load_u32(data, STATE38 + 0x2C)) == FN_42F20, "0x38 jal 42F20")
    require(load_u32(data, STATE38 + 0x28) == 0x24020039, "0x38 li 0x39")
    require(j_target(load_u32(data, STATE38 + 0x38)) == REDISPATCH, "0x38 j 14518")
    require(load_u32(data, STATE38 + 0x3C) == 0xA20200F4, "0x38 delay sb +0xF4")

    require(jal_target(load_u32(data, STATE39)) == FN_6914C, "0x39 jal 6914C")
    require(load_u32(data, STATE39 + 4) == 0x24040001, "0x39 a0=1")
    require(load_u32(data, STATE39 + 8) == 0x24030001, "0x39 li 1")
    require(load_u32(data, STATE39 + 0xC) == 0x1043001D, "0x39 beq park")
    require(STATE39 + 0x10 + 0x1D * 4 == PARK, "0x39 park 14660")
    require(load_u32(data, STATE39 + 0x10) == 0x2402003A, "0x39 li 0x3A")
    require(j_target(load_u32(data, STATE39 + 0x14)) == REDISPATCH, "0x39 j 14518")
    require(load_u32(data, STATE39 + 0x18) == 0xA20200F4, "0x39 delay sb +0xF4")
    require(load_u32(data, PARK) == 0x00001021, "14660 v0=0")

    require((END_6914C - FN_6914C) // 4 == 274, "6914C 274 words")
    require(window_sha(data, FN_6914C, END_6914C) == SHA_6914C, "6914C sha256")
    require(load_u32(data, FN_6914C + 0x28) == 0x928300EF, "6914C lbu +0xEF")
    require(load_u32(data, 0x800691E0) == 0x8E820188, "state0 lw +0x188")
    require(load_u32(data, 0x80069210) == 0x2A02000B, "state0 slti 11")
    require(load_u32(data, 0x80069218) == 0x24630A0C, "state0 stride 0xA0C")
    require(load_u32(data, 0x80069238) == 0x24426E84, "state0 +0x6E84")
    require(load_u32(data, 0x8006926C) == 0x2463010C, "state0 stride 0x10C")
    require(load_u32(data, 0x800692A4) == 0x0040F809, "state0 jalr overlay")
    require(load_u32(data, 0x8006A9B4) == 0xAC220E60, "6A8D4 sw +0x188")
    require(load_u32(data, 0x8006A9C4) == 0xAC220E6C, "6A8D4 sw +0x194")
    require(load_u32(data, 0x8006A9B8) == 0x3C02801F, "6A8D4 lui 0x801F")
    require(load_u32(data, 0x8006A9BC) == 0x2442D800, "6A8D4 dest 0x801ED800")
    require(jal_target(load_u32(data, 0x80069414)) == FN_6E6A8, "0x34 jal 6E6A8")
    require(load_u32(data, 0x800693F8) == 0x8E850194, "0x34 lw +0x194")
    require(load_u32(data, 0x80069400) == 0x944230E2, "0x34 lhu 930E2")
    require(load_u32(data, 0x80069404) == 0x8E840100, "0x34 lw +0x100")
    require(load_u32(data, 0x8006940C) == 0x94C630E4, "0x34 lhu 930E4")
    require(jal_target(load_u32(data, 0x80069430)) == FN_6E7E8, "0x35 jal 6E7E8")
    require(load_u32(data, 0x8006944C) == 0x0801A55C, "0x34/-1 j return 1")
    require(load_u32(data, 0x80069450) == 0x24020001, "0x34 delay v0=1")

    print(
        "PASS: 42F20 6w gp+0x168=5 gp+0x174=-1; "
        "0x38 jal 6D60C then 42F20 sb 0x39; "
        "0x39 jal 6914C(1); 6914C 274w 0x34=6E6A8 0x35=6E7E8; "
        "no 6914C-success/mode7/0x55 claim"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
