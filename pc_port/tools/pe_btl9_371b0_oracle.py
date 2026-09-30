#!/usr/bin/env python3
"""PE-BTL9 independent oracle: func_800371B0, 125E0, 12574.

Pins SHA-1-exact EXE. 371B0 is 169 words, SHA-256 83a0b015…ba366.
3F074 selects D_800B162C when overlay[0] bit 0x40000000 is set,
else D_800B1628, then jal 371B0 / 125E0 / E0060. 371B0 stores a0
at 0x120($gp) and builds the 320x54 y=170 TILE. 12574 (27w) is
the sole publisher of 0x94($gp). 125E0 (35w) DrawSync(0) then
walks lbu(**CE04) descriptors into 35038(desc+1+i*2, 0, 1).

Does not import production C. Does not mark M2. Does not invent
overlay-loading machinery.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FN = 0x800371B0
FN_END = 0x80037454
WIN_SHA = "83a0b015e3842420c437c0a79e556e43c88c24f6699f922283c3de28cd5ba366"
E0 = 0x800125E0
E0_END = 0x8001266C
E0_SHA = "7c30399dbb3245dd902a66aaf32db83d143b81f53ed0e7a862101eb58881434c"
P74 = 0x80012574
P74_END = 0x800125E0
P74_SHA = "bf4a0017e68e1e0a390d35a155ff49b5d6117e5d566d6d10b846151e8943e124"
CTOR = 0x80035038
CTOR_END = 0x80035558


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
    require(FN_END <= 0x80010000 + tsize, "371B0 inside EXE")
    require(E0_END <= 0x80010000 + tsize, "125E0 inside EXE")
    require(CTOR_END <= 0x80010000 + tsize, "35038 inside EXE")

    require((FN_END - FN) // 4 == 169, "371B0 169 words")
    require(window_sha(data, FN, FN_END) == WIN_SHA, "371B0 sha")
    require(load_u32(data, FN) == 0x27BDFFC8, "371B0 prologue")
    require(load_u32(data, 0x800372DC) == 0xAF840120, "sw a0, gp+0x120")
    require(load_u32(data, 0x800372D0) == 0xA3800130, "sb 0 gp+0x130")
    require(load_u32(data, 0x800372D4) == 0xA3800160, "sb 0 gp+0x160")
    require(load_u32(data, 0x800372D8) == 0xAF800164, "sw 0 gp+0x164")
    require(load_u32(data, 0x80037224) == 0xA020CEB4, "sb 0 D_800BCEB4")
    require(load_u32(data, 0x8003724C) == 0xA020CEA8, "sb 0 D_800BCEA8")
    require(load_u32(data, 0x800373EC) == 0x24020140, "li 0x140")
    require(load_u32(data, 0x800373F0) == 0xA482000C, "sh w=320")
    require(load_u32(data, 0x800373F4) == 0x24020036, "li 0x36")
    require(load_u32(data, 0x800373F8) == 0xA482000E, "sh h=54")
    require(load_u32(data, 0x800373FC) == 0x240200AA, "li 0xAA")
    require(load_u32(data, 0x80037408) == 0xA482000A, "sh y=170 delay")
    require(load_u32(data, 0x8003744C) == 0x03E00008, "371B0 jr")

    jals = [
        jal_target(load_u32(data, va))
        for va in range(FN, FN_END, 4)
        if load_u32(data, va) >> 26 == 3
    ]
    require(
        jals
        == [
            0x80077C84,
            0x80077C04,
            0x80077CB4,
            0x800719E4,
            0x80077B34,
            0x80077A64,
            0x80077C84,
            0x80077C44,
            0x80077CB4,
            0x800719E4,
            0x80077B04,
        ],
        f"371B0 jals {jals}",
    )
    require(jal_sites(data, FN) == [0x8003F274, 0x80069C2C], "371B0 callers")

    require(load_u32(data, 0x8003F248) == 0x8C420CD8, "3F074 lw overlay[0]")
    require(load_u32(data, 0x8003F24C) == 0x3C034000, "lui 0x4000")
    require(load_u32(data, 0x8003F260) == 0x8C84162C, "lw D_800B162C")
    require(load_u32(data, 0x8003F270) == 0x8C841628, "lw D_800B1628")
    require(jal_target(load_u32(data, 0x8003F274)) == FN, "jal 371B0")
    require(jal_target(load_u32(data, 0x8003F27C)) == E0, "jal 125E0")
    require(jal_target(load_u32(data, 0x8003F284)) == 0x800E0060, "jal E0060")
    require(load_u32(data, 0x8006B94C) == 0xAC620950, "writer overlay+0x950")
    require(load_u32(data, 0x8005286C) == 0x3C054000, "527C8 lui 0x4000")
    require(load_u32(data, 0x80052870) == 0x00651825, "527C8 or language bit")

    require((E0_END - E0) // 4 == 35, "125E0 35 words")
    require(window_sha(data, E0, E0_END) == E0_SHA, "125E0 sha")
    require(jal_target(load_u32(data, 0x800125F0)) == 0x80074DC0, "DrawSync")
    require(load_u32(data, 0x800125F8) == 0x8F840094, "lw gp+0x94")
    require(jal_target(load_u32(data, 0x80012628)) == CTOR, "jal 35038")
    require(load_u32(data, 0x80012624) == 0x24060001, "a2=1")
    require(load_u32(data, 0x8001261C) == 0x00002821, "a1=0")
    require(jal_sites(data, E0) == [0x8003F27C], "125E0 callers")

    require((P74_END - P74) // 4 == 27, "12574 27 words")
    require(window_sha(data, P74, P74_END) == P74_SHA, "12574 sha")
    require(load_u32(data, 0x8001257C) == 0xAF840094, "sw a0, gp+0x94")
    require(jal_sites(data, P74) == [0x8006B8BC], "12574 callers")
    require(load_u32(data, 0x8006B8C4) == 0xAEC20944, "sw v0 overlay+0x944")

    require((CTOR_END - CTOR) // 4 == 328, "35038 328 words")
    require(load_u32(data, CTOR) == 0x8F82053C, "lw gp+0x53C freelist")
    require(load_u32(data, 0x80035058) == 0x14400003, "bnez freelist")
    require(load_u32(data, 0x80035064) == 0x00001021, "v0=0 empty")
    require(
        jal_sites(data, CTOR) == [0x80012628, 0x80016C1C, 0x80017394],
        "35038 callers",
    )

    require((0x80035038 - 0x80034FC4) // 4 == 29, "34FC4 29 words")
    require(load_u32(data, 0x80034FE0) == 0xAF82053C, "34FC4 sw gp+0x53C")
    require(load_u32(data, 0x80034FCC) == 0x2442EA90, "34FC4 D_800BEA90")
    require(jal_sites(data, 0x80034FC4) == [0x8003F0B0], "34FC4 callers")

    require((0x80012700 - 0x8001266C) // 4 == 37, "1266C 37 words")
    require(
        window_sha(data, 0x8001266C, 0x80012700)
        == "34d11f74f5167d70c1e204cc11ba59877b0328cf69c13295cb11beebc792645d",
        "1266C sha",
    )
    require(load_u32(data, 0x80012680) == 0xAF82008C, "1266C sw gp+0x8C")
    require(load_u32(data, 0x80012674) == 0x2442D310, "1266C D_8009D310")
    require(jal_sites(data, 0x8001266C) == [0x8003F0B8], "1266C callers")
    require(jal_target(load_u32(data, 0x8003526C)) == 0x80012700, "35038 jal 12700")
    require(load_u32(data, 0x80035078) == 0x10A0000B, "35038 beq a1,0")
    require(load_u32(data, 0x80035294) == 0x106000A3, "35038 beq +0x1AC,0")
    require(load_u32(data, 0x8003552C) == 0x344200E0, "35038 ori +0x98 0xE0")
    require(load_u32(data, 0x8006B8C4) == 0xAEC20944, "12574 v0 -> +0x944")

    print(
        "PASS: 371B0 169w sha 83a0b015… window 320x54 y=170 sw gp+0x120; "
        "3F074 bit 0x40000000 -> 162C; 12574 publishes gp+0x94; "
        "125E0 35w DrawSync+35038(a1=0,a2=1); all EXE-resident"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
