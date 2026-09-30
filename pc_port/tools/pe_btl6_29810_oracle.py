#!/usr/bin/env python3
"""PE-BTL6 independent oracle: 29810 prologue at 0x80029810.

Pins SHA-1-exact EXE. 15 words before jal 20EFC: lw D_8009D254,
lw 0(actor), sw 0 D_8009D1E8 / D_8009D290 / D_8009D28C (mode 0,
NOT 7), sb 0 D_8009CE7C / D_8009CE78 / D_8009D288 / D_8009CE74,
sw *actor → D_8009D278. Delay of 20EFC is addu s0,a0,zero.
Does not import production C. Does not claim mode 7, overlay jalr,
or 0x55 completion.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
START = 0x80029810
END = 0x8002984C
SHA = "a8fd26f9f2fca11af23420bcfdcb6c44a4b21acaf316c616a5919c995e3f64e8"
GP = 0x8009CD70
FN_20EFC = 0x80020EFC


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

    require((END - START) // 4 == 15, "15 words")
    require(window_sha(data, START, END) == SHA, "prologue sha")
    require(load_u32(data, START) == 0x3C02800A, "lui 0x800A")
    require(load_u32(data, START + 4) == 0x8C42D254, "lw D_8009D254")
    require(0x800A0000 - 0x2DAC == 0x8009D254, "D254")
    require(load_u32(data, 0x80029824) == 0x8C420000, "lw *actor")
    require(load_u32(data, 0x80029828) == 0xAF800478, "sw D_8009D1E8")
    require(GP + 0x478 == 0x8009D1E8, "D1E8")
    require(load_u32(data, 0x80029830) == 0xAC20D290, "sw D_8009D290")
    require(0x800A0000 - 0x2D70 == 0x8009D290, "D290")
    require(load_u32(data, 0x80029834) == 0xAF80051C, "sw D_8009D28C=0")
    require(GP + 0x51C == 0x8009D28C, "D28C")
    require(load_u32(data, 0x80029838) == 0xA380010C, "sb D_8009CE7C")
    require(GP + 0x10C == 0x8009CE7C, "CE7C")
    require(load_u32(data, 0x8002983C) == 0xA3800108, "sb D_8009CE78")
    require(GP + 0x108 == 0x8009CE78, "CE78")
    require(load_u32(data, 0x80029840) == 0xA3800518, "sb D_8009D288")
    require(GP + 0x518 == 0x8009D288, "D288")
    require(load_u32(data, 0x80029844) == 0xA3800104, "sb D_8009CE74")
    require(GP + 0x104 == 0x8009CE74, "CE74")
    require(load_u32(data, 0x80029848) == 0xAF820508, "sw D_8009D278=*actor")
    require(GP + 0x508 == 0x8009D278, "D278")
    require(jal_target(load_u32(data, 0x8002984C)) == FN_20EFC, "jal 20EFC")
    require(load_u32(data, 0x80029850) == 0x00808021, "delay s0=a0")
    require(load_u32(data, 0x80029834) != 0x24020007, "not li 7")
    require(load_u32(data, 0x80029834) != 0xAF82051C, "not sw v0 mode7")

    print(
        "PASS: 29810 prologue 15w sha a8fd26f9…64e8; D28C=0 not 7; "
        "D278=lw(*D254); then 20EFC; no mode7/0x55"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
