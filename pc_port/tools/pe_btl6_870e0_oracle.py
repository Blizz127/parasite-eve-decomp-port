#!/usr/bin/env python3
"""PE-BTL6 independent oracle: func_800870E0 + 6CDA4 state 0xA.

Pins SHA-1-exact EXE. 870E0 is 4 words: return D_8009D24C.
State 0xA: -1 sb F0=0; busy stays 0xA; 0 remain-=chunk sb 7.
Does not import production C. Does not claim DMA-complete,
851A8/6914C success, mode 7, or 0x55 completion.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FN = 0x800870E0
FN_END = 0x800870F0
WIN_SHA256 = (
    "bc8b76454f526e929e58c6350e12408fbb4e627594ef3377e91234dd862a6b66"
)
D24C_LO = 0xD24C


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


def is_d24c_store(word: int) -> bool:
    return (word >> 26) == 0x2B and (word & 0xFFFF) == D24C_LO


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")

    require((FN_END - FN) // 4 == 4, "870E0 4 words")
    require(window_sha(data, FN, FN_END) == WIN_SHA256, "870E0 sha256")
    require(load_u32(data, FN) == 0x3C02800A, "lui 800a")
    require(load_u32(data, FN + 4) == 0x8C42D24C, "lw D_8009D24C")
    require(load_u32(data, FN + 8) == 0x03E00008, "jr")
    require(load_u32(data, FN + 12) == 0x00000000, "nop")
    for va in range(FN, FN_END, 4):
        require(not is_d24c_store(load_u32(data, va)), f"870E0 store @{va:#x}")

    require(jal_target(load_u32(data, 0x8006CFE8)) == FN, "stateA jal 870E0")
    require(load_u32(data, 0x8006CFF0) == 0x00403021, "a2=v0")
    require(load_u32(data, 0x8006CFF4) == 0x14D60003, "bne a2,s6")
    require(load_u32(data, 0x8006D000) == 0xA24000F0, "-1 sb F0=0")
    require(load_u32(data, 0x8006D004) == 0x10C00005, "beq a2,0 complete")
    require(load_u32(data, 0x8006D02C) == 0x24030007, "complete li 7")
    require(load_u32(data, 0x8006D030) == 0xA24300F0, "complete sb F0=7")
    require(load_u32(data, 0x8006D024) == 0x8F820408, "lw remain gp+0x408")
    require(load_u32(data, 0x8006D028) == 0x8F84040C, "lw chunk gp+0x40C")
    require(load_u32(data, 0x8006D034) == 0x00441023, "subu remain,chunk")
    require(load_u32(data, 0x8006D038) == 0xAF820408, "sw remain")

    require(is_d24c_store(load_u32(data, 0x800850AC)), "85098 sw 0")
    require(load_u32(data, 0x800850AC) == 0xAC20D24C, "85098 $zero")
    require(is_d24c_store(load_u32(data, 0x800850D8)), "850C0 sw v0")
    require(load_u32(data, 0x800850C4) == 0x24020001, "850C0 v0=1")
    require(is_d24c_store(load_u32(data, 0x8008526C)), "851A8 fail sw")

    print(
        "PASS: 870E0 4w sha256 return D_8009D24C no store; "
        "stateA -1→0 busy stay 0 complete remain-=chunk sb 7; "
        "writers 85098=0 850C0=1 851A8-fail"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
