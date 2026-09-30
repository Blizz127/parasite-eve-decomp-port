#!/usr/bin/env python3
"""PE-BTL11 independent oracle: 35558 D20C walk and type!=0 35E04.

Pins SHA-1-exact EXE. 35558 is 459 words, SHA-256 7352fc04…ba83.
Sole TEXT caller is 3F3C4 @ 0x8003F4F0. Prologue walks D_8009D20C
and jalrs +0x190 unless D1A0 bit 2 (value 4) is set.

Types 1–9 vtable is 0x80035E04 (83w). 361F4 (24w) publishes the
actor to gp+0x580 and +0xA0 slots to D_8009D300, then jals 17018
on nonempty slots. 17018 is not this cut.

Does not import production C. Does not mark M2.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
WALK = 0x80035558
WALK_END = 0x80035C84
WALK_SHA = "7352fc04217ff00c7777a716285d9a109189f122ed15f2b8834fac81892eba83"
VT = 0x80035E04
VT_END = 0x80035F50
VT_SHA = "848071161e0b27f785bc765a049211545a2c9666366dbe8340f1ee4a1ef75c6d"
PUB = 0x800361F4
PUB_END = 0x80036254
PUB_SHA = "1ebed0df893df12fa824df626a4498d21010c197e6c7b81cc9a129b2cec1cbe5"


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
    require(WALK_END <= 0x80010000 + tsize, "35558 inside EXE")
    require(VT_END <= 0x80010000 + tsize, "35E04 inside EXE")
    require(PUB_END <= 0x80010000 + tsize, "361F4 inside EXE")

    require((WALK_END - WALK) // 4 == 459, "35558 459 words")
    require(window_sha(data, WALK, WALK_END) == WALK_SHA, "35558 sha")
    require(load_u32(data, 0x80035570) == 0x30420004, "andi D1A0 4")
    require(load_u32(data, 0x8003557C) == 0x8F91049C, "lw gp+0x49C")
    require(load_u32(data, 0x8003558C) == 0x8E220190, "lw +0x190")
    require(load_u32(data, 0x8003559C) == 0x0040F809, "jalr v0")
    require(load_u32(data, 0x800355A4) == 0x8E310004, "lw +4 next")
    require(jal_sites(data, WALK) == [0x8003F4F0], "35558 callers")
    require(jal_target(load_u32(data, 0x8003F4F0)) == WALK, "3F3C4 jal 35558")
    require(jal_sites(data, 0x8003F3C4) == [0x800123D8], "3F3C4 callers")

    require(load_u32(data, 0x800915E4) == VT, "type1 vtable 35E04")
    require(load_u32(data, 0x800915DC) == 0x80035C84, "type0 vtable 35C84")
    require((VT_END - VT) // 4 == 83, "35E04 83 words")
    require(window_sha(data, VT, VT_END) == VT_SHA, "35E04 sha")
    require(load_u32(data, 0x80035E18) == 0x30420100, "35E04 andi D1A0 0x100")
    require(jal_target(load_u32(data, 0x80035E24)) == PUB, "35E04 jal 361F4")
    require(jal_target(load_u32(data, 0x80035E64)) == PUB, "35E04 jal 361F4 b")
    require(load_u32(data, 0x80035E74) == 0x30420002, "35E04 andi +0x98 2")

    require((PUB_END - PUB) // 4 == 24, "361F4 24 words")
    require(window_sha(data, PUB, PUB_END) == PUB_SHA, "361F4 sha")
    require(load_u32(data, 0x8003620C) == 0xAF840580, "361F4 sw gp+0x580")
    require(jal_target(load_u32(data, 0x80036224)) == 0x80017018, "361F4 jal 17018")
    require(jal_sites(data, 0x80017018) == [0x80036224], "17018 sole caller")
    require(load_u32(data, 0x80036230) == 0x2E220003, "361F4 sltiu 3")

    print(
        "PASS: 35558 459w sha 7352fc04… 3F3C4@3F4F0 D20C jalr +0x190; "
        "35E04 83w types 1-9; 361F4 24w gp+0x580 / 17018; EXE-resident"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
