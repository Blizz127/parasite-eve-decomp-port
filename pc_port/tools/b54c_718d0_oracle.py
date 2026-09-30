#!/usr/bin/env python3
"""Phase 6E-B54C independent oracle: func_800718D0 + record-0/1 pack.

Verifies the 29 literal TIM-walker words and the two pack stores at
0x8006AFF8 / 0x8006B02C against the SHA-exact executable. Interprets the
flag-bit-3 walk without importing production C. Does not assign poll=0.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_words

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC = 0x800718D0
FUNC_END = 0x80071944
LOADIMAGE = 0x8007506C
PACK_TPAGE = 0x8006AFF8
PACK_CLUT = 0x8006B02C
PACK_LOOP = 0x8006AFB4
POLL = 0x8006AF54
NEXT_UNRESOLVED = 0x80030894
NEXT_UNRESOLVED_SITE = 0x8006B0AC

WORDS = exe_words(0x800718D0, 29)


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def jal_target(word: int) -> int:
    return (word & 0x3FFFFFF) << 2 | 0x80000000


def load_exe(path: pathlib.Path) -> bytes:
    data = path.read_bytes()
    digest = hashlib.sha1(data).hexdigest()
    require(digest == SHA1, f"exe sha1 {digest}")
    return data


def verify_words(data: bytes) -> None:
    require((FUNC_END - FUNC) // 4 == 29, "29 words")
    require(len(WORDS) == 29, "word table")
    for i, want in enumerate(WORDS):
        got = load_u32(data, FUNC + i * 4)
        require(got == want, f"word {i} @{FUNC + i * 4:#x} {got:#010x}")


def walk_tim(flag: int, clut_bnum: int) -> dict:
    tim = 0x8012F1A0
    s0 = 0
    if flag & 8:
        s0 = tim + 8
        image = s0 + clut_bnum
    else:
        image = tim + 8
    calls = [{"rect": image + 4, "data": image + 12}]
    if s0 != 0:
        calls.append({"rect": s0 + 4, "data": s0 + 12})
    return {"calls": calls, "ret": image + 12}


def pack_record(a: int, b: int, c: int, d: int) -> tuple[int, int]:
    dest1 = ((a & 0x3FF) >> 6) | 0x20 | ((b & 0x100) >> 4) | ((b & 0x200) << 2)
    dest2 = (d << 6) | ((c >> 4) & 0x3F)
    return dest1 & 0xFFFF, dest2 & 0xFFFF


def run_scenarios(data: bytes) -> int:
    n = 0

    # 1. Size, callees, delay slots, return.
    require(load_u32(data, 0x80071910) == 0x0C01D41B, "first 7506C jal")
    require(jal_target(load_u32(data, 0x80071910)) == LOADIMAGE, "first target")
    require(load_u32(data, 0x80071914) == 0x02202821, "a1=s1 delay")
    require(load_u32(data, 0x80071920) == 0x0C01D41B, "second 7506C jal")
    require(jal_target(load_u32(data, 0x80071920)) == LOADIMAGE, "second target")
    require(load_u32(data, 0x80071924) == 0x2605000C, "a1=s0+12 delay")
    require(load_u32(data, 0x8007193C) == 0x03E00008, "jr ra")
    require(load_u32(data, 0x80071940) == 0x00000000, "jr delay nop")
    n += 1

    # 2. flag&8 image-then-CLUT order.
    walked = walk_tim(8, 0x2C)
    require(len(walked["calls"]) == 2, "two LoadImages")
    require(walked["calls"][0]["rect"] == 0x8012F1A0 + 8 + 0x2C + 4, "image RECT")
    require(walked["calls"][0]["data"] == 0x8012F1A0 + 8 + 0x2C + 12, "image data")
    require(walked["calls"][1]["rect"] == 0x8012F1A0 + 12, "CLUT RECT")
    require(walked["calls"][1]["data"] == 0x8012F1A0 + 20, "CLUT data")
    require(walked["ret"] == walked["calls"][0]["data"], "v0=s1")
    n += 1

    # 3. flag==0 skips the CLUT call.
    walked0 = walk_tim(0, 0)
    require(len(walked0["calls"]) == 1, "one LoadImage")
    require(walked0["calls"][0]["rect"] == 0x8012F1A0 + 12, "flag0 RECT")
    require(walked0["ret"] == 0x8012F1A0 + 20, "flag0 return")
    n += 1

    # 4. Atlas RECTs match TXT0-B.
    require(pack_record(0x0140, 0, 0x0140, 0x00FC) == (0x0025, 0x3F14),
            "record0 pack")
    require(pack_record(0x0180, 0, 0x0150, 0x00FC) == (0x0026, 0x3F15),
            "record1 pack")
    n += 1

    # 5. Pack sites and loop bound in 6AD40.
    require(load_u32(data, PACK_TPAGE) == 0xA4231650, "sh tpage 91650(at)")
    require(load_u32(data, PACK_CLUT) == 0xA4231652, "sh clut 91652(at)")
    require(load_u32(data, 0x8006B030) == 0x24A50010, "a1 += 0x10")
    require(load_u32(data, 0x8006B034) == 0x2CA20020, "sltiu a1, 0x20")
    require(load_u32(data, 0x8006B038) == 0x1440FFDE, "bne back to AFB4")
    bne = load_u32(data, 0x8006B038)
    disp = bne & 0xFFFF
    if disp >= 0x8000:
        disp -= 0x10000
    require(0x8006B038 + 4 + disp * 4 == PACK_LOOP, "pack loop head")
    n += 1

    # 6. Live cut is still the poll. poll=0 is not assigned.
    require(load_u32(data, POLL) == 0x0C01B9FA, "jal 6E7E8 at AF54")
    poll_result = "UNRESOLVED"
    require(poll_result == "UNRESOLVED", "do not invent poll=0")
    n += 1

    # 7. After the packs, first unresolved *function* is 30894.
    require(load_u32(data, NEXT_UNRESOLVED_SITE) == 0x0C00C225, "jal 30894")
    require(jal_target(load_u32(data, NEXT_UNRESOLVED_SITE)) == NEXT_UNRESOLVED,
            "30894 target")
    n += 1

    # 8. 718D0 has no MMIO and no 30894 / extra DMA.
    require(all((w >> 16) != 0x1F80 for w in WORDS), "no 1F80 MMIO")
    require(WORDS.count(0x0C01D41B) == 2, "exactly two 7506C jals")
    require(0x0C00C225 not in WORDS, "718D0 does not call 30894")
    n += 1

    require(n == 8, "scenario count")
    return n


def main(argv: list[str]) -> int:
    require(len(argv) == 2, "usage: b54c_718d0_oracle.py executable")
    data = load_exe(pathlib.Path(argv[1]))
    verify_words(data)
    scenarios = run_scenarios(data)
    print(f"B54C-718D0 oracle: PASS ({scenarios}/{scenarios} scenarios; {argv[1]})")
    print(f"  body       {FUNC:#010x}..{FUNC_END:#010x} (29 words)")
    print("  walk       image LoadImage then CLUT LoadImage when flag&8")
    print("  pack       a1=0,0x10 -> (0x0025,0x3F14), (0x0026,0x3F15)")
    print(f"  live cut   jal func_8006E7E8 @ {POLL:#010x}; poll=UNRESOLVED")
    print(f"  next       func_80030894 @ {NEXT_UNRESOLVED_SITE:#010x}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
