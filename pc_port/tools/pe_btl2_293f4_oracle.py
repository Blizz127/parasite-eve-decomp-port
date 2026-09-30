#!/usr/bin/env python3
"""PE-BTL2 independent oracle: func_800293F4 HP named cut.

Verifies the SHA-1-exact EXE window that clamps record+0x0C to
record+0x1C and copies +0x0C into +0x0E, plus the blez reader and
default-record halfwords. Does not import production C.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC = 0x800293F4
FUNC_END = 0x80029448  # exclusive; addiu $v0, 1 (a0==1 arm)
GP = 0x8009CD70
RECORD_P = GP + 0x508  # D_8009D278
GP_460 = GP + 0x460  # D_8009D1D0
DEFAULT = 0x80010928
BLEZ_LH = 0x80029350
BLEZ = 0x80029358
JAL_FROM_29810 = 0x80029910
JAL_DELAY = 0x80029914
TAG4 = 0x80030098  # func_8002FF78 sh +0x0C
TAG5 = 0x800300A0  # func_8002FF78 sh +0x0E
JT_2FE78 = 0x80010AC8
SEL4 = 0x8002FEE0
SEL5 = 0x8002FF70  # nop / default return

WORDS = [
    0x8F850508,  # lw    $a1, 0x508($gp)
    0x27BDFFE0,  # addiu $sp, $sp, -0x20
    0xAFBF0018,  # sw    $ra, 0x18($sp)
    0x84A2001C,  # lh    $v0, 0x1C($a1)
    0x84A3000C,  # lh    $v1, 0x0C($a1)
    0x00403021,  # move  $a2, $v0
    0x0043102A,  # slt   $v0, $v0, $v1
    0x10400002,  # beqz  $v0, 0x8002941C
    0x00000000,  # nop
    0xA4A6000C,  # sh    $a2, 0x0C($a1)
    0x8F830508,  # lw    $v1, 0x508($gp)
    0x24020004,  # addiu $v0, $zero, 4
    0xA0620012,  # sb    $v0, 0x12($v1)
    0x8F850508,  # lw    $a1, 0x508($gp)
    0x00000000,  # nop
    0x94A2000C,  # lhu   $v0, 0x0C($a1)
    0x308300FF,  # andi  $v1, $a0, 0xFF
    0xAF800460,  # sw    $zero, 0x460($gp)
    0xA4A00010,  # sh    $zero, 0x10($a1)
    0xACA00034,  # sw    $zero, 0x34($a1)
    0xA4A2000E,  # sh    $v0, 0x0E($a1)
]


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
    return ((word & 0x3FFFFFF) << 2) | 0x80000000


def load_exe(path: pathlib.Path) -> bytes:
    data = path.read_bytes()
    digest = hashlib.sha1(data).hexdigest()
    require(digest == SHA1, f"exe sha1 {digest}")
    return data


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = load_exe(exe)
    require((FUNC_END - FUNC) // 4 == 21, "21 words")
    require(len(WORDS) == 21, "word table")
    for i, want in enumerate(WORDS):
        got = load_u32(data, FUNC + i * 4)
        require(got == want, f"word {i} @{FUNC + i * 4:#x} {got:#010x}")
    require(WORDS[0] == 0x8F850508, "lw gp+0x508")
    require((WORDS[0] & 0xFFFF) == 0x508, "record ptr gp+0x508")
    require(GP + 0x508 == RECORD_P, "D_8009D278")
    require((WORDS[3] & 0xFFFF) == 0x1C, "lh +0x1C")
    require((WORDS[3] >> 26) == 0x21, "lh")
    require((WORDS[4] & 0xFFFF) == 0x0C, "lh +0x0C")
    require((WORDS[6] >> 26) == 0, "slt")
    require((WORDS[9] >> 26) == 0x29, "sh clamp")
    require((WORDS[9] & 0xFFFF) == 0x0C, "sh +0x0C")
    require(WORDS[11] == 0x24020004, "li 4")
    require((WORDS[12] >> 26) == 0x28, "sb")
    require((WORDS[12] & 0xFFFF) == 0x12, "sb +0x12")
    require((WORDS[15] >> 26) == 0x25, "lhu +0x0C")
    require((WORDS[17] >> 26) == 0x2B, "sw gp+0x460")
    require((WORDS[17] & 0xFFFF) == 0x460, "gp+0x460")
    require(GP + 0x460 == GP_460, "D_8009D1D0")
    require((WORDS[18] & 0xFFFF) == 0x10, "sh +0x10 zero")
    require((WORDS[19] & 0xFFFF) == 0x34, "sw +0x34 zero")
    require((WORDS[20] >> 26) == 0x29, "sh copy")
    require((WORDS[20] & 0xFFFF) == 0x0E, "sh +0x0E")
    require(load_u32(data, FUNC_END) == 0x24020001, "exclusive addiu 1")
    require(load_u32(data, BLEZ_LH) == 0x8442000C, "reader lh +0x0C")
    require(load_u32(data, BLEZ) == 0x18400003, "blez on +0x0C")
    require(load_u16(data, DEFAULT + 0x0C) == 0x002D, "default +0x0C=45")
    require(load_u16(data, DEFAULT + 0x0E) == 0x002D, "default +0x0E=45")
    require(load_u16(data, DEFAULT + 0x1C) == 0x002D, "default +0x1C=45")
    require(jal_target(load_u32(data, JAL_FROM_29810)) == FUNC, "29810 jal 293F4")
    require(load_u32(data, JAL_DELAY) == 0x00002021, "29810 a0=0 delay")
    require(load_u32(data, TAG4) == 0xA4C5000C, "0x5A tag4 sh +0x0C")
    require(load_u32(data, TAG5) == 0xA4C5000E, "0x5A tag5 sh +0x0E")
    require(load_u32(data, JT_2FE78 + 4 * 4) == SEL4, "0x59 sel4 +0x0C")
    require(load_u32(data, JT_2FE78 + 5 * 4) == SEL5, "0x59 sel5 nop")
    print("PASS: func_800293F4_hp_cut 21/21 words + HP +0x0C/+0x0E/+0x1C")
    return 0


if __name__ == "__main__":
    sys.exit(main())
