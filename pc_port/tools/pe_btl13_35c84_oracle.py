#!/usr/bin/env python3
"""PE-BTL13 independent oracle: type0 35C84 and 2F76C stores.

Pins SHA-1-exact EXE. 35C84 is 96 words, SHA-256 ab904f90…da62.
D_800915DC[0] = 35C84. 2F76C is 27 words, sole caller 35038.
Does not import production C. Does not mark M2. Does not reopen
1A918/E0060. E0060 remains EXE-resident list-clear.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
VT = 0x80035C84
VT_END = 0x80035E04
VT_SHA = "ab904f90342b4cf0730ca6bfc349734b7a0b814f1cf7e8854f1e072d0a9bda62"
CTOR = 0x8002F76C
CTOR_END = 0x8002F7D8
CTOR_SHA = "96c27c7ba13a82028cedbedff2ad56b64a4e582a8dffc3b1402385333b01e32c"


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
    require(VT_END <= 0x80010000 + tsize, "35C84 inside EXE")
    require(CTOR_END <= 0x80010000 + tsize, "2F76C inside EXE")
    require(0x800E00CC <= 0x80010000 + tsize, "E0060 still EXE-resident")

    require((VT_END - VT) // 4 == 96, "35C84 96 words")
    require(window_sha(data, VT, VT_END) == VT_SHA, "35C84 sha")
    require(load_u32(data, 0x800915DC) == VT, "type0 vtable")
    require(jal_target(load_u32(data, 0x80035CC0)) == 0x800361F4, "35C84 jal 361F4")
    require(load_u32(data, 0x80035CC8) == 0x8F820578, "lw gp+0x578 D2E8")
    require(load_u32(data, 0x80035CD0) == 0x30420001, "andi D2E8 1")
    require(jal_target(load_u32(data, 0x80035D14)) == 0x8003999C, "35C84 jal 3999C")
    require(load_u32(data, 0x80035D24) == 0x30420002, "andi +0x98 2")
    require(jal_sites(data, VT) == [], "35C84 jalr-only")

    require((CTOR_END - CTOR) // 4 == 27, "2F76C 27 words")
    require(window_sha(data, CTOR, CTOR_END) == CTOR_SHA, "2F76C sha")
    require(load_u32(data, 0x8002F77C) == 0xAC820000, "2F76C sw actor+0")
    require(jal_target(load_u32(data, 0x8002F7A0)) == 0x8005218C, "jal 5218C")
    require(jal_target(load_u32(data, 0x8002F7B0)) == 0x80051980, "jal 51980")
    require(jal_target(load_u32(data, 0x8002F7C0)) == 0x80051E64, "jal 51E64")
    require(jal_sites(data, CTOR) == [0x8003515C], "2F76C callers")

    print(
        "PASS: 35C84 96w sha ab904f90… type0 vtable jal 361F4; "
        "D2E8&1 skips 3999C; 2F76C 27w stores+5218C/51980/51E64; "
        "E0060 still EXE-resident"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
