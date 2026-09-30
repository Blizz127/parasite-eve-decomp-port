#!/usr/bin/env python3
"""PE-BTL7 independent oracle: func_8006CC68 EE=0 idle / 3F074 poll-exit.

Pins SHA-1-exact EXE. 79 words 0x8006CC68..0x8006CDA4,
SHA-256 262dcfc6…a74d. Every arm returns 0. Live gates:
overlay[0]&0x000C0000, D254, actor+0x98&0x20000040, D1A0 bit1
or D2E8 bit1. First pass stores D_800B0D10=D254+0x1B4,
sh 3 / 0x12 at overlay+0x3C/+0x3E. Jals 661A4, 3A088,
3AC90, 3AF14, 661CC. 661A4/661CC are OFX/OFY ctc2.
3F074 after v0=0 jals 1A918, 371B0, 125E0, 0x800E0060
(loaded; do not fake), 74DC0, 74D28. Does not import
production C. Does not claim 0x55 complete by itself.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FN = 0x8006CC68
FN_END = 0x8006CDA4
WIN_SHA = "262dcfc6644b12a4e4398b45d04569040cedbc64b96a05de8dad652bf7fba74d"
WIN_661A4 = "79eb7a676b370467b30a5e04726833bf40c3cfe844e484c4071945353386de66"
WIN_661CC = "43c61e1933e6226f522058a9a12250713acba04d079deff0ef384709523c231e"


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

    require((FN_END - FN) // 4 == 79, "6CC68 79 words")
    require(window_sha(data, FN, FN_END) == WIN_SHA, "6CC68 sha")
    require(load_u32(data, FN) == 0x27BDFFE0, "prologue")
    require(load_u32(data, 0x8006CC70) == 0x3C10800B, "s0 lui overlay")
    require(load_u32(data, 0x8006CC74) == 0x26100CD8, "s0 = D_800B0CD8")
    require(load_u32(data, 0x8006CC80) == 0x8E020000, "lw overlay[0]")
    require(load_u32(data, 0x8006CC84) == 0x3C03000C, "lui 0x000C")
    require(load_u32(data, 0x8006CC8C) == 0x1440003E, "bnez 0xC0000 skip")
    require(load_u32(data, 0x8006CC98) == 0x8C84D254, "lw D254")
    require(load_u32(data, 0x8006CCA0) == 0x10800039, "beqz D254 skip")
    require(load_u32(data, 0x8006CCA8) == 0x8C820098, "lw actor+0x98")
    require(load_u32(data, 0x8006CCAC) == 0x34630040, "ori 0x20000040")
    require(load_u32(data, 0x8006CCC0) == 0x8C42D1A0, "lw D1A0")
    require(load_u32(data, 0x8006CCC8) == 0x30420002, "andi D1A0 2")
    require(load_u32(data, 0x8006CD04) == 0xAC220D10, "sw D_800B0D10")
    require(load_u32(data, 0x8006CD08) == 0x24020003, "li 3")
    require(load_u32(data, 0x8006CD10) == 0xA4220D14, "sh D_800B0D14")
    require(load_u32(data, 0x8006CD14) == 0x24020012, "li 0x12")
    require(load_u32(data, 0x8006CD1C) == 0xA4220D16, "sh D_800B0D16")
    require(load_u32(data, 0x8006CD88) == 0x00001021, "join v0=0")
    require(load_u32(data, 0x8006CD9C) == 0x03E00008, "jr ra")

    jals = []
    for va in range(FN, FN_END, 4):
        word = load_u32(data, va)
        if word >> 26 == 3:
            jals.append(jal_target(word))
    require(
        jals == [0x800661A4, 0x8003A088, 0x8003AC90, 0x8003AF14, 0x800661CC],
        f"6CC68 jals {jals}",
    )
    jalr = [va for va in range(FN, FN_END, 4) if load_u32(data, va) & 0xFC00003F == 0x00000009]
    require(jalr == [], f"6CC68 jalr {jalr}")

    require((0x800661CC - 0x800661A4) // 4 == 10, "661A4 10w")
    require(window_sha(data, 0x800661A4, 0x800661CC) == WIN_661A4, "661A4 sha")
    require(load_u32(data, 0x800661A8) == 0x9463CF94, "661A4 lhu CF94")
    require(load_u32(data, 0x800661B0) == 0x9484CF96, "661A4 lhu CF96")
    require(load_u32(data, 0x800661BC) == 0x48CCC000, "661A4 ctc2 OFX")
    require(load_u32(data, 0x800661C0) == 0x48CDC800, "661A4 ctc2 OFY")
    require((0x800661EC - 0x800661CC) // 4 == 8, "661CC 8w")
    require(window_sha(data, 0x800661CC, 0x800661EC) == WIN_661CC, "661CC sha")
    require(load_u32(data, 0x800661CC) == 0x240300A0, "661CC 0xA0")
    require(load_u32(data, 0x800661D0) == 0x24040070, "661CC 0x70")

    require((0x8003AF14 - 0x8003AC90) // 4 == 161, "3AC90 161w")
    require((0x8003B144 - 0x8003AF14) // 4 == 140, "3AF14 140w")

    require(jal_target(load_u32(data, 0x8003F23C)) == 0x8001A918, "after-poll 1A918")
    require(jal_target(load_u32(data, 0x8003F274)) == 0x800371B0, "after-poll 371B0")
    require(jal_target(load_u32(data, 0x8003F27C)) == 0x800125E0, "after-poll 125E0")
    require(jal_target(load_u32(data, 0x8003F284)) == 0x800E0060, "after-poll E0060")
    require(jal_target(load_u32(data, 0x8003F298)) == 0x80074DC0, "after-poll 74DC0")
    require(jal_target(load_u32(data, 0x8003F2A0)) == 0x80074D28, "after-poll 74D28")

    print(
        "PASS: 6CC68 79w sha 262dcfc6… always v0=0; D_800B0D10=actor+0x1B4; "
        "jals 661A4/3A088/3AC90/3AF14/661CC; 3F074 after-poll 1A918…E0060"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
