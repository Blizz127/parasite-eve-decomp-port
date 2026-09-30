#!/usr/bin/env python3
"""PE-BTL6 independent oracle: 3D834 a1==0 callees.

Checks SHA-1-exact EXE windows without importing production C.
Does not claim 3A088/3B97C GTE walks, andi 0xFC, or 0x55 done.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"

FN_3D834 = 0x8003D834
FN_3D834_END = 0x8003D94C
FN_3A088 = 0x8003A088
FN_3A088_END = 0x8003A6A8
FN_3B97C = 0x8003B97C
FN_3B97C_END = 0x8003BCE0
FN_3BCE0 = 0x8003BCE0
FN_3BCE0_END = 0x8003C0B4
FN_3DBE4 = 0x8003DBE4
FN_3DD08 = 0x8003DD08

WIN_3A088 = "9b66ed6ce2fcd0d1f1da408436fe1f85c7ef3e07c64533dc28a9ca629230c3fe"
WIN_3B97C = "bce3a03949e472e16d16b609d3171dfee35db459891c235d761ed43eb426c500"
WIN_3BCE0 = "207d5d5ffa27e57ad16dfe5d9ab3881c47af8b10b8e1b86062bc692ff38fe6e6"
WIN_3D834 = "520529b07ee5f4234242a5911c06895083fbf2955c6c58a54ae5a7d8a317f7ad"


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

    require((FN_3D834_END - FN_3D834) // 4 == 70, "3D834 70 words")
    require(window_sha(data, FN_3D834, FN_3D834_END) == WIN_3D834, "3D834 sha")
    require(load_u32(data, 0x8003D85C) == 0x12400013, "3D834 beq a1==0")
    require(jal_target(load_u32(data, 0x8003D8AC)) == FN_3A088, "a1==0 jal 3A088")
    require(jal_target(load_u32(data, 0x8003D8CC)) == FN_3B97C, "a1==0 jal 3B97C")
    require(jal_target(load_u32(data, 0x8003D8E0)) == FN_3BCE0, "a1==0 jal 3BCE0")
    require(jal_target(load_u32(data, 0x8003D908)) == FN_3BCE0, "a1==0 jal 3BCE0 #2")

    require((FN_3A088_END - FN_3A088) // 4 == 392, "3A088 392 words")
    require(window_sha(data, FN_3A088, FN_3A088_END) == WIN_3A088, "3A088 sha")
    require(load_u32(data, 0x8003A0C0) == 0x86030028, "3A088 lh +0x28")
    require(load_u32(data, 0x8003A0C8) == 0x14620007, "3A088 bne mode 1")
    require(jal_target(load_u32(data, 0x8003A0D8)) == FN_3DBE4, "jal 3DBE4")
    require(load_u32(data, 0x8003A0E8) == 0x24020003, "3A088 li 3")
    require(load_u32(data, 0x8003A0EC) == 0x1462003C, "3A088 bne mode 3")
    require(jal_target(load_u32(data, 0x8003A1D0)) == FN_3DD08, "jal 3DD08")
    require(load_u32(data, 0x8003A1E0) == 0x14620059, "3A088 bne mode 4")
    require(load_u32(data, 0x8003A348) == 0xAE600000, "3A088 join sw scratch")
    require(load_u32(data, 0x8003A3A4) == 0x94420018, "3A088 lhu obj+0x18")
    require(load_u32(data, 0x8003A3AC) == 0x184000B5, "3A088 blez empty")
    require(load_u32(data, 0x8003A6A0) == 0x03E00008, "3A088 jr")
    a088_jals = []
    for va in range(FN_3A088, FN_3A088_END, 4):
        word = load_u32(data, va)
        if word >> 26 == 3:
            a088_jals.append(jal_target(word))
    require(a088_jals == [FN_3DBE4, FN_3DD08], f"3A088 jals {a088_jals}")

    require((FN_3B97C_END - FN_3B97C) // 4 == 217, "3B97C 217 words")
    require(window_sha(data, FN_3B97C, FN_3B97C_END) == WIN_3B97C, "3B97C sha")
    require(load_u32(data, 0x8003B9AC) == 0x8D620000, "3B97C lw dest+0")
    require(load_u32(data, 0x8003B9B4) == 0x104000C0, "3B97C beq dest+0")
    require(load_u32(data, 0x8003B9BC) == 0x856200BA, "3B97C lh +0xBA")
    require(load_u32(data, 0x8003B9C4) == 0x104000BC, "3B97C beq +0xBA")
    require(load_u32(data, 0x8003BA08) == 0x90420002, "3B97C lbu obj+2")
    require(load_u32(data, 0x8003BA10) == 0x184000A9, "3B97C blez obj+2")
    require(load_u32(data, 0x8003BCD8) == 0x03E00008, "3B97C jr")
    b97c_jals = []
    for va in range(FN_3B97C, FN_3B97C_END, 4):
        word = load_u32(data, va)
        if word >> 26 == 3:
            b97c_jals.append(jal_target(word))
    require(b97c_jals == [], f"3B97C jals {b97c_jals}")

    require((FN_3BCE0_END - FN_3BCE0) // 4 == 245, "3BCE0 245 words")
    require(window_sha(data, FN_3BCE0, FN_3BCE0_END) == WIN_3BCE0, "3BCE0 sha")
    require(load_u32(data, FN_3BCE0) == 0x27BDFFE0, "3BCE0 addiu sp")
    require(load_u32(data, 0x8003BCE8) == 0x8C830000, "3BCE0 lw dest+0")
    require(load_u32(data, 0x8003BCF8) == 0x848200BA, "3BCE0 lh +0xBA")
    require(load_u32(data, 0x8003BD18) == 0x8C8A0010, "3BCE0 lw +0x10")
    require(load_u32(data, 0x8003BD20) == 0x8C890054, "3BCE0 lw +0x54")
    require(load_u32(data, 0x8003BD1C) == 0x94620008, "3BCE0 lhu obj+8")
    require(load_u32(data, 0x8003BD78) == 0x94C2FFFA, "3BCE0 lhu rec-6")
    require(load_u32(data, 0x8003BD90) == 0xACA20004, "3BCE0 sw pkt+4")
    require(load_u32(data, 0x8003BDE0) == 0xA0A70007, "3BCE0 sb keep +7 after sw+4")
    require(load_u32(data, 0x8003BE14) == 0x9442000A, "3BCE0 lhu obj+A")
    require(load_u32(data, 0x8003BEEC) == 0x9442000C, "3BCE0 lhu obj+C")
    require(load_u32(data, 0x8003BFDC) == 0x9442000E, "3BCE0 lhu obj+E")
    require(load_u32(data, 0x8003C0AC) == 0x03E00008, "3BCE0 jr")
    bce0_jals = []
    for va in range(FN_3BCE0, FN_3BCE0_END, 4):
        word = load_u32(data, va)
        if word >> 26 == 3:
            bce0_jals.append(jal_target(word))
    require(bce0_jals == [], f"3BCE0 jals {bce0_jals}")

    print(
        "PASS: 3D834 a1==0 jals 3A088/3B97C/3BCE0x2; "
        "3A088 392w mode 1/3/4 skip + empty blez; "
        "3B97C 217w empty gates; 3BCE0 245w four directory walks"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
