#!/usr/bin/env python3
"""PE-BTL6 independent oracle: func_8006D078 +0xF3 state machine.

Pins SHA-1-exact EXE. The leaf is 117 words (0x8006D078..0x8006D24C),
not the 357-word span to 6D60C. Does not import production C.
Does not claim 6CDA4/6D60C/6914C success, mode 7, or 0x55 completion.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FN = 0x8006D078
FN_END = 0x8006D24C
JT = 0x80011458
DEFAULT = 0x8006D22C
FN_6CDA4 = 0x8006CDA4
FN_6CDA4_END = 0x8006D078
WIN_SHA256 = (
    "7cf2bb6168b5ccf24db7639f024d093d1925dc90d63cba77ac861c4ffde5b1dd"
)
WIN_6CDA4_SHA256 = (
    "c94c08adeed015909c745201d6c1f142f9966204b52470fc0fc0778fce3c8f79"
)

STATE0_WORDS = [
    0x24020028,
    0xAF80005C,
    0x0801B42E,
    0xA20200F3,
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


def window_sha(data: bytes, start: int, end: int) -> str:
    return hashlib.sha256(data[exe_off(start):exe_off(end)]).hexdigest()


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")

    require((FN_END - FN) // 4 == 117, "6D078 117 words")
    require(window_sha(data, FN, FN_END) == WIN_SHA256, "6D078 sha256")
    require(load_u32(data, FN) == 0x3C03800B, "lui D_800B0E64")
    require(load_u32(data, FN + 4) == 0x8C630E64, "lw D_800B0E64")
    require(load_u32(data, FN + 0x10) == 0x3C10800B, "s0 overlay")
    require(load_u32(data, FN + 0x14) == 0x26100CD8, "s0 +0xCD8")
    require(load_u32(data, FN + 0x40) == 0x920300F3, "lbu +0xF3")
    require(load_u32(data, FN + 0x48) == 0x2C62002C, "sltiu 0x2C")
    require(load_u32(data, FN + 0x5C) == 0x8C221458, "lw JT 0x80011458")
    require(load_u32(data, DEFAULT) == 0x00001021, "default v0=0")
    require(load_u32(data, 0x8006D244) == 0x03E00008, "jr")

    require(load_u32(data, JT + 0) == 0x8006D0E4, "JT[0]")
    require(load_u32(data, JT + 0x28 * 4) == 0x8006D0F4, "JT[0x28]")
    require(load_u32(data, JT + 0x29 * 4) == 0x8006D140, "JT[0x29]")
    require(load_u32(data, JT + 0x2A * 4) == 0x8006D178, "JT[0x2A]")
    require(load_u32(data, JT + 0x2B * 4) == 0x8006D1DC, "JT[0x2B]")
    for i in range(1, 0x28):
        require(load_u32(data, JT + i * 4) == DEFAULT, f"JT[{i:#x}] default")

    for i, want in enumerate(STATE0_WORDS):
        got = load_u32(data, 0x8006D0E4 + i * 4)
        require(got == want, f"state0 @{0x8006D0E4 + i * 4:#x}")

    require(jal_target(load_u32(data, 0x8006D10C)) == FN_6CDA4, "0x28 jal 6CDA4")
    require(load_u32(data, 0x8006D0F4) == 0x24040001, "0x28 a0=1")
    require(load_u32(data, 0x8006D0F8) == 0x24050001, "0x28 a1=1")
    require(load_u32(data, 0x8006D0FC) == 0x00003021, "0x28 a2=0")
    require(load_u32(data, 0x8006D100) == 0x8E070194, "0x28 lw dest +0x194")
    require(load_u32(data, 0x8006D104) == 0x24020021, "0x28 stack 0x21")
    require(load_u32(data, 0x8006D128) == 0x2C420002, "0x28 +0x10 sltiu 2")
    require(jal_target(load_u32(data, 0x8006D158)) == FN_6CDA4, "0x29 jal 6CDA4")
    require(jal_target(load_u32(data, 0x8006D200)) == FN_6CDA4, "0x2B jal 6CDA4")
    require(load_u32(data, 0x8006D178) == 0x8E420024, "0x2A lw +0x24")
    require(load_u32(data, 0x8006D180) == 0x00021582, "0x2A srl 22")
    require(((load_u32(data, 0x8006D180) >> 6) & 31) == 22,
            "packed sound count occupies bits 22..31")
    require(load_u32(data, 0x8006D184) == 0x0062102A, "0x2A slt index,count")
    require(load_u32(data, 0x8006D19C) == 0x30420010, "0x2A andi 0x10")
    require(load_u32(data, 0x8006D1A8) == 0x94820004, "0x2A lhu +4")
    require(load_u32(data, 0x8006D1C8) == 0x2402002B, "0x2A li 0x2B")
    require(load_u32(data, 0x8006D1D8) == 0xA20000F3, "exhaust sb F3=0")
    require(load_u32(data, 0x8006D1DC) == 0x24040003, "0x2B a0=3")
    require(load_u32(data, 0x8006CE9C) == 0x0C021D05, "state0 a0=3 jal 87414")
    require((0x80087428 - 0x80087414) // 4 == 5, "87414 5 words")
    require(load_u32(data, 0x80087414) == 0x24020002, "87414 v0=2")
    require(load_u32(data, 0x8008741C) == 0xAC22D270, "87414 sw D_8009D270")
    require(load_u32(data, 0x80087420) == 0x03E00008, "87414 jr")
    require(load_u32(data, 0x80087424) == 0x00001021, "87414 v0=0")
    require(
        window_sha(data, 0x80087414, 0x80087428)
        == "acf05be3c1bbedd3bb0be1938bc0203ea8f6e5a4bedf4134b31d4443e8342ab3",
        "87414 sha256",
    )

    for va in range(FN, FN_END, 4):
        word = load_u32(data, va)
        if (word >> 26) in (0x28, 0x29, 0x2B) and (word & 0xFFFF) == 0x000E:
            raise SystemExit(f"FAIL: 6D078 stores +0xE @{va:#x}")

    jal = 0x0C000000 | ((FN & 0x0FFFFFFF) >> 2)
    hits = [
        0x80010000 + offset - 0x800
        for offset in range(0, len(data) - 4, 4)
        if struct.unpack_from("<I", data, offset)[0] == jal
    ]
    require(hits == [0x8006D788], f"6D078 jals {hits}")

    require((FN_6CDA4_END - FN_6CDA4) // 4 == 181, "6CDA4 181 words")
    require(
        window_sha(data, FN_6CDA4, FN_6CDA4_END) == WIN_6CDA4_SHA256,
        "6CDA4 sha256",
    )
    require(load_u32(data, FN_6CDA4 + 0x6C) == 0x924300F0, "6CDA4 lbu +0xF0")
    for va in range(FN_6CDA4, FN_6CDA4_END, 4):
        word = load_u32(data, va)
        if (word >> 26) in (0x28, 0x29, 0x2B) and (word & 0xFFFF) == 0x000E:
            raise SystemExit(f"FAIL: 6CDA4 stores +0xE @{va:#x}")

    print(
        "PASS: 6D078 117w sha256; +0xF3 JT 0/28/29/2A/2B; state0 sb 0x28; "
        "0x2A srl22/andi10/lhu+4; 0x2B a0=3; 87414 D270=2 ret0; "
        "0x28/29/2B jal 6CDA4; no +0xE; sole jal from 6D60C 0x2E"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
