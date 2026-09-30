#!/usr/bin/env python3
"""PE-BTL6 independent oracle: 144FC 0x3A at 0x800145F8.

Pins SHA-1-exact EXE. After 6914C v0=0, 0x39 sb 0x3A. 0x3A:
D_8009D1A0 |= 2, lw v0,0(s1); lbu a0,0(v0); jal 29810, sb F4=0x3B,
s1 is 144FC a0 (binder arg0). NYPD 0x55(2) mode 0 → lbu(2).
j 0x80014660 (v0=0 park). Does not re-enter 0x3B in the same
call. 29810 first jals 20EFC / 71A64 / 293F4(0). Does not
import production C. Does not claim 0x55 completion, mode 7,
or overlay jalr.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
JT = 0x800100A0
STATE39 = 0x800145DC
STATE3A = 0x800145F8
STATE3B = 0x80014630
PARK = 0x80014660
FN_29810 = 0x80029810
FN_293F4 = 0x800293F4
FN_20EFC = 0x80020EFC
FN_71A64 = 0x80071A64


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

    require(load_u32(data, JT + 0x3A * 4) == STATE3A, "JT 0x3A")
    require(load_u32(data, JT + 0x3B * 4) == STATE3B, "JT 0x3B")
    require(jal_target(load_u32(data, STATE39)) == 0x8006914C, "0x39 jal 6914C")
    require(load_u32(data, STATE39 + 0x10) == 0x2402003A, "0x39 li 0x3A")
    require(load_u32(data, STATE39 + 0x18) == 0xA20200F4, "0x39 sb F4")

    require(load_u32(data, STATE3A) == 0x3C02800A, "lui 0x800A")
    require(load_u32(data, STATE3A + 4) == 0x8C42D1A0, "lw D1A0")
    require(load_u32(data, STATE3A + 0xC) == 0x34420002, "ori 2")
    require(load_u32(data, STATE3A + 0x14) == 0xAC22D1A0, "sw D1A0")
    require(load_u32(data, STATE3A + 0x18) == 0x8E220000, "lw 0(s1)")
    require(load_u32(data, STATE3A + 0x20) == 0x90440000, "lbu 0(v0)")
    require(load_u32(data, 0x80014504) == 0x00808821, "s1=a0 binder")
    require(jal_target(load_u32(data, STATE3A + 0x24)) == FN_29810, "jal 29810")
    require(load_u32(data, STATE3A + 0x2C) == 0x2402003B, "li 0x3B")
    require(j_target(load_u32(data, STATE3A + 0x30)) == PARK, "j park")
    require(load_u32(data, STATE3A + 0x34) == 0xA20200F4, "sb F4=0x3B")
    require(load_u32(data, PARK) == 0x00001021, "park v0=0")

    require(jal_target(load_u32(data, FN_29810 + 0x3C)) == FN_20EFC, "29810 jal 20EFC")
    require(jal_target(load_u32(data, FN_29810 + 0xF8)) == FN_71A64, "29810 jal 71A64")
    require(jal_target(load_u32(data, 0x80029910)) == FN_293F4, "29810 jal 293F4")
    require(load_u32(data, 0x80029914) == 0x00002021, "293F4 a0=0")

    print(
        "PASS: 145F8 D1A0|=2 lw 0(s1) lbu 0(v0) jal 29810 sb 0x3B park; "
        "no 0x55/mode7/overlay claim"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
