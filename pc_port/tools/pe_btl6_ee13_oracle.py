#!/usr/bin/env python3
"""PE-BTL6 independent oracle: EE=13 prefix and live callees.

Checks SHA-1-exact EXE windows without importing production C.
Live bit1 path is EE 0→11→12→13. EE=13 does not jal 6CC68.
Clear andi 0xFC is after jal 3D050 / 6698C / 3D834 only.
Does not claim 0x55 completion, CD done, or those three callees.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FN_6C5BC_END = 0x8006CC68
EE13 = 0x8006C9F8
JAL_3D050 = 0x8006CB98
JAL_6698C = 0x8006CBF4
JAL_3D834 = 0x8006CC18
FN_6CC68 = 0x8006CC68
FN_6CC68_END = 0x8006CDA4
FN_3D050 = 0x8003D050
FN_3D050_END = 0x8003D834
FN_6698C = 0x8006698C
FN_6698C_END = 0x80066CE8
FN_3D834 = 0x8003D834
FN_3D834_END = 0x8003D94C
FN_3D834_THROUGH_3DFD8 = 0x8003DFD8
SITES_6CC68 = (
    0x8006C660, 0x8006C6B0, 0x8006C704,
    0x8006C7F4, 0x8006C83C, 0x8006C984,
)
EE13_SHA256 = (
    "2f4039f7c586b0379f31a0acf6d0d98a34ab5bc0a7c81d888e5a18dca94ae242"
)
WIN_6CC68_SHA256 = (
    "262dcfc6644b12a4e4398b45d04569040cedbc64b96a05de8dad652bf7fba74d"
)
WIN_3D050_SHA256 = (
    "50b5ff7516a04dd203ee5fdba30510547f4e47bb6e2b8988b8bde73bec00809a"
)
WIN_6698C_SHA256 = (
    "3c24408d3f33c94fc1f554c15ec0df6a06fb98091c0be666844bf22095e434e7"
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
    return hashlib.sha256(data[exe_off(start):exe_off(end)]).hexdigest()


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")

    require(load_u32(data, EE13) == 0x8E920158, "EE13 lw +0x158")
    require(load_u32(data, 0x8006CA48) == 0xAC6201C0, "EE13 sw +0x1C0")
    require(load_u32(data, 0x8006CA64) == 0xA2800010, "EE13 sb +0x10 zero")
    require(load_u32(data, 0x8006CA70) == 0xAC600134, "EE13 sw +0x134 zero")
    require(load_u32(data, 0x8006CB00) == 0x8C42D254, "EE13 lw D254")
    require(load_u32(data, 0x8006CB08) == 0x14400006, "EE13 bne D254")
    require(load_u32(data, 0x8006CB28) == 0x8C42D1A0, "EE13 lw D1A0")
    require(load_u32(data, 0x8006CB30) == 0x30420002, "EE13 andi D1A0 2")
    require(load_u32(data, 0x8006CB68) == 0x26840014, "EE13 a0=overlay+0x14")
    require(load_u32(data, 0x8006CB6C) == 0x240702C0, "EE13 a3=704")
    require(jal_target(load_u32(data, JAL_3D050)) == FN_3D050, "jal 3D050")
    require(jal_target(load_u32(data, JAL_6698C)) == FN_6698C, "jal 6698C")
    require(jal_target(load_u32(data, JAL_3D834)) == FN_3D834, "jal 3D834")
    require(load_u32(data, 0x8006CC20) == 0x9283000E, "clear lbu +0xE")
    require(load_u32(data, 0x8006CC2C) == 0x306300FC, "andi 0xFC after 3D834")
    require(load_u32(data, 0x8006CC34) == 0xA283000E, "sb cleared +0xE")

    ee13_jals = []
    for va in range(EE13, FN_6C5BC_END, 4):
        word = load_u32(data, va)
        if word >> 26 == 3:
            ee13_jals.append(jal_target(word))
    require(ee13_jals == [FN_3D050, FN_6698C, FN_3D834],
            f"EE13 jals {ee13_jals}")
    require(FN_6CC68 not in ee13_jals, "EE13 jal 6CC68")

    require((FN_6CC68_END - FN_6CC68) // 4 == 79, "6CC68 79 words")
    require(load_u32(data, FN_6CC68) == 0x27BDFFE0, "6CC68 prologue")
    require((FN_3D050_END - FN_3D050) // 4 == 505, "3D050 505 words")
    require(load_u32(data, FN_3D050) == 0x27BDFFC8, "3D050 addiu sp")
    require(load_u32(data, 0x8003D058) == 0x00808021, "3D050 s0=a0")
    require(load_u32(data, 0x8003D078) == 0xAE050000, "3D050 sw a1,0(s0)")
    require(load_u32(data, 0x8003D07C) == 0x2585001C, "3D050 a1=obj+0x1C")
    require(load_u32(data, 0x8003D080) == 0xAE050004, "3D050 sw +4")
    require(load_u32(data, 0x8003D09C) == 0xAE050008, "3D050 sw +8")
    require(load_u32(data, 0x8003D0B0) == 0xAE05000C, "3D050 sw +0xC")
    require(load_u32(data, 0x8003D0C0) == 0xAE110054, "3D050 sw a2,+0x54")
    require(load_u32(data, 0x8003D0CC) == 0xA61800BA, "3D050 sh stack,+0xBA")
    require(load_u32(data, 0x8003D0D0) == 0xAE050010, "3D050 sw +0x10")
    require((FN_6698C_END - FN_6698C) // 4 == 215, "6698C 215-word window")
    require((FN_3D834_END - FN_3D834) // 4 == 70, "3D834 70-word leaf")
    require((FN_3D834_THROUGH_3DFD8 - FN_3D834) // 4 == 489,
            "3D834-through-3DFD8 489-word span")
    require(load_u32(data, 0x8003D944) == 0x03E00008, "3D834 jr")
    require(load_u32(data, 0x8003D85C) == 0x12400013, "3D834 beq a1==0")
    require(window_sha(data, EE13, FN_6C5BC_END) == EE13_SHA256, "EE13 sha")
    require(window_sha(data, FN_6CC68, FN_6CC68_END) == WIN_6CC68_SHA256,
            "6CC68 sha")
    require(window_sha(data, FN_3D050, FN_3D050_END) == WIN_3D050_SHA256,
            "3D050 sha")
    require(window_sha(data, FN_6698C, FN_6698C_END) == WIN_6698C_SHA256,
            "6698C sha")

    jal_6cc68 = 0x0C01B31A
    sites = []
    for offset in range(0, len(data) - 4, 4):
        if struct.unpack_from("<I", data, offset)[0] == jal_6cc68:
            sites.append(0x80010000 + offset - 0x800)
    require(tuple(sites) == SITES_6CC68, f"6CC68 sites {sites}")
    for site in sites:
        require(site < EE13, f"6CC68 site in EE13 {site:#x}")

    print(
        "PASS: EE13 lw +0x158; walk +0x1C0; D254/D1A0 gates; "
        "jals 3D050/6698C/3D834; andi 0xFC after 3D834; "
        "6CC68 79w six sites all before EE13"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
