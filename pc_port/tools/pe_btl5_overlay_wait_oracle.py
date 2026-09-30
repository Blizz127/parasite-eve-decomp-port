#!/usr/bin/env python3
"""PE-BTL5 independent oracle: overlay +0xE writers and 6914C role.

Checks SHA-1-exact EXE windows without importing production C:
144FC state 0x39 jals 6914C(1); 0x3B only lbu +0xE; 6914C never stores
D_800B0CE6; func_8006C4C4 sets wait bits; 6C5BC andi 0xFC clears them.
Does not claim 0x55 completion, mode 7, or a 6914C stub return.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_words

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
JT = 0x800100A0
FN_6914C = 0x8006914C
FN_6914C_END = 0x80069594
FN_6C4C4 = 0x8006C4C4
FN_6C4C4_END = 0x8006C5BC
FN_6C5BC = 0x8006C5BC
FN_6C5BC_END = 0x8006CC68
CLEAR = (0x8006CC20, 0x8006CC38)
STATE39 = (0x800145DC, 0x800145F8)
BYTE_E = 0x800B0CE6
WIN_6C5BC_SHA256 = (
    "d15126b6beafc9c3f4debd9155e47646e6c6c0a0df3e4daf3f96a7eae056686a"
)
JAL_6C5BC = 0x0C01B16F
JT_EE = 0x800113F0
EE_JT = exe_words(0x800113F0, 14)
JALS_6C5BC = [
    0x8006CC68, 0x8006E6A8, 0x8006CC68, 0x8006E7E8,
    0x8006CC68, 0x8006E1C0, 0x8006E6A8, 0x8006CC68,
    0x8006E7E8, 0x8006CC68, 0x8006CC68, 0x8006CDA4,
    0x8003D050, 0x8006698C, 0x8003D834,
]
SITES_6C5BC = (0x80035B24, 0x8003F22C, 0x8006C358)

WINDOWS = {
    STATE39: [
        0x0C01A453, 0x24040001, 0x24030001, 0x1043001D,
        0x2402003A, 0x08005146, 0xA20200F4,
    ],
    (FN_6914C, FN_6914C + 0x28): exe_words(0x8006914C, 10),
    (FN_6C4C4, FN_6C4C4_END): exe_words(0x8006C4C4, 62),
    CLEAR: [
        0x9283000E, 0x24020001, 0xA28000EE, 0x306300FC,
        0x0801B30F, 0xA283000E,
    ],
}


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def covers_ce6(kind: str, addr: int) -> bool:
    width = {"sb": 1, "sh": 2, "sw": 4}[kind]
    return addr <= BYTE_E < addr + width


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")

    for (start, end), words in WINDOWS.items():
        require(len(words) == (end - start) // 4, f"window size {start:#x}")
        for index, want in enumerate(words):
            got = load_u32(data, start + index * 4)
            require(got == want, f"{start + index * 4:#x}: {got:#010x}")

    require(load_u32(data, JT + 0x39 * 4) == 0x800145DC, "JT 0x39")
    require(load_u32(data, JT + 0x3A * 4) == 0x800145F8, "JT 0x3A")
    require(load_u32(data, JT + 0x3B * 4) == 0x80014630, "JT 0x3B")
    require(jal_target(load_u32(data, 0x800145DC)) == FN_6914C,
            "state 0x39 jal 6914C")
    require(load_u32(data, 0x800145E0) == 0x24040001, "state 0x39 a0=1")
    require(load_u32(data, 0x80014630) == 0x9202000E, "0x3B lbu +0xE")
    require(load_u32(data, 0x80014638) == 0x30420003, "0x3B andi 3")
    require(jal_target(load_u32(data, 0x8001461C)) == 0x80029810,
            "0x3A is 29810, not 6914C")

    require(jal_target(load_u32(data, 0x8002CEE0)) == FN_6914C,
            "mode-7 jal 6914C")
    require(load_u32(data, 0x8002CEE4) == 0x00002021, "mode-7 a0=0")

    require(load_u32(data, FN_6914C + 0x10) == 0x3C14800B, "6914C lui overlay")
    require(load_u32(data, FN_6914C + 0x14) == 0x26940CD8, "6914C $s4=+0xCD8")
    require((FN_6914C_END - FN_6914C) // 4 == 274, "6914C 274 words")

    store_ops = {0x28: "sb", 0x29: "sh", 0x2B: "sw"}
    hit = 0
    abs_reg: dict[int, int] = {}
    for va in range(FN_6914C, FN_6914C_END, 4):
        word = load_u32(data, va)
        op = word >> 26
        rs = (word >> 21) & 0x1F
        rt = (word >> 16) & 0x1F
        imm = word & 0xFFFF
        simm = imm - 0x10000 if imm >= 0x8000 else imm
        if op == 0x0F:
            abs_reg[rt] = (imm << 16) & 0xFFFFFFFF
        elif op == 0x09 and rs in abs_reg:
            abs_reg[rt] = (abs_reg[rs] + simm) & 0xFFFFFFFF
        elif op in store_ops and rs in abs_reg:
            addr = (abs_reg[rs] + simm) & 0xFFFFFFFF
            if covers_ce6(store_ops[op], addr):
                hit += 1
        elif op not in (0x0F, 0x09) and op not in store_ops:
            dest = rt if op in (0x08, 0x0C, 0x0D, 0x20, 0x21, 0x23, 0x24, 0x25) else None
            if dest is not None:
                abs_reg.pop(dest, None)
    require(hit == 0, f"6914C stored D_800B0CE6 ({hit})")

    require(load_u32(data, 0x8006C4F4) == 0x34420003, "6C4C4 ori 3")
    require(load_u32(data, 0x8006C504) == 0xA0220CE6, "6C4C4 sb +0xE ori3")
    require(load_u32(data, 0x8006C540) == 0x34420002, "6C4C4 ori 2")
    require(load_u32(data, 0x8006C548) == 0xA0220CE6, "6C4C4 sb +0xE ori2")
    require(load_u32(data, 0x8006C5AC) == 0x34420001, "6C4C4 ori 1")
    require(load_u32(data, 0x8006C5B0) == 0xA0C2000E, "6C4C4 sb +0xE ori1")
    require(load_u32(data, 0x8006CC2C) == 0x306300FC, "6C5BC andi 0xFC")
    require(load_u32(data, 0x8006CC34) == 0xA283000E, "6C5BC sb cleared +0xE")

    require((FN_6C5BC_END - FN_6C5BC) // 4 == 427, "6C5BC 427 words")
    require(load_u32(data, FN_6C5BC) == 0x3C02800B, "6C5BC lui CE2")
    require(load_u32(data, FN_6C5BC + 4) == 0x90420CE2, "6C5BC lbu CE2")
    require(load_u32(data, 0x8006C5FC) == 0x2442FFF6, "CE2-10")
    require(load_u32(data, 0x8006C600) == 0x2C420005, "CE2 sltiu 5")
    require(load_u32(data, 0x8006C60C) == 0x928300EE, "lbu +0xEE")
    require(load_u32(data, 0x8006C628) == 0x8C2213F0, "lw JT 0x800113F0")
    require(load_u32(data, 0x8006CC60) == 0x03E00008, "6C5BC jr ra")
    require(load_u32(data, FN_6C5BC_END) == 0x27BDFFE0, "next 6CC68")
    window = data[exe_off(FN_6C5BC):exe_off(FN_6C5BC_END)]
    require(hashlib.sha256(window).hexdigest() == WIN_6C5BC_SHA256,
            "6C5BC window sha256")
    for index, want in enumerate(EE_JT):
        require(load_u32(data, JT_EE + index * 4) == want,
                f"+0xEE JT[{index}]")

    jals = []
    for va in range(FN_6C5BC, FN_6C5BC_END, 4):
        word = load_u32(data, va)
        if word >> 26 == 3:
            jals.append(jal_target(word))
    require(jals == JALS_6C5BC, f"6C5BC jal callees {jals}")

    require(load_u32(data, 0x8006C9DC) == 0x34630001, "EE11 ori 1")
    require(load_u32(data, 0x8006C9E4) == 0xA283000E, "EE11 sb +0xE")
    require(load_u32(data, 0x80014604) == 0x34420002, "0x3A D1A0 ori 2")
    require(load_u32(data, 0x8001460C) == 0xAC22D1A0, "0x3A sw D1A0")

    require(jal_target(load_u32(data, 0x8003F3D4)) == 0x8003F074,
            "3F3C4 first jal 3F074")
    require(jal_target(load_u32(data, 0x8003F4F0)) == 0x80035558,
            "3F3C4 jal 35558")
    require(load_u32(data, 0x8003F224) == 0x0C01B131, "3F074 jal 6C4C4")
    require(load_u32(data, 0x8003F22C) == JAL_6C5BC, "3F074 jal 6C5BC")
    require(load_u32(data, 0x8003F234) == 0x1050FFFD, "3F074 poll beq v0,s0")
    require(load_u32(data, 0x80035B24) == JAL_6C5BC, "35558 jal 6C5BC")
    require(load_u32(data, 0x8006C358) == JAL_6C5BC, "6C1CC st6 jal 6C5BC")
    require(load_u32(data, 0x800113E8) == 0x8006C358, "6C1CC JT state 6")

    sites = []
    jal = JAL_6C5BC
    for offset in range(0, len(data) - 4, 4):
        if struct.unpack_from("<I", data, offset)[0] == jal:
            sites.append(0x80010000 + offset - 0x800)
    require(tuple(sites) == SITES_6C5BC, f"6C5BC jal sites {sites}")
    for name, lo, hi in (
        ("144FC", 0x800144FC, 0x80014694),
        ("29810", 0x80029810, 0x800299CC),
        ("6914C", 0x8006914C, 0x80069594),
        ("6D60C", 0x8006D60C, 0x8006DC18),
        ("6C4C4", FN_6C4C4, FN_6C4C4_END),
        ("6BECC", 0x8006BECC, 0x8006C1CC),
    ):
        for va in range(lo, hi, 4):
            require(load_u32(data, va) != jal, f"{name} jal 6C5BC")

    print(
        "PASS: 144FC 0x39 jal 6914C(1); 0x3B lbu+andi3; "
        "6914C 274 no +0xE store; 6C4C4 62; 6C5BC 427 "
        "sites 35B24/3F22C/6C358; 3F3C4→3F074 poll"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
