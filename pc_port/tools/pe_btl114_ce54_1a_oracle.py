#!/usr/bin/env python3
"""PE-BTL114 — CE54 writer is 24A3C case 9; +0x1A is 1A4AC's +0x18 high half."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
ROOT = pathlib.Path(__file__).resolve().parents[2]
EXE = ROOT / "build" / "disc1.candidate.exe"
TADDR = 0x80010000
HDR = 0x800


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(va: int) -> int:
    return va - TADDR + HDR


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, exe_off(va))[0]


def jal_target(word: int) -> int:
    return 0x80000000 | ((word & 0x3FFFFFF) << 2)


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require(load_u32(blob, 0x80024A3C) == 0x938304EC, "24A3C lbu D25C")
    require(load_u32(blob, 0x80010824 + 9 * 4) == 0x80024F48, "jtbl[9]")
    require(load_u32(blob, 0x80024F54) == 0x9083000F, "case9 lbu +0x0F")
    require(load_u32(blob, 0x80024F58) == 0x9482001A, "case9 lhu +0x1A")
    require(load_u32(blob, 0x80024F90) == 0x24020001, "li 1")
    require(load_u32(blob, 0x80024F94) == 0xA38200E4, "sb CE54")
    require(load_u32(blob, 0x80024F8C) == 0xA38200E5, "sb CE55")
    require(jal_target(load_u32(blob, 0x80021E6C)) == 0x80022394, "21DE0 22394")
    require(jal_target(load_u32(blob, 0x800229D8)) == 0x80024A3C, "22394 24A3C")
    require(load_u32(blob, 0x8001A520) == 0xAE070018, "1A4AC sw +0x18")
    require(load_u32(blob, 0x8002B1E8) == 0x9083000F, "phase2 lbu +0x0F")
    require(load_u32(blob, 0x8002B1EC) == 0x9482001A, "phase2 lhu +0x1A")
    require(load_u32(blob, 0x80024C7C) == 0x94820016, "case4 lhu +0x16")
    require(load_u32(blob, 0x80024D04) == 0x24050006, "case5 a1=6")
    require(load_u32(blob, 0x80024D1C) == 0x90C20252, "case6 lbu +0x252")
    require(load_u32(blob, 0x80024E88) == 0x9482001A, "case7 lhu +0x1A")
    require(load_u32(blob, 0x80024F10) == 0x9482001A, "case8 lhu +0x1A")
    require(load_u32(blob, 0x80024988) == 0x3C030008, "24250 lui 0x80000")
    require(load_u32(blob, 0x80024994) == 0xAC82004C, "24250 sw rec+0x4C")
    require(jal_target(load_u32(blob, 0x80022C9C)) == 0x80024250, "22394 24250")
    require(load_u32(blob, 0x800107D4 + 19 * 4) == 0x80024974, "jtbl[19]")
    require(load_u32(blob, 0x80051314) == 0x24420183, "5112C addiu 387")
    require(load_u32(blob, 0x8006D634) == 0x2C620041, "6D60C sltiu 0x41")
    require(load_u32(blob, 0x80011508) == 0x8006D658, "jtbl[0] 6D658")
    require(load_u32(blob, 0x80011508 + 45 * 4) == 0x8006D8BC, "jtbl[45]")
    require(load_u32(blob, 0x80011508 + 50 * 4) == 0x8006D944, "jtbl[50]")
    require(load_u32(blob, 0x80011508 + 64 * 4) == 0x8006D9E8, "jtbl[64]")
    require(load_u32(blob, 0x8006D664) == 0x12400021, "a0==0 → 6D6EC")
    require(load_u32(blob, 0x8006D71C) == 0x2402002D, "6D6EC li 45")
    require(load_u32(blob, 0x8006D8F0) == 0x24020032, "45 fallthrough li 50")
    require(load_u32(blob, 0x8006D948) == 0x2402FFFF, "50 lh -1 compare")
    require(load_u32(blob, 0x8006DA54) == 0xA22000F2, "64 sb F2=0")
    require(jal_target(load_u32(blob, 0x80024AF4)) == 0x8003C5D8, "case0 3C5D8")
    require(jal_target(load_u32(blob, 0x80024B48)) == 0x8006F39C, "case0 6F39C")
    require(load_u32(blob, 0x80024B4C) == 0x2404006B, "case0 a0=0x6B")
    require(jal_target(load_u32(blob, 0x80024BF0)) == 0x8003C5D8, "case2 3C5D8")
    require(jal_target(load_u32(blob, 0x80024C10)) == 0x8006F39C, "case2 6F39C")
    require(load_u32(blob, 0x80024C08) == 0x2404006C, "case2 a0=0x6C")
    require(jal_target(load_u32(blob, 0x80025150)) == 0x8003C5D8, "case5 3C5D8")
    require(load_u32(blob, 0x8002514C) == 0x2405000F, "case5 a1=15")
    require(load_u32(blob, 0x800113D0) == 0x8006C218, "6C1CC jtbl[0]")
    require(load_u32(blob, 0x8006C23C) == 0x2402000E, "state32 li 14")
    require(load_u32(blob, 0x800113D0 + 7 * 4) == 0x8006C390, "jtbl[7] 39")
    require(jal_target(load_u32(blob, 0x8006C43C)) == 0x8003D834, "39 3D834")
    require(load_u32(blob, 0x8006C4A0) == 0x24020020, "39 li 32")
    require(load_u32(blob, 0x8003D834) == 0x27BDFFD8, "3D834 addiu sp")
    print("PASS: 24F94 is the CE54 store; 1A4AC copies +0x14 to +0x18")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
