#!/usr/bin/env python3
"""PE-BTL6 independent oracle: func_80086464 + func_80086C1C.

Pins SHA-1-exact EXE. 86464 is 13 words: D_800BCD80=0x10,
D_800BCD84=a0, jal 8CBA8. 86C1C is 16 words: CD80=0xC0,
CD84=a1&0x7F, CD90=a0, jal 8CBA8. 8CBA8 cmd 0x10 jals 85084
(*CD84); v0!=0 → s1=-1. 85084 is 5 words: *buf+0xB0BEB4BF.
Does not import production C. Does not claim stream-complete,
6914C success, mode 7, or 0x55 completion.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FN_86464 = 0x80086464
END_86464 = 0x80086498
SHA_86464 = (
    "aa81a73bf6e074eb93d78c647d03974c31f27fd062c6d6b234bb34aa40fca782"
)
FN_86C1C = 0x80086C1C
END_86C1C = 0x80086C5C
SHA_86C1C = (
    "e1bd632dc75d54e70eee5d192efb6ef2de1d932d0657ce3d4a0db134de4fd13d"
)
FN_85084 = 0x80085084
END_85084 = 0x80085098
SHA_85084 = (
    "cec5712d8df2507debff33089467fe4fb954f99ddef03be92a264f1ed876cbfa"
)


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

    require((END_86464 - FN_86464) // 4 == 13, "86464 13 words")
    require(window_sha(data, FN_86464, END_86464) == SHA_86464, "86464 sha256")
    require(load_u32(data, FN_86464 + 4) == 0x24020010, "86464 li 0x10")
    require(load_u32(data, FN_86464 + 0x10) == 0xAC22CD80, "86464 sw CD80")
    require(load_u32(data, FN_86464 + 0x18) == 0xAC24CD84, "86464 sw CD84")
    require(jal_target(load_u32(data, FN_86464 + 0x1C)) == 0x8008CBA8, "86464 jal 8CBA8")

    require((END_86C1C - FN_86C1C) // 4 == 16, "86C1C 16 words")
    require(window_sha(data, FN_86C1C, END_86C1C) == SHA_86C1C, "86C1C sha256")
    require(load_u32(data, FN_86C1C + 4) == 0x240200C0, "86C1C li 0xC0")
    require(load_u32(data, FN_86C1C + 8) == 0x30A5007F, "86C1C andi 0x7F")
    require(load_u32(data, FN_86C1C + 0x14) == 0xAC22CD80, "86C1C sw CD80")
    require(load_u32(data, FN_86C1C + 0x1C) == 0xAC25CD84, "86C1C sw CD84")
    require(load_u32(data, FN_86C1C + 0x24) == 0xAC24CD90, "86C1C sw CD90")
    require(jal_target(load_u32(data, FN_86C1C + 0x28)) == 0x8008CBA8, "86C1C jal 8CBA8")

    require((END_85084 - FN_85084) // 4 == 5, "85084 5 words")
    require(window_sha(data, FN_85084, END_85084) == SHA_85084, "85084 sha256")
    require(load_u32(data, FN_85084) == 0x3C03B0BE, "85084 lui B0BE")
    require(load_u32(data, FN_85084 + 4) == 0x8C820000, "85084 lw 0(a0)")
    require(load_u32(data, FN_85084 + 8) == 0x3463B4BF, "85084 ori B4BF")
    require(load_u32(data, FN_85084 + 12) == 0x03E00008, "85084 jr")
    require(load_u32(data, FN_85084 + 16) == 0x00431021, "85084 addu")

    require(load_u32(data, 0x8008CBF4) == 0x24020010, "8CBA8 li 0x10")
    require(load_u32(data, 0x8008CBF8) == 0x1202001B, "8CBA8 beq cmd 0x10")
    require(jal_target(load_u32(data, 0x8008CC78)) == FN_85084, "cmd0x10 jal 85084")
    require(load_u32(data, 0x8008CC6C) == 0x8E10CD84, "cmd0x10 lw CD84")
    require(load_u32(data, 0x8008CC80) == 0x144000B1, "cmd0x10 bne v0 fail")
    require(load_u32(data, 0x8008CC84) == 0x2411FFFF, "cmd0x10 s1=-1")
    require(load_u32(data, 0x8008CF4C) == 0xAC20D268, "8CBA8 sw D268=0")

    require(load_u32(data, 0x8006D898) == 0x8E240124, "0x30 lw +0x124")
    require(jal_target(load_u32(data, 0x8006D89C)) == FN_86464, "0x30 jal 86464")
    require(jal_target(load_u32(data, 0x8006D8A8)) == FN_86C1C, "0x30 jal 86C1C")
    require(load_u32(data, 0x8006D8AC) == 0x2405007F, "0x30 a1=0x7F")
    require(load_u32(data, 0x8006D8B8) == 0xA22000F2, "0x30 sb F2=0")

    print(
        "PASS: 86464 13w CD80=0x10 CD84=a0 jal 8CBA8; "
        "86C1C 16w CD80=0xC0 CD84=a1&7F CD90=a0; "
        "cmd 0x10 jal 85084 fail s1=-1; 0x30 both then sb F2=0; "
        "no stream-complete/6914C/0x55 claim"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
