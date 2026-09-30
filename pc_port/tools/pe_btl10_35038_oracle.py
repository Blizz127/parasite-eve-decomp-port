#!/usr/bin/env python3
"""PE-BTL10 independent oracle: func_8001266C and func_80035038 a1=0.

Pins SHA-1-exact EXE. 1266C is 37 words, SHA-256 34d11f74…645d.
Sole TEXT caller is 3F074 @ 0x8003F0B8 after 34FC4. It publishes
D_8009D310 to 0x8C($gp) for 12700.

35038 is 328 words. Live 125E0 arguments are a1=0, a2=1. Empty
D_8009D2AC returns 0. Nonempty a1=0 inserts at D_8009D20C, jals
12700, and when +0x1AC==0 ORs 0xE0 into +0x98.

Does not import production C. Does not mark M2. Type0 jal 2F76C
and +0x1AC!=0 (1A680/362B8/3D050) are not this cut.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
POOL = 0x8001266C
POOL_END = 0x80012700
POOL_SHA = "34d11f74f5167d70c1e204cc11ba59877b0328cf69c13295cb11beebc792645d"
CTOR = 0x80035038
CTOR_END = 0x80035558
CTOR_SHA = "412e4f80956062751f9d5e9826474c258689adfa1710b81295503a2c77e76ea7"


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
    require(POOL_END <= 0x80010000 + tsize, "1266C inside EXE")
    require(CTOR_END <= 0x80010000 + tsize, "35038 inside EXE")

    require((POOL_END - POOL) // 4 == 37, "1266C 37 words")
    require(window_sha(data, POOL, POOL_END) == POOL_SHA, "1266C sha")
    require(load_u32(data, 0x80012680) == 0xAF82008C, "1266C sw gp+0x8C")
    require(load_u32(data, 0x80012674) == 0x2442D310, "1266C D_8009D310")
    require(load_u32(data, 0x8001267C) == 0xAF800590, "1266C sw 0 gp+0x590")
    require(load_u32(data, 0x800126B4) == 0x2C420047, "1266C sltiu 71")
    require(load_u32(data, 0x800126C4) == 0xAC20DF68, "1266C last +0x24=0")
    require(load_u32(data, 0x800126EC) == 0x2C420040, "1266C zero 64 words")
    require(jal_sites(data, POOL) == [0x8003F0B8], "1266C callers")
    require(jal_target(load_u32(data, 0x8003F0B0)) == 0x80034FC4, "3F074 jal 34FC4")
    require(jal_target(load_u32(data, 0x8003F0B8)) == POOL, "3F074 jal 1266C")

    require((CTOR_END - CTOR) // 4 == 328, "35038 328 words")
    require(window_sha(data, CTOR, CTOR_END) == CTOR_SHA, "35038 sha")
    require(load_u32(data, CTOR) == 0x8F82053C, "lw gp+0x53C freelist")
    require(load_u32(data, 0x80035058) == 0x14400003, "bnez freelist")
    require(load_u32(data, 0x80035064) == 0x00001021, "v0=0 empty")
    require(load_u32(data, 0x80035074) == 0xAF82053C, "pop sw gp+0x53C")
    require(load_u32(data, 0x80035078) == 0x10A0000B, "beq a1,0")
    require(load_u32(data, 0x800350C4) == 0xAF91049C, "a1=0 sw gp+0x49C")
    require(jal_target(load_u32(data, 0x8003515C)) == 0x8002F76C, "type0 jal 2F76C")
    require(jal_target(load_u32(data, 0x8003526C)) == 0x80012700, "jal 12700")
    require(load_u32(data, 0x80035270) == 0xAE24009C, "sw entry +0x9C")
    require(load_u32(data, 0x80035274) == 0xAE2200A8, "sw task +0xA8")
    require(load_u32(data, 0x80035294) == 0x106000A3, "beq +0x1AC,0")
    require(load_u32(data, 0x8003552C) == 0x344200E0, "ori +0x98 0xE0")
    require(load_u32(data, 0x800351A8) == 0x8C220E70, "lw D_800B0E70[type]")
    require(
        jal_sites(data, CTOR) == [0x80012628, 0x80016C1C, 0x80017394],
        "35038 callers",
    )
    require(load_u32(data, 0x8001261C) == 0x00002821, "125E0 a1=0")
    require(load_u32(data, 0x80012624) == 0x24060001, "125E0 a2=1")

    print(
        "PASS: 1266C 37w sha 34d11f74… 3F074@3F0B8 sw gp+0x8C; "
        "35038 328w sha 412e4f80… a1=0 insert D20C jal 12700; "
        "+0x1AC==0 ori 0xE0; EXE-resident"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
