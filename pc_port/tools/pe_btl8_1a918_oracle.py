#!/usr/bin/env python3
"""PE-BTL8 independent oracle: func_8001A918 and EXE-resident E0060.

Pins SHA-1-exact EXE. 1A918 is 56 words, SHA-256 c38eb3c9…6496,
zero jal/jalr, sole caller 3F074 @ 0x8003F23C. Rebases D_800B1620
(overlay+0x948; writer 0x8006B8E8, zeroer 0x8006B3F0). +0x18 >
0x80000000 is the already-relocated short path.

0x800E0060 is EXE-resident (taddr 0x80010000, tsize 0x1EE000),
27 words, SHA-256 cfa139eb…750e, zero jal/jalr, sole caller
3F074 @ 0x8003F284. REJECTED: loaded-overlay identity.
Does not import production C. Does not invent 371B0/125E0.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FN = 0x8001A918
FN_END = 0x8001A9F8
WIN_SHA = "c38eb3c91d64397a9902d85b020e5d13d2834c70c329252404c4ffe879056496"
E0 = 0x800E0060
E0_END = 0x800E00CC
E0_SHA = "cfa139eb4767fb5427e7b01f332548a85ce5cf86a05f8de4095d2701f55f750e"


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
    tsize = struct.unpack_from("<I", data, 0x1C)[0]
    require(tsize == 0x1EE000, "tsize")
    require(0x800E00CC <= 0x80010000 + tsize, "E0060 inside EXE")

    require((FN_END - FN) // 4 == 56, "1A918 56 words")
    require(window_sha(data, FN, FN_END) == WIN_SHA, "1A918 sha")
    require(load_u32(data, FN) == 0x3C02800B, "lui 800B")
    require(load_u32(data, FN + 4) == 0x8C421620, "lw D_800B1620")
    require(load_u32(data, 0x8001A930) == 0xAF84048C, "sw gp+0x48C")
    require(load_u32(data, 0x8001A934) == 0x0043102B, "sltu 0x80000000")
    require(load_u32(data, 0x8001A948) == 0xAF820098, "sw gp+0x98 relocated")
    require(load_u32(data, 0x8001A960) == 0xAC830018, "sw rebased +0x18")
    require(load_u32(data, 0x8001A9A0) == 0xAF860098, "sw gp+0x98 table")
    require(load_u32(data, 0x8001A9B4) == 0xAF8400A4, "sw gp+0xA4")
    require(load_u32(data, 0x8001A9BC) == 0xA4A20008, "sh +8 delay")
    require(load_u32(data, 0x8001A9F0) == 0x03E00008, "jr ra")

    jals = []
    jalrs = []
    for va in range(FN, FN_END, 4):
        word = load_u32(data, va)
        if word >> 26 == 3:
            jals.append(jal_target(word))
        if word & 0xFC00003F == 0x00000009:
            jalrs.append(va)
    require(jals == [], f"1A918 jals {jals}")
    require(jalrs == [], f"1A918 jalr {jalrs}")

    jal_word = 0x0C006A46
    sites = []
    for offset in range(0, len(data) - 4, 4):
        if struct.unpack_from("<I", data, offset)[0] == jal_word:
            sites.append(0x80010000 + offset - 0x800)
    require(sites == [0x8003F23C], f"1A918 callers {sites}")

    require(load_u32(data, 0x8006B8E8) == 0xAEC20948, "writer overlay+0x948")
    require(load_u32(data, 0x8006B3F0) == 0xAC400948, "zeroer overlay+0x948")

    require((E0_END - E0) // 4 == 27, "E0060 27 words")
    require(window_sha(data, E0, E0_END) == E0_SHA, "E0060 sha")
    require(load_u32(data, E0) == 0x27BDFFF8, "E0060 prologue")
    require(load_u32(data, 0x800E0068) == 0x8C420E5C, "lw D_800B0E5C")
    require(load_u32(data, 0x800E0070) == 0x846321A4, "lh D_800E21A4")
    require(load_u32(data, 0x800E007C) == 0xAC242800, "sw D_800E2800")
    require(load_u32(data, 0x800E00BC) == 0xA42021A4, "sh E21A4=0")
    e0_jals = [
        jal_target(load_u32(data, va))
        for va in range(E0, E0_END, 4)
        if load_u32(data, va) >> 26 == 3
    ]
    require(e0_jals == [], f"E0060 jals {e0_jals}")
    jal_e0 = 0x0C038018
    e0_sites = []
    for offset in range(0, len(data) - 4, 4):
        if struct.unpack_from("<I", data, offset)[0] == jal_e0:
            e0_sites.append(0x80010000 + offset - 0x800)
    require(e0_sites == [0x8003F284], f"E0060 callers {e0_sites}")

    print(
        "PASS: 1A918 56w sha c38eb3c9… rebase D_800B1620; "
        "E0060 27w EXE-resident sha cfa139eb… not loaded"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
