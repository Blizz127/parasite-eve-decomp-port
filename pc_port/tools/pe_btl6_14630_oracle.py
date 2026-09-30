#!/usr/bin/env python3
"""PE-BTL7 independent oracle: 144FC 0x3B at 0x80014630.

Pins SHA-1-exact EXE. 0x80014630 is JT[0x3B] inside func_800144FC
(102 words), not a separate leaf. 12 words to the shared park.

$s0 = D_800B0CD8 (lui/addiu at 0x8001450C). 0x3B and state 0
lw/sw 0($s0) = overlay word 0 (8E02/AE02). 0x3A uses 0($s1)
(8E22) for lbu(*binder) — a different object.

Busy (+0xE&3): j 0x80014660 v0=0, rewind gp+0x90 by 12.
Clear: overlay[0] &= ~0x00800000, sb F4=0, j 0x8001467C v0=1.
Zero jal / jalr in 0x80014630..0x80014694. Not 6914C.
Not 0x800E086C. Live +0xE bit1 keeps v0=0; 3F074 must poll
6C5BC to v0=0 (EE=0 → jal 6CC68) before 0x3B can return 1.
Does not import production C. Does not claim 0x55 complete.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC = 0x800144FC
FUNC_END = 0x80014694
STATE0 = 0x80014544
STATE3A = 0x800145F8
STATE3B = 0x80014630
PARK = 0x80014660
SUCCESS = 0x8001467C
JT = 0x800100A0
WIN_SHA = "3e531fb720f55132e195ee2459b482bed90be5f2e07a8bb3d74755d4c1ea5bab"


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def j_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")

    require((FUNC_END - FUNC) // 4 == 102, "144FC 102 words")
    require(load_u32(data, JT + 0x3B * 4) == STATE3B, "JT 0x3B")
    require(load_u32(data, FUNC + 8) == 0x00808821, "s1=a0")
    require(load_u32(data, FUNC + 0x10) == 0x3C10800B, "s0 lui 0x800B")
    require(load_u32(data, FUNC + 0x14) == 0x26100CD8, "s0 = overlay")

    window = data[exe_off(STATE3B) : exe_off(PARK)]
    require(len(window) == 12 * 4, "12 words to park")
    require(hashlib.sha256(window).hexdigest() == WIN_SHA, "14630..14660 sha")

    require(load_u32(data, STATE3B) == 0x9202000E, "0x3B lbu +0xE($s0)")
    require(load_u32(data, STATE3B + 8) == 0x30420003, "0x3B andi 3")
    require(load_u32(data, STATE3B + 0xC) == 0x14400008, "0x3B bnez +8")
    require(STATE3B + 0xC + 4 + 8 * 4 == PARK, "busy lands at park")
    require(load_u32(data, STATE3B + 0x10) == 0x3C03FF7F, "lui 0xFF7F")
    require(load_u32(data, STATE3B + 0x14) == 0x8E020000, "lw 0($s0)")
    require(((0x8E020000 >> 21) & 0x1F) == 16, "lw rs=s0")
    require(load_u32(data, STATE3B + 0x18) == 0x3463FFFF, "ori 0xFFFF")
    require(load_u32(data, STATE3B + 0x1C) == 0xA20000F4, "sb F4=0")
    require(load_u32(data, STATE3B + 0x20) == 0x00431024, "and ~0x00800000")
    require(load_u32(data, STATE3B + 0x24) == 0xAE020000, "sw 0($s0)")
    require(((0xAE020000 >> 21) & 0x1F) == 16, "sw rs=s0")
    require(j_target(load_u32(data, STATE3B + 0x28)) == SUCCESS, "j success")
    require(load_u32(data, STATE3B + 0x2C) == 0x24020001, "li v0,1")

    require(load_u32(data, PARK) == 0x00001021, "park v0=0")
    require(load_u32(data, PARK + 4) == 0x8F830090, "lw gp+0x90")
    require(load_u32(data, PARK + 0xC) == 0x2463FFF4, "rewind -12")
    require(load_u32(data, SUCCESS) == 0x8FBF0018, "success is epilogue")

    require(load_u32(data, STATE0 + 0x18) == 0x8E020000, "state0 lw 0($s0)")
    require(load_u32(data, STATE0 + 0x1C) == 0x3C030080, "state0 lui 0x0080")
    require(load_u32(data, STATE0 + 0x28) == 0xAE020000, "state0 sw 0($s0)")
    require(load_u32(data, STATE3A + 0x18) == 0x8E220000, "0x3A lw 0($s1)")
    require(((0x8E220000 >> 21) & 0x1F) == 17, "0x3A rs=s1")

    for va in range(STATE3B, FUNC_END, 4):
        word = load_u32(data, va)
        require(word >> 26 != 3, f"jal at {va:#x}")
        require(not (word >> 26 == 0 and (word & 0x3F) == 9), f"jalr at {va:#x}")

    print(
        "PASS: 14630 12w sha 3e531fb7… overlay[0] &= ~0x00800000 v0=1; "
        "busy park v0=0; 8E02=$s0 not 8E22=$s1; 0 jal/jalr"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
