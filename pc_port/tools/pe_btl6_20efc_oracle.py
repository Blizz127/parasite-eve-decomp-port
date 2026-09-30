#!/usr/bin/env python3
"""PE-BTL6 independent oracle: func_80020EFC + 29810 jal / 71A64.

Pins SHA-1-exact EXE. 20EFC is 7 words (5 gp-relative sb 0 + jr/nop):
D_8009CE3C, D_8009D1D4, D_8009D1DC, D_8009D2D8, D_8009D1F0.
Exactly two jal sites: 29388 @0x800293DC and 29810 @0x8002984C
(delay addu s0,a0,zero — 20EFC is void(void); s0 keeps 0x3A
lbu(*overlay)). 71A64 is 3 words: li t2,0xA0 / jr t2 / li t9,0x30
(BIOS A(0x30) std_out_puts). 29810 a0 = lw D_8009D250. Does not
import production C. Does not invent puts output, mode 7, overlay
jalr, or 0x55 completion.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FN_20EFC = 0x80020EFC
END_20EFC = 0x80020F18
SHA_20EFC = (
    "9136c11e9e651fd71d91c8cb6658fe831c83459c61367e44119c7db508a21e7b"
)
FN_71A64 = 0x80071A64
END_71A64 = 0x80071A70
SHA_71A64 = (
    "cc10814f3017ef65c652e0e81949c196ba54adb13691f218d354e144d8790747"
)
FN_29810 = 0x80029810
JAL_20EFC_29810 = 0x8002984C
JAL_20EFC_29388 = 0x800293DC
JAL_71A64_29810 = 0x80029908
GP = 0x8009CD70

WORDS_20EFC = [
    0xA38000CC,  # sb 0, 0xCC(gp)  D_8009CE3C
    0xA3800464,  # sb 0, 0x464(gp) D_8009D1D4
    0xA380046C,  # sb 0, 0x46C(gp) D_8009D1DC
    0xA3800568,  # sb 0, 0x568(gp) D_8009D2D8
    0xA3800480,  # sb 0, 0x480(gp) D_8009D1F0
    0x03E00008,
    0x00000000,
]

WORDS_71A64 = [
    0x240A00A0,  # li t2, 0xA0
    0x01400008,  # jr t2
    0x24090030,  # li t9, 0x30  A(0x30)
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


def jal_enc(target: int) -> int:
    return 0x0C000000 | ((target & 0x0FFFFFFF) >> 2)


def window_sha(data: bytes, start: int, end: int) -> str:
    return hashlib.sha256(data[exe_off(start) : exe_off(end)]).hexdigest()


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")

    require((END_20EFC - FN_20EFC) // 4 == 7, "20EFC 7 words")
    require(window_sha(data, FN_20EFC, END_20EFC) == SHA_20EFC, "20EFC sha")
    for i, want in enumerate(WORDS_20EFC):
        got = load_u32(data, FN_20EFC + i * 4)
        require(got == want, f"20EFC word {i} {got:#010x}")
    require(GP + 0xCC == 0x8009CE3C, "CE3C")
    require(GP + 0x464 == 0x8009D1D4, "D1D4")
    require(GP + 0x46C == 0x8009D1DC, "D1DC")
    require(GP + 0x568 == 0x8009D2D8, "D2D8")
    require(GP + 0x480 == 0x8009D1F0, "D1F0")

    enc = jal_enc(FN_20EFC)
    hits = [
        0x80010000 + (i - 0x800)
        for i in range(0, len(data) - 4, 4)
        if struct.unpack_from("<I", data, i)[0] == enc
    ]
    require(hits == [JAL_20EFC_29388, JAL_20EFC_29810], f"20EFC sites {hits}")
    require(jal_target(load_u32(data, JAL_20EFC_29810)) == FN_20EFC, "29810 jal")
    require(load_u32(data, JAL_20EFC_29810 + 4) == 0x00808021, "delay s0=a0")

    require((END_71A64 - FN_71A64) // 4 == 3, "71A64 3 words")
    require(window_sha(data, FN_71A64, END_71A64) == SHA_71A64, "71A64 sha")
    for i, want in enumerate(WORDS_71A64):
        got = load_u32(data, FN_71A64 + i * 4)
        require(got == want, f"71A64 word {i} {got:#010x}")
    require(jal_target(load_u32(data, JAL_71A64_29810)) == FN_71A64, "29810 jal 71A64")
    require(load_u32(data, JAL_71A64_29810 - 8) == 0x3C04800A, "lui a0 0x800A")
    require(load_u32(data, JAL_71A64_29810 - 4) == 0x8C84D250, "lw D_8009D250")

    print(
        "PASS: 20EFC 5-byte clear 7w sha 9136c11e…1e7b; 29810 jal "
        "void(void) s0=a0; 71A64 A(0x30) a0=D_8009D250; no mode7/0x55"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
