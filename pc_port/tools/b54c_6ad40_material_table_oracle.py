#!/usr/bin/env python3
"""Phase 6E-B54C independent func_8006AD40 material-table oracle.

Read-only reconstruction of the retail prefix beginning at 0x8006AE68.
Verifies literal words against the SHA-1-exact executable, packs the
static D_80091648 records, walks the captured ABADC06C payload, and
stops before inventing a func_8006E7E8 poll result.

Imports no production C. Models no DMA, IRQ, callback, display, or CD
progress.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_words


EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"

FUNC_START = 0x8006AD40
AUDIT_START = 0x8006AE68
POLL_JAL = 0x8006AF54
FIRST_UNRESOLVED_IF_POLL0 = 0x8006AFA8
FUNC_8006E498 = 0x8006E498
FUNC_8007506C = 0x8007506C
FUNC_8006E7E8 = 0x8006E7E8
FUNC_800718D0 = 0x800718D0
FUNC_800718D0_END = 0x80071944
D_80091648 = 0x80091648

# 59 words: 0x8006AE68 .. 0x8006AF54 exclusive.
WORDS = exe_words(0x8006AE68, 59)
POLL_WORD = 0x0C01B9FA  # 0x8006AF54 jal func_8006E7E8; not consumed

CANON_BASE = 0x801229A0
CANON_S2 = 1
CANON_S4 = 0x801229A0
CANON_KEY = 0xABADC06C
CANON_DIR_HDR = 0x0040B5AC
CANON_LOOKUP = 0x801229A8
CANON_LOOKUP_SIZE = 0x00007F0C
CANON_RECT = (448, 0, 64, 254)
CANON_TERMINATOR = 0x8012A8B4
CANON_CD_BUSY = 0x01004000
CANON_D800B0CD8 = 0x41004003
CANON_BUF174 = 0x8012F1A0

# Captured / disc-extracted ABADC06C payload walk. Not invented.
CANON_WALK = (
    {
        "s0": CANON_LOOKUP,
        "size": CANON_LOOKUP_SIZE,
        "rect": CANON_RECT,
        "a0": CANON_LOOKUP + 4,
        "a1": CANON_LOOKUP + 0xC,
    },
)


def require(cond: bool, message: str) -> None:
    if not cond:
        raise SystemExit(f"FATAL: {message}")


def u16(value: int) -> int:
    return value & 0xFFFF


def u32(value: int) -> int:
    return value & 0xFFFFFFFF


def jal_target(word: int) -> int:
    return 0x80000000 | ((word & 0x03FFFFFF) << 2)


def load_exe(path: pathlib.Path) -> bytes:
    data = path.read_bytes()
    got = hashlib.sha1(data).hexdigest()
    require(got == EXE_SHA1, f"{path} SHA-1 {got} != {EXE_SHA1}")
    require(data[:8] == b"PS-X EXE", "not a PS-X EXE")
    return data


def text_addr(data: bytes) -> int:
    return struct.unpack_from("<I", data, 0x18)[0]


def file_off(data: bytes, pc: int) -> int:
    return pc - text_addr(data) + 0x800


def load_u16(data: bytes, pc: int) -> int:
    return struct.unpack_from("<H", data, file_off(data, pc))[0]


def load_u32(data: bytes, pc: int) -> int:
    return struct.unpack_from("<I", data, file_off(data, pc))[0]


def verify_words(data: bytes) -> None:
    require(len(WORDS) == (POLL_JAL - AUDIT_START) // 4, "prefix word count")
    require((POLL_JAL - AUDIT_START) // 4 == 59, "59-word audited prefix")
    for i, expected in enumerate(WORDS):
        actual = load_u32(data, AUDIT_START + i * 4)
        require(actual == expected,
                f"word at {AUDIT_START + i * 4:#010x}: "
                f"{actual:#010x} != {expected:#010x}")
    require(load_u32(data, POLL_JAL) == POLL_WORD, "poll jal changed")
    require(jal_target(WORDS[37]) == FUNC_8006E498, "lookup jal")
    require(WORDS[38] == 0x34A5C06C, "key delay ori")
    require(jal_target(WORDS[44]) == FUNC_8007506C, "LoadImage jal")
    require(jal_target(POLL_WORD) == FUNC_8006E7E8, "poll target")
    require(WORDS[0] == 0x24050020, "a1 = 0x20")
    require(WORDS[34] == 0x1440FFDE, "packing back-edge")
    require(WORDS[57] == 0x1242FFA2, "s2==-1 back-edge encoding")
    require(WORDS[58] == 0x00000000, "nop delay before poll")


def pack_dest1(src_a: int, src_b: int) -> int:
    v0 = ((src_a & 0x3FF) >> 6) | 0x20
    v1 = (src_b & 0x100) >> 4
    v1 |= v0
    v1 |= (src_b & 0x200) << 2
    return u16(v1)


def pack_dest2(src_c: int, src_d: int) -> int:
    return u16((src_d << 6) | ((src_c >> 4) & 0x3F))


def run_pack_loop(slots: dict[int, tuple[int, int, int, int]]) -> dict:
    """Execute AE68..AEF8 packing. slots keyed by a1."""
    writes: list[tuple[int, int, int]] = []
    a1 = 0x20
    iterations = 0
    while True:
        src_a, src_b, src_c, src_d = slots[a1]
        dest1 = pack_dest1(src_a, src_b)
        dest2 = pack_dest2(src_c, src_d)
        writes.append((D_80091648 + 8 + a1, dest1, D_80091648 + 0xA + a1, dest2))
        a1 = u32(a1 + 0x10)
        iterations += 1
        if not (a1 < 0x40):
            break
    return {
        "iterations": iterations,
        "final_a1": a1,
        "writes": writes,
        "a0": CANON_S4,
    }


def decode_rect_words(word0: int, word1: int) -> tuple[int, int, int, int]:
    def sx(half: int) -> int:
        half &= 0xFFFF
        return half - 0x10000 if half >= 0x8000 else half

    return (sx(word0), sx(word0 >> 16), sx(word1), sx(word1 >> 16))


def run_lookup(base: int, key: int, hdr: int, entries: list[tuple[int, int]]) -> int:
    """entries: list of (offset_word, key_word) in directory order."""
    count = u32(hdr) >> 22
    if count == 0:
        return 0
    for i, (off, ent_key) in enumerate(entries):
        if i >= count:
            break
        if u32(ent_key) == u32(key):
            return u32(base + (u32(off) & 0xFFFFFF))
    return 0


def run_walk(records: list[dict], first: int) -> dict:
    s0 = first
    calls: list[dict] = []
    for rec in records:
        require(s0 == rec["s0"], f"walk s0 {s0:#x} != {rec['s0']:#x}")
        size = rec["size"]
        if size == 0:
            break
        calls.append({
            "a0": s0 + 4,
            "a1": s0 + 0xC,
            "rect": rec["rect"],
            "size": size,
        })
        s0 = u32(s0 + (u32(size) & ~3))
    return {"calls": calls, "terminator": s0, "final_s0_after_force": 1}


def run_scenarios(data: bytes) -> int:
    n = 0

    # 1. Geometry, delay slots, jal targets, first excluded poll word.
    require(AUDIT_START - FUNC_START == 0x128, "audit start offset")
    require(POLL_JAL - AUDIT_START == 0xEC, "59-word span")
    require(WORDS[35] == 0x02802021, "delay a0=s4")
    require(WORDS[36] == 0x3C05ABAD and WORDS[38] == 0x34A5C06C, "key split")
    require(load_u32(data, POLL_JAL + 4) == 0x00000000, "poll delay nop")
    n += 1

    # 2. Static D_80091648 records match the captured B54A / EXE image.
    slots: dict[int, tuple[int, int, int, int]] = {}
    expected = {
        0x00: (0x0140, 0x0000, 0x0140, 0x00FC),
        0x10: (0x0180, 0x0000, 0x0150, 0x00FC),
        0x20: (0x0110, 0x01D4, 0x0130, 0x01DE),
        0x30: (0x011C, 0x01D4, 0x0130, 0x01DD),
    }
    for a1, exp in expected.items():
        got = (
            load_u16(data, D_80091648 + a1),
            load_u16(data, D_80091648 + 2 + a1),
            load_u16(data, D_80091648 + 4 + a1),
            load_u16(data, D_80091648 + 6 + a1),
        )
        require(got == exp, f"slot a1={a1:#x} {got} != {exp}")
        dest1 = load_u16(data, D_80091648 + 8 + a1)
        dest2 = load_u16(data, D_80091648 + 0xA + a1)
        require(dest1 == 0 and dest2 == 0, f"packed fields already set at {a1:#x}")
        slots[a1] = got
    n += 1

    # 3. Packing loop: a1=0x20,0x30 only; exact dest halfwords.
    packed = run_pack_loop(slots)
    require(packed["iterations"] == 2, "two packing iterations")
    require(packed["final_a1"] == 0x40, "a1 exits at 0x40")
    require(packed["a0"] == CANON_S4, "lookup a0")
    require(packed["writes"][0] == (0x80091670, 0x0034, 0x80091672, 0x7793),
            "record 2 packed fields")
    require(packed["writes"][1] == (0x80091680, 0x0034, 0x80091682, 0x7753),
            "record 3 packed fields")
    written = {w[0] for w in packed["writes"]} | {w[2] for w in packed["writes"]}
    require(0x80091650 not in written and 0x80091652 not in written,
            "record 0 packed fields must stay unwritten")
    require(0x80091660 not in written and 0x80091662 not in written,
            "record 1 packed fields must stay unwritten")
    n += 1

    # 4. func_8006E498: canonical key hits entry 0, result 0x801229A8.
    result = run_lookup(
        CANON_BASE, CANON_KEY, CANON_DIR_HDR, [(0x00000008, CANON_KEY)]
    )
    require((CANON_DIR_HDR >> 22) == 1, "directory count")
    require((CANON_DIR_HDR & 0x3FFFFF) == 0xB5AC, "directory offset")
    require(result == CANON_LOOKUP, "lookup result")
    require(run_lookup(CANON_BASE, 0xDEADBEEF, CANON_DIR_HDR,
                       [(0x00000008, CANON_KEY)]) == 0,
            "unknown key must miss")
    n += 1

    # 5. func_8007506C walk: one RECT, inline pixels, zero terminator.
    require(CANON_RECT[2] * CANON_RECT[3] * 2 == CANON_LOOKUP_SIZE - 0xC,
            "size = header + pixels")
    require(decode_rect_words(0x000001C0, 0x00FE0040) == CANON_RECT,
            "RECT words")
    require(CANON_LOOKUP + (CANON_LOOKUP_SIZE & ~3) == CANON_TERMINATOR,
            "terminator address")
    walk = run_walk(list(CANON_WALK), CANON_LOOKUP)
    require(len(walk["calls"]) == 1, "one LoadImage")
    require(walk["calls"][0]["a0"] == CANON_LOOKUP + 4, "rect pointer")
    require(walk["calls"][0]["a1"] == CANON_LOOKUP + 0xC, "pixel pointer")
    require(walk["calls"][0]["rect"] == CANON_RECT, "RECT")
    require(walk["terminator"] == CANON_TERMINATOR, "terminator")
    require(walk["final_s0_after_force"] == 1, "s0 forced to 1")
    n += 1

    # 6. s2==1 does not take the -1 back-edge; s2==-1 would.
    require(CANON_S2 != -1, "canonical s2 is not -1")
    require(CANON_S2 == 1, "canonical s2 is the post-issue 1")
    require((CANON_D800B0CD8 & CANON_CD_BUSY) == CANON_CD_BUSY,
            "busy bits still set; do not invent poll=0")
    n += 1

    # 7. Poll is not invented. 718D0 is not the next dependency.
    require(POLL_JAL == 0x8006AF54, "poll site")
    require(FIRST_UNRESOLVED_IF_POLL0 == 0x8006AFA8, "718D0 site")
    require(load_u32(data, FIRST_UNRESOLVED_IF_POLL0) == 0x0C01C634,
            "718D0 jal encoding")
    require(jal_target(load_u32(data, FIRST_UNRESOLVED_IF_POLL0))
            == FUNC_800718D0, "718D0 target")
    require((FUNC_800718D0_END - FUNC_800718D0) // 4 == 29, "718D0 size")
    require(load_u32(data, 0x80071910) == 0x0C01D41B, "718D0 first 7506C")
    require(load_u32(data, 0x80071920) == 0x0C01D41B, "718D0 second 7506C")
    require(load_u32(data, 0x8007193C) == 0x03E00008, "718D0 jr")
    # Reachability requires poll==0, which this oracle refuses to assign.
    poll_result = "UNRESOLVED"
    require(poll_result == "UNRESOLVED", "poll must stay unresolved")
    n += 1

    # 8. Recommended cut is AF54, not AFA8. No 1F80xxxx in the prefix.
    require(all((w >> 16) != 0x1F80 for w in WORDS),
            "suffix-local MMIO in packing/walk prefix")
    require(WORDS.count(0xA4231650) == 1, "one dest1 store encoding")
    require(WORDS.count(0xA4231652) == 1, "one dest2 store encoding")
    require(POLL_JAL != FIRST_UNRESOLVED_IF_POLL0,
            "poll cut is not the 718D0 site")
    require(CANON_BUF174 == 0x8012F1A0, "718D0 a0 if later reached")
    n += 1

    require(n == 8, "scenario count")
    return n


def main(argv: list[str]) -> int:
    require(len(argv) == 2, "usage: b54c_6ad40_material_table_oracle.py executable")
    data = load_exe(pathlib.Path(argv[1]))
    verify_words(data)
    scenarios = run_scenarios(data)
    print(f"B54C oracle: PASS ({scenarios}/{scenarios} scenarios; {argv[1]})")
    print(f"  audit      {AUDIT_START:#010x}..{POLL_JAL:#010x} "
          f"({len(WORDS)} words)")
    print("  packing    a1=0x20,0x30 -> (0x0034,0x7793), (0x0034,0x7753)")
    print(f"  lookup     func_8006E498({CANON_S4:#010x}, {CANON_KEY:#010x}) "
          f"-> {CANON_LOOKUP:#010x}")
    print("  walk       1 x func_8007506C RECT{448,0,64,254}; "
          f"term {CANON_TERMINATOR:#010x}")
    print(f"  poll       jal func_8006E7E8 @ {POLL_JAL:#010x}; "
          "canonical_poll_result=UNRESOLVED")
    print("  718D0      not reached; required poll==0 is not assigned")
    print(f"  recommended cut: {POLL_JAL:#010x}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
