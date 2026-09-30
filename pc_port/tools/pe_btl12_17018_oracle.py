#!/usr/bin/env python3
"""PE-BTL12 independent oracle: func_80017018 task VM.

Pins SHA-1-exact EXE. 17018 is 159 words, SHA-256 0b2a2f69…fcd8.
Sole TEXT jal is 361F4 @ 0x80036224. Dispatch is jalr
D_800910A0[word & 0x1FFF]. Opcode 1 is 0x800172BC (ori +0x98
0x10, v0=0). Does not import production C. Does not mark M2.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FN = 0x80017018
FN_END = 0x80017294
FN_SHA = "0b2a2f695512016e38ccb63aea6065fb6914e9fdecbf57ed69fc9c7b2609fcd8"
OP1 = 0x800172BC
OP1_END = 0x800172E0


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


def jal_sites(data: bytes, target: int) -> list[int]:
    jal_word = 0x0C000000 | ((target & 0x0FFFFFFF) >> 2)
    sites = []
    for offset in range(0, len(data) - 4, 4):
        if struct.unpack_from("<I", data, offset)[0] == jal_word:
            sites.append(0x80010000 + offset - 0x800)
    return sites


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")
    tsize = struct.unpack_from("<I", data, 0x1C)[0]
    require(tsize == 0x1EE000, "tsize")
    require(FN_END <= 0x80010000 + tsize, "17018 inside EXE")

    require((FN_END - FN) // 4 == 159, "17018 159 words")
    require(window_sha(data, FN, FN_END) == FN_SHA, "17018 sha")
    require(load_u32(data, 0x80017034) == 0x8F830590, "lw gp+0x590 task")
    require(load_u32(data, 0x80017044) == 0x30420050, "andi task+8 0x50")
    require(load_u32(data, 0x800170CC) == 0x8C620010, "lw task+0x10")
    require(load_u32(data, 0x800170EC) == 0xAF820090, "sw PC gp+0x90")
    require(load_u32(data, 0x80017110) == 0x30CA1FFF, "andi op 0x1FFF")
    require(load_u32(data, 0x80017240) == 0x0040F809, "jalr handler")
    require(load_u32(data, 0x80017268) == 0x8C420024, "lw task+0x24")
    require(jal_sites(data, FN) == [0x80036224], "17018 callers")
    require(jal_target(load_u32(data, 0x80036224)) == FN, "361F4 jal 17018")

    require(load_u32(data, 0x800910A0) == 0x80017294, "table[0]")
    require(load_u32(data, 0x800910A4) == OP1, "table[1]")
    require(load_u32(data, 0x80091110) == 0x80017764, "table[0x1C]")
    require(load_u32(data, 0x8009111C) == 0x800177AC, "table[0x1F]")
    require((OP1_END - OP1) // 4 == 9, "op1 9 words")
    require(load_u32(data, 0x800172D0) == 0x34420010, "op1 ori 0x10")
    require(load_u32(data, 0x800172DC) == 0x00001021, "op1 v0=0")
    require(load_u32(data, 0x80010690) == 0x80017154, "kind0")
    require(load_u32(data, 0x80010694) == 0x8001718C, "kind1")
    require(load_u32(data, 0x80010698) == 0x800171BC, "kind2")
    require(load_u32(data, 0x8001069C) == 0x8001716C, "kind3")
    require(load_u32(data, 0x800106A0) == 0x800171DC, "kind4")

    print(
        "PASS: 17018 159w sha 0b2a2f69… 361F4@36224 jalr 910A0; "
        "op1 172BC ori +0x98 0x10 v0=0; EXE-resident"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
