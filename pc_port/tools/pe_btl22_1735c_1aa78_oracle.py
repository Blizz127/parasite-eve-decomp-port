#!/usr/bin/env python3
"""PE-BTL22 independent oracle: 0x08 / 1735C / 1AA78 / 1C614.

Pins SHA-1-exact EXE. Does not import production C. Does not invent
bytecode. Does not force type 0 into the 125E0 set. Does not force
3999C or D2E8.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FN_1735C = 0x8001735C
END_1735C = 0x800173F4
SHA_1735C = "a367c31f75fed6c2e7be4ce91d06086f27c3c2253b3c258e86c7ab5c5cab9af8"
FN_1AA78 = 0x8001AA78
END_1AA78 = 0x8001ACE0
SHA_1AA78 = "eaf36cc43c5d5167dba0e6435dc1e2ff261a1cf73d0330fdba87b2a5a55ba935"
FN_1C614 = 0x8001C614
END_1C614 = 0x8001C7DC
SHA_1C614 = "53432d0cdf295adb54161e4ad079c0e75793227405fe9fa63a54d0eb093d942d"


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

    require((END_1735C - FN_1735C) // 4 == 38, "1735C 38w")
    require(window_sha(data, FN_1735C, END_1735C) == SHA_1735C, "1735C sha")
    require(load_u32(data, 0x800910A0 + 0x08 * 4) == FN_1735C, "table[0x08]")
    require(jal_sites(data, FN_1735C) == [], "1735C jalr-only")
    require(jal_target(load_u32(data, 0x80017394)) == 0x80035038, "jal 35038")
    require(load_u32(data, 0x80017388) == 0x8CA5D2F0, "parent D2F0")
    require(load_u32(data, 0x80017390) == 0x24060001, "a2=1")
    require(load_u32(data, 0x8001737C) == 0xA3A20010, "sb type sp+16")
    require(load_u32(data, 0x80017398) == 0xA3A20011, "sb idB sp+17")
    require(load_u32(data, 0x800173AC) == 0xAC430028, "sw *arg2 +0x28")
    require(load_u32(data, 0x800173C0) == 0xAC43002C, "sw *arg3 +0x2C")
    require(jal_target(load_u32(data, 0x800173D4)) == FN_1AA78, "jal 1AA78")
    require(load_u32(data, 0x800173D8) == 0xAC830030, "sw *arg4 +0x30 delay")
    require(load_u32(data, 0x800173DC) == 0x24020001, "v0=1")

    require((END_1AA78 - FN_1AA78) // 4 == 154, "1AA78 154w")
    require(window_sha(data, FN_1AA78, END_1AA78) == SHA_1AA78, "1AA78 sha")
    require(jal_sites(data, FN_1AA78) == [0x80012C9C, 0x800173D4], "1AA78 callers")
    require(load_u32(data, 0x8001AAB0) == 0x30420080, "andi +0x98 0x80")
    require(load_u32(data, 0x8001AABC) == 0x8F82048C, "lw D1FC")
    require(load_u32(data, 0x8001AAD4) == 0x8F820468, "lw D1D8")
    require(jal_target(load_u32(data, 0x8001AB3C)) == FN_1C614, "jal 1C614 arm0")
    require(jal_target(load_u32(data, 0x8001ABD4)) == FN_1C614, "jal 1C614 arm1")
    require(jal_target(load_u32(data, 0x8001AC18)) == 0x8003708C, "jal 3708C")
    require(load_u32(data, 0x8001AB50) == 0xAE9001A4, "sw +0x1A4")
    require(load_u32(data, 0x8001AB5C) == 0xAE9001A8, "sw +0x1A8")

    require((END_1C614 - FN_1C614) // 4 == 114, "1C614 114w")
    require(window_sha(data, FN_1C614, END_1C614) == SHA_1C614, "1C614 sha")
    require(jal_sites(data, FN_1C614) == [
        0x8001AB3C, 0x8001ABD4, 0x8001AE04, 0x8001B368, 0x8001C504, 0x8001C584
    ], "1C614 callers")
    require(load_u32(data, 0x8001C7D4) == 0x03E00008, "1C614 jr")
    require(load_u32(data, 0x8001C7D8) == 0x01001021, "v0=t0")
    require(load_u32(data, 0x8001C7C8) == 0x2DA20003, "sltiu 3 edges")

    print(
        "PASS: 0x08=1735C 38w sha a367c31f… parent D2F0 a2=1 +0x28/2C/30 "
        "jal 35038/1AA78 v0=1; 1AA78 154w sha eaf36cc4… 0x80/D1FC/D1D8 "
        "jal 1C614/3708C; 1C614 114w sha 53432d0c… 3-edge t0"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
