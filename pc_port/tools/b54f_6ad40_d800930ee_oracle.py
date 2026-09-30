#!/usr/bin/env python3
"""Phase 6E-B54F independent oracle: AF68..B04C issue, 718D0, packs.

Verifies the exclusive 57-word window against the SHA-exact executable.
Does not assign the second poll at 0x8006B04C.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
AF68 = 0x8006AF68
AF88 = 0x8006AF88
AFA8 = 0x8006AFA8
AFF8 = 0x8006AFF8
B02C = 0x8006B02C
B044 = 0x8006B044
B04C = 0x8006B04C
ISSUE = 0x8006E6A8
WALK = 0x800718D0
POLL = 0x8006E7E8
UNRESOLVED = 0x80030894
UNRESOLVED_SITE = 0x8006B0AC


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def load_u16(data: bytes, addr: int) -> int:
    return struct.unpack_from("<H", data, exe_off(addr))[0]


def jal_target(word: int) -> int:
    return (word & 0x3FFFFFF) << 2 | 0x80000000


def bne_target(pc: int, word: int) -> int:
    disp = word & 0xFFFF
    if disp >= 0x8000:
        disp -= 0x10000
    return pc + 4 + disp * 4


def pack_record(a: int, b: int, c: int, d: int) -> tuple[int, int]:
    dest1 = ((a & 0x3FF) >> 6) | 0x20 | ((b & 0x100) >> 4) | ((b & 0x200) << 2)
    dest2 = (d << 6) | ((c >> 4) & 0x3F)
    return dest1 & 0xFFFF, dest2 & 0xFFFF


def load_exe(path: pathlib.Path) -> bytes:
    data = path.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "exe sha1")
    return data


def run_scenarios(data: bytes) -> int:
    n = 0

    # 1. Exclusive window size.
    require((B04C - AF68) // 4 == 57, "57 words")
    require(load_u32(data, AF68) == 0x00008021, "AF68 s0=0")
    require(load_u32(data, B04C) == 0x0C01B9FA, "B04C jal 6E7E8")
    n += 1

    # 2. D_800930EE issue site.
    require(jal_target(load_u32(data, AF88)) == ISSUE, "jal 6E6A8")
    require(load_u32(data, 0x8006AF70) == 0x263130EE, "s1 = &D_800930EE")
    require(load_u32(data, 0x8006AF78) == 0x8EA50180, "a1 = dest+0x180")
    require(load_u16(data, 0x800930EE) == 197, "start 197")
    require(load_u16(data, 0x800930F0) == 200, "end 200")
    n += 1

    # 3. 718D0 walks dest+0x174, not the new issue.
    require(jal_target(load_u32(data, AFA8)) == WALK, "jal 718D0")
    require(load_u32(data, 0x8006AFA4) == 0x8EA40174, "a0 = dest+0x174")
    require(load_u32(data, 0x8006AF9C) == 0x16000029, "bne s0, B044")
    n += 1

    # 4. Record 0 pack from EXE sources.
    require(pack_record(0x0140, 0, 0x0140, 0x00FC) == (0x0025, 0x3F14),
            "record0")
    require(load_u16(data, 0x80091648) == 0x0140, "rec0 A")
    require(load_u16(data, 0x8009164A) == 0x0000, "rec0 B")
    require(load_u16(data, 0x8009164C) == 0x0140, "rec0 C")
    require(load_u16(data, 0x8009164E) == 0x00FC, "rec0 D")
    require(load_u32(data, AFF8) == 0xA4231650, "sh tpage 91650")
    require(load_u32(data, B02C) == 0xA4231652, "sh clut 91652")
    n += 1

    # 5. Record 1 pack from EXE sources — not assumed to be a font.
    require(pack_record(0x0180, 0, 0x0150, 0x00FC) == (0x0026, 0x3F15),
            "record1")
    require(load_u16(data, 0x80091658) == 0x0180, "rec1 A")
    require(load_u16(data, 0x8009165A) == 0x0000, "rec1 B")
    require(load_u16(data, 0x8009165C) == 0x0150, "rec1 C")
    require(load_u16(data, 0x8009165E) == 0x00FC, "rec1 D")
    n += 1

    # 6. Pack loop bound a1=0,0x10; a1<0x20.
    require(load_u32(data, 0x8006AFB0) == 0x00002821, "a1=0")
    require(load_u32(data, 0x8006B030) == 0x24A50010, "a1+=0x10")
    require(load_u32(data, 0x8006B034) == 0x2CA20020, "sltiu a1,0x20")
    require(bne_target(0x8006B038, load_u32(data, 0x8006B038)) == 0x8006AFB4,
            "pack loop head")
    n += 1

    # 7. After packs, s2==1 skips AF6C reissue; next is the live poll.
    require(load_u32(data, 0x8006AF98) == 0x24120001, "s2=1")
    require(load_u32(data, 0x8006B040) == 0x24100001, "s0=1")
    require(bne_target(B044, load_u32(data, B044)) == 0x8006AF6C, "beq AF6C")
    require(jal_target(load_u32(data, B04C)) == POLL, "second poll")
    n += 1

    # 8. Second poll is not this rung; 30894 is later.
    poll_consumed = False
    require(poll_consumed is False, "do not consume B04C")
    require(jal_target(load_u32(data, UNRESOLVED_SITE)) == UNRESOLVED,
            "later 30894")
    n += 1

    require(n == 8, "scenario count")
    return n


def main(argv: list[str]) -> int:
    require(len(argv) == 2, "usage: b54f_6ad40_d800930ee_oracle.py executable")
    data = load_exe(pathlib.Path(argv[1]))
    scenarios = run_scenarios(data)
    print(f"B54F oracle: PASS ({scenarios}/{scenarios} scenarios; {argv[1]})")
    print(f"  window     {AF68:#010x}..{B04C:#010x} (57 words)")
    print("  issue      func_8006E6A8(lba+197, dest+0x180, 3)")
    print("  walk       func_800718D0(dest+0x174)")
    print("  packs      record0 0x0025/0x3F14; record1 0x0026/0x3F15")
    print("  cut        before jal func_8006E7E8 @ 0x8006b04c")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
