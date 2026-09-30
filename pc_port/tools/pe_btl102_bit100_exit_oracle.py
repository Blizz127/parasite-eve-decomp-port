#!/usr/bin/env python3
"""PE-BTL102 — 3F3C4 bit 0x100 skips draw, still dest-changes; 1220C restarts."""
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


def j_target(word: int) -> int:
    return 0x80000000 | ((word & 0x3FFFFFF) << 2)


def simm16(value: int) -> int:
    return value - 0x10000 if value & 0x8000 else value


def branch_target(pc: int, word: int) -> int:
    return pc + 4 + simm16(word & 0xFFFF) * 4


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")

    require(jal_target(load_u32(blob, 0x8003F4F0)) == 0x80035558, "jal 35558")
    require(load_u32(blob, 0x8003F500) == 0x30420100, "andi 0x100")
    require(branch_target(0x8003F504, load_u32(blob, 0x8003F504)) == 0x8003F5F4,
            "bnez 3F5F4 not jr")
    require(load_u32(blob, 0x8003F5F4) != 0x03E00008, "3F5F4 is join not jr")
    require(jal_target(load_u32(blob, 0x8003F6CC)) == 0x80074DC0, "dest-change DrawSync")
    require(jal_target(load_u32(blob, 0x8003F6D4)) == 0x80087024, "dest-change 87024")
    require(jal_target(load_u32(blob, 0x8003F6DC)) == 0x8003DFC8, "dest-change 3DFC8")
    require(jal_target(load_u32(blob, 0x8003F6E4)) == 0x800696F0, "dest-change 696F0")

    require(load_u32(blob, 0x800124A4) == 0x30420100, "1220C andi 0x100")
    require(branch_target(0x800124A8, load_u32(blob, 0x800124A8)) == 0x80012294,
            "beq continue inner")
    require(jal_target(load_u32(blob, 0x800124B0)) == 0x80073A44, "VSync")
    require(load_u32(blob, 0x800124B4) == 0x00002021, "VSync a0=0")
    require(jal_target(load_u32(blob, 0x800124B8)) == 0x80074D28, "SetDispMask")
    require(load_u32(blob, 0x800124BC) == 0x00002021, "SetDispMask a0=0")
    require(load_u32(blob, 0x800124C4) == 0x2403FEFF, "addiu ~0x100")
    require(j_target(load_u32(blob, 0x800124CC)) == 0x8001224C, "j outer 6A5BC")
    require(jal_target(load_u32(blob, 0x8001224C)) == 0x8006A5BC, "1224C is 6A5BC")

    print("PASS: 3F3C4 0x100 → 3F5F4 dest-change; 1220C j 1224C")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
