#!/usr/bin/env python3
"""PE-BTL6 independent oracle: 3D050 remainder cuts + live 6698C/3C5D8.

Checks SHA-1-exact EXE windows without importing production C.
Does not claim jal 794C4, 3D834 callees, andi 0xFC, or 0x55 done.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"

FN_3D050 = 0x8003D050
FN_3D050_END = 0x8003D834
PTR14 = 0x8003D36C
PTR14_END = 0x8003D394
POST = 0x8003D5D8
POST_END = 0x8003D750
EPI = 0x8003D76C
JAL_3D94C = 0x8003D5D0
JAL_794C4 = 0x8003D750
JAL_3C5D8 = 0x8003D764
FN_3C5D8 = 0x8003C5D8
FN_3C5D8_END = 0x8003C638
FN_6698C = 0x8006698C
FN_6698C_LIVE_END = 0x80066B60
FN_6698C_WINDOW_END = 0x80066CE8
FN_3D834 = 0x8003D834
FN_3D834_END = 0x8003D94C
FN_3D94C = 0x8003D94C
FN_3DFD8 = 0x8003DFD8
FN_3DFD8_LIVE_END = 0x8003E0A4

WIN_3D050 = "50b5ff7516a04dd203ee5fdba30510547f4e47bb6e2b8988b8bde73bec00809a"
WIN_PTR14 = "825bccd27b2e54515b540f052d5b478a791ee5fd39f7cb0abc5657fa249c14c6"
WIN_POST = "93d42cd9d265c528378c66ce3e2f3a2f79e10697cfe154ed6ea2dd8c7c1b1b41"
WIN_EPI = "cc904e701f395d47bfd1c4cbe5a0c703bf859b4364fdd5f88fe91117d6f07833"
WIN_3C5D8 = "237a6038c83912cb06091b4027da4a5d4234b5f71150499eb0133fc474cf4e8c"
WIN_6698C_LIVE = "71478d80954afcad2616494a803d5cc26d677eccafcdfd0e5bf085b060f11a45"
WIN_6698C_WIN = "3c24408d3f33c94fc1f554c15ec0df6a06fb98091c0be666844bf22095e434e7"
WIN_3D834 = "520529b07ee5f4234242a5911c06895083fbf2955c6c58a54ae5a7d8a317f7ad"
WIN_3DFD8_LIVE = "c30623182ab2e16e2bcb5de7a82d8ccf7dd39446c84a82e117960e2f37d9e881"
FN_794C4 = 0x800794C4
FN_794C4_LEAF_END = 0x80079750
WIN_794C4_LEAF = "19a788c448e9f29638e673adaa9be46de34a81b53a99fbb389b4833a7a77ac7c"


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

    require((FN_3D050_END - FN_3D050) // 4 == 505, "3D050 505 words")
    require(window_sha(data, FN_3D050, FN_3D050_END) == WIN_3D050, "3D050 sha")
    require(load_u32(data, 0x8003D0D0) == 0xAE050010, "prefix sw +0x10")
    require(load_u32(data, 0x8003D0E8) == 0x13000093, "beq t8 skip fill")
    require(load_u32(data, 0x8003D338) == 0x95830008, "t8==0 lhu obj+8")
    require(load_u32(data, PTR14) == 0xAE050014, "ptr14 sw +0x14")
    require(load_u32(data, 0x8003D374) == 0xAE050018, "ptr14 sw +0x18")
    require(load_u32(data, 0x8003D388) == 0xAE05001C, "ptr14 sw +0x1C")
    require(load_u32(data, 0x8003D390) == 0xAE050020, "ptr14 sw +0x20")
    require((PTR14_END - PTR14) // 4 == 10, "ptr14 10 words")
    require(window_sha(data, PTR14, PTR14_END) == WIN_PTR14, "ptr14 sha")

    require(load_u32(data, 0x8003D5A8) == 0x1840000B, "blez skip 3D94C")
    require(jal_target(load_u32(data, JAL_3D94C)) == FN_3D94C, "jal 3D94C")
    require(load_u32(data, POST) == 0x8E020014, "post lw +0x14")
    require(load_u32(data, 0x8003D5E8) == 0xA6020070, "post sh +0x70")
    require(load_u32(data, 0x8003D5FC) == 0xA6020072, "post sh +0x72")
    require(load_u32(data, 0x8003D5F8) == 0xAE060080, "post sw +0x80")
    require(load_u32(data, 0x8003D6B0) == 0xAE060084, "post sw +0x84")
    require(load_u32(data, 0x8003D738) == 0xA600002C, "post sh +0x2C")
    require(load_u32(data, 0x8003D744) == 0xA6020032, "post sh +0x32 = 1")
    require((POST_END - POST) // 4 == 94, "post 94 words")
    require(window_sha(data, POST, POST_END) == WIN_POST, "post sha")
    require(jal_target(load_u32(data, JAL_794C4)) == 0x800794C4, "jal 794C4")
    require(jal_target(load_u32(data, JAL_3C5D8)) == FN_3C5D8, "jal 3C5D8")
    require(load_u32(data, 0x8003D768) == 0xA202008C, "delay sb -1 +0x8C")

    require((FN_3C5D8_END - FN_3C5D8) // 4 == 24, "3C5D8 24 words")
    require(window_sha(data, FN_3C5D8, FN_3C5D8_END) == WIN_3C5D8, "3C5D8 sha")
    require(load_u32(data, FN_3C5D8) == 0x00A03021, "3C5D8 move a2,a1")
    require(load_u32(data, 0x8003C624) == 0xA086008D, "3C5D8 sb +0x8D")
    require(load_u32(data, 0x8003C634) == 0xA0830093, "3C5D8 sb +0x93")

    require(load_u32(data, EPI) == 0x24020080, "epi li 0x80")
    require(load_u32(data, 0x8003D770) == 0xA2020090, "epi sb +0x90")
    require(load_u32(data, 0x8003D780) == 0xA202009E, "epi sb +0x9E")
    require(load_u32(data, 0x8003D7A0) == 0xA202009F, "epi sb CDDC +0x9F")
    require(load_u32(data, 0x8003D7FC) == 0xAE0000B0, "epi sw +0xB0")
    require(window_sha(data, EPI, FN_3D050_END) == WIN_EPI, "epi sha")

    jals = []
    for va in range(FN_3D050, FN_3D050_END, 4):
        word = load_u32(data, va)
        if word >> 26 == 3:
            jals.append((va, jal_target(word)))
    require(jals == [
        (JAL_3D94C, FN_3D94C),
        (JAL_794C4, 0x800794C4),
        (JAL_3C5D8, FN_3C5D8),
    ], f"3D050 jals {jals}")

    require((FN_6698C_LIVE_END - FN_6698C) // 4 == 117, "6698C live 117")
    require((FN_6698C_WINDOW_END - FN_6698C) // 4 == 215, "6698C window 215")
    require(window_sha(data, FN_6698C, FN_6698C_LIVE_END) == WIN_6698C_LIVE,
            "6698C live sha")
    require(window_sha(data, FN_6698C, FN_6698C_WINDOW_END) == WIN_6698C_WIN,
            "6698C window sha")
    require(load_u32(data, FN_6698C) == 0x24021000, "6698C li 4096")
    require(load_u32(data, 0x80066994) == 0xA420EA40, "6698C sh 0 EA40")
    require(load_u32(data, 0x80066B58) == 0x03E00008, "6698C first jr")
    live_jals = []
    for va in range(FN_6698C, FN_6698C_LIVE_END, 4):
        word = load_u32(data, va)
        if word >> 26 == 3:
            live_jals.append(jal_target(word))
    require(live_jals == [], f"6698C live jals {live_jals}")

    require((FN_3D834_END - FN_3D834) // 4 == 70, "3D834 70 words")
    require(window_sha(data, FN_3D834, FN_3D834_END) == WIN_3D834, "3D834 sha")
    require(load_u32(data, 0x8003D85C) == 0x12400013, "3D834 beq a1==0")
    require(load_u32(data, 0x8003D944) == 0x03E00008, "3D834 jr")
    require(FN_3D94C == FN_3D834_END, "3D94C follows 3D834")
    d834_jals = []
    for va in range(FN_3D834, FN_3D834_END, 4):
        word = load_u32(data, va)
        if word >> 26 == 3:
            d834_jals.append(jal_target(word))
    require(d834_jals == [
        0x8003DFD8, 0x8003DFD8, 0x80039B74, 0x8003A088,
        0x8003DFD8, 0x8003B97C, 0x8003BCE0, 0x8003BCE0,
    ], f"3D834 jals {d834_jals}")
    require(load_u32(data, 0x8003D8AC) == 0x0C00E822, "a1==0 first jal 3A088")

    require((FN_3DFD8_LIVE_END - FN_3DFD8) // 4 == 51, "3DFD8 live 51")
    require(window_sha(data, FN_3DFD8, FN_3DFD8_LIVE_END) == WIN_3DFD8_LIVE,
            "3DFD8 live sha")
    require(load_u32(data, FN_3DFD8) == 0x27BDFFF8, "3DFD8 addiu sp")
    require(load_u32(data, 0x8003DFEC) == 0x18C0002A, "3DFD8 blez count")
    require(load_u32(data, 0x8003E09C) == 0x03E00008, "3DFD8 first jr")
    dfd8_jals = []
    for va in range(FN_3DFD8, FN_3DFD8_LIVE_END, 4):
        word = load_u32(data, va)
        if word >> 26 == 3:
            dfd8_jals.append(jal_target(word))
    require(dfd8_jals == [], f"3DFD8 live jals {dfd8_jals}")

    require((FN_794C4_LEAF_END - FN_794C4) // 4 == 163, "794C4 leaf 163")
    require(window_sha(data, FN_794C4, FN_794C4_LEAF_END) == WIN_794C4_LEAF,
            "794C4 leaf sha")
    require(load_u32(data, FN_794C4) == 0x848F0000, "794C4 lh a0")
    require(load_u32(data, 0x800794EC) == 0x8F3966EC, "794C4 lw sincos")
    require(load_u32(data, 0x80079748) == 0x03E00008, "794C4 first jr")
    leaf_jals = []
    for va in range(FN_794C4, FN_794C4_LEAF_END, 4):
        word = load_u32(data, va)
        if word >> 26 == 3:
            leaf_jals.append(jal_target(word))
    require(leaf_jals == [], f"794C4 leaf jals {leaf_jals}")

    print(
        "PASS: 3D050 ptr14/post-3D94C-skip/epilogue exclusive cuts; "
        "3C5D8 24w; 6698C live 117w first jr; 3D834 70w a1==0; "
        "3DFD8 live 51w copy; 794C4 first leaf 163w RotMatrix"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
