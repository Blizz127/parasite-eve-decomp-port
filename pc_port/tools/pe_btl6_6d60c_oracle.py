#!/usr/bin/env python3
"""PE-BTL6 independent oracle: 144FC 0x38 jal 6D60C(1).

Pins SHA-1-exact EXE: 387-word window, +0xF2 JT, state 0 / 0x2C / 0x2E,
no +0xE store, sole 6D078 jal from 0x2E. Does not import production C.
Does not claim 6D60C success, 6914C success, mode 7, or 0x55 completion.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FN = 0x8006D60C
FN_END = 0x8006DC18
JT = 0x80011508
STATE0 = 0x8006D658
STATE2C = 0x8006D728
STATE2E = 0x8006D788
DEFAULT = 0x8006DB28
FN_87024 = 0x80087024
FN_6D078 = 0x8006D078
FN_6D078_END = 0x8006D24C
WIN_SHA256 = (
    "14e5794d4515e763d20ee26c939500f8764e747e83eddd7ad64b4642d905c3a2"
)
WIN_6D078_SHA256 = (
    "7cf2bb6168b5ccf24db7639f024d093d1925dc90d63cba77ac861c4ffde5b1dd"
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

    require((FN_END - FN) // 4 == 387, "6D60C 387 words")
    require(window_sha(data, FN, FN_END) == WIN_SHA256, "6D60C sha256")
    require(load_u32(data, FN + 8) == 0x00809021, "s2=a0")
    require(load_u32(data, FN + 0x10) == 0x3C11800B, "s1 lui overlay")
    require(load_u32(data, FN + 0x14) == 0x26310CD8, "s1 +0xCD8")
    require(load_u32(data, FN + 0x20) == 0x922300F2, "lbu +0xF2")
    require(load_u32(data, FN + 0x28) == 0x2C620041, "sltiu 0x41")
    require(load_u32(data, FN + 0x3C) == 0x8C221508, "lw JT 0x80011508")

    require(load_u32(data, JT + 0) == STATE0, "JT[0]")
    require(load_u32(data, JT + 0x2C * 4) == STATE2C, "JT[0x2C]")
    require(load_u32(data, JT + 0x2E * 4) == STATE2E, "JT[0x2E]")
    require(load_u32(data, DEFAULT) == 0x00001021, "default v0=0")
    for i in range(1, 0x2C):
        require(load_u32(data, JT + i * 4) == DEFAULT, f"JT[{i:#x}] default")

    require(jal_target(load_u32(data, 0x8006D65C)) == FN_87024, "state0 jal 87024")
    require(load_u32(data, 0x8006D6E0) == 0x2402002C, "state0 li 0x2C")
    require(load_u32(data, 0x8006D6E8) == 0xA22200F2, "state0 sb +0xF2")
    require(load_u32(data, 0x8006D728) == 0x822300E0, "0x2C lb +0xE0")
    require(load_u32(data, 0x8006D72C) == 0x822200DC, "0x2C lb +0xDC")
    require(jal_target(load_u32(data, STATE2E)) == FN_6D078, "0x2E jal 6D078")
    require(load_u32(data, STATE2E + 8) == 0x24030001, "0x2E li 1")
    require(load_u32(data, STATE2E + 0xC) >> 26 == 4, "0x2E beq v0,1 busy")
    require(load_u32(data, 0x8006D79C) == 0x8E230000, "D79C lw overlay word")
    require(load_u32(data, 0x8006D7A4) == 0x30620004, "D79C andi 4")
    require(load_u32(data, 0x8006D7AC) == 0x30620040, "D79C andi 0x40")
    require(load_u32(data, 0x8006D814) == 0xA22200F2, "D79C sb F2")
    require(load_u32(data, 0x8006D80C) == 0x2402003F, "live sb 0x3F")
    require(load_u32(data, JT + 0x3F * 4) == 0x8006D818, "JT[0x3F]")
    require(load_u32(data, JT + 0x2F * 4) == 0x8006D860, "JT[0x2F]")
    require(load_u32(data, 0x8006D858) == 0x0801B58B, "0x3F !bit40 j dispatch")
    require(load_u32(data, 0x8006D830) == 0x2402002F, "0x3F li 0x2F")
    require(jal_target(load_u32(data, 0x8006D878)) == 0x8006CDA4, "0x2F jal 6CDA4")
    require(load_u32(data, 0x8006D860) == 0x00002021, "0x2F a0=0")
    require(load_u32(data, 0x8006A9D0) == 0xAC220E64, "6A8D4 sw D_800B0E64")
    require(load_u32(data, 0x8006A9C8) == 0x2462FFF8, "6A8D4 D11614-8")
    require(load_u32(data, 0x8006EB10) == 0xAC220E64, "6EB10 sw D_800B0E64")
    require(load_u32(data, 0x8006EB08) == 0x2462FFF8, "6EB10 D11614-8")

    store_ops = {0x28: "sb", 0x29: "sh", 0x2B: "sw"}
    for va in range(FN, FN_END, 4):
        word = load_u32(data, va)
        op = word >> 26
        if op in store_ops and (word & 0xFFFF) == 0x000E:
            raise SystemExit(f"FAIL: 6D60C stores +0xE @{va:#x}")

    require(jal_target(load_u32(data, 0x8001459C)) == FN, "0x38 jal 6D60C")
    require(load_u32(data, 0x800145A0) == 0x24040001, "0x38 a0=1")

    jal = 0x0C000000 | ((FN_6D078 & 0x0FFFFFFF) >> 2)
    hits = [
        0x80010000 + offset - 0x800
        for offset in range(0, len(data) - 4, 4)
        if struct.unpack_from("<I", data, offset)[0] == jal
    ]
    require(hits == [STATE2E], f"6D078 jals {hits}")
    require((FN_6D078_END - FN_6D078) // 4 == 117, "6D078 117 words")
    require(
        window_sha(data, FN_6D078, FN_6D078_END) == WIN_6D078_SHA256,
        "6D078 sha256",
    )
    require(load_u32(data, FN_6D078 + 0x40) == 0x920300F3, "6D078 lbu +0xF3")

    print(
        "PASS: 6D60C 387w sha256; +0xF2 JT; state0 jal 87024 sb 0x2C; "
        "0x2C lb +0xE0/+0xDC; 0x2E jal 6D078; D79C bit4/40 sb 0x3F; "
        "0x3F !bit40 sb 0x2F; 0x2F 6CDA4 a0=0; B0E64=D11614-8 only; "
        "no +0xE; 0x38 a0=1; default v0=0"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
