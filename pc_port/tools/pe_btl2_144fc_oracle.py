#!/usr/bin/env python3
"""PE-BTL2 independent oracle: opcode 0x55 func_800144FC → jal 0x80029810.

Verifies the SHA-1-exact handler, D_800B0CD8+0xF4 jump table, sole
jal of func_80029810, and that 29810 jals the HP cut with a0=0.
Does not import production C. Does not claim 0x55 completion.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_words

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC = 0x800144FC
FUNC_END = 0x80014694  # exclusive; next leaf
TABLE = 0x800910A0
JT = 0x800100A0
GP = 0x8009CD70
JAL_29810 = 0x8001461C
INIT = 0x80029810
HP_JAL = 0x80029910
HP_DELAY = 0x80029914
HP_CUT = 0x800293F4

WORDS = exe_words(0x800144FC, 102)


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def jal_target(word: int) -> int:
    return ((word & 0x3FFFFFF) << 2) | 0x80000000


def load_exe(path: pathlib.Path) -> bytes:
    data = path.read_bytes()
    digest = hashlib.sha1(data).hexdigest()
    require(digest == SHA1, f"exe sha1 {digest}")
    return data


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = load_exe(exe)
    require((FUNC_END - FUNC) // 4 == 102, "102 words")
    require(len(WORDS) == 102, "word table")
    for i, want in enumerate(WORDS):
        got = load_u32(data, FUNC + i * 4)
        require(got == want, f"word {i} @{FUNC + i * 4:#x} {got:#010x}")
    require(load_u32(data, TABLE + 0x55 * 4) == FUNC, "D_800910A0[0x55]")
    require(WORDS[4] == 0x3C10800B and WORDS[5] == 0x26100CD8, "s0=D_800B0CD8")
    require((WORDS[7] >> 26) == 0x24 and (WORDS[7] & 0xFFFF) == 0xF4,
            "lbu +0xF4")
    require(WORDS[9] == 0x2C62003C, "sltiu 0x3C")
    require(WORDS[14] == 0x8C2200A0, "lw JT 0x800100A0")
    require(load_u32(data, JT + 0) == 0x80014544, "state 0")
    for i in range(1, 0x37):
        require(load_u32(data, JT + i * 4) == 0x80014658, f"state {i:#x} default")
    require(load_u32(data, JT + 0x37 * 4) == 0x80014570, "state 0x37")
    require(load_u32(data, JT + 0x38 * 4) == 0x8001459C, "state 0x38")
    require(load_u32(data, JT + 0x39 * 4) == 0x800145DC, "state 0x39")
    require(load_u32(data, JT + 0x3A * 4) == 0x800145F8, "state 0x3A")
    require(load_u32(data, JT + 0x3B * 4) == 0x80014630, "state 0x3B")
    require(jal_target(load_u32(data, JAL_29810)) == INIT, "jal 29810")
    require(WORDS[72] == 0x0C00A604, "encoded jal 29810")
    jal = 0x0C000000 | ((INIT & 0x0FFFFFFF) >> 2)
    hits = [
        0x80010000 + (i - 0x800)
        for i in range(0, len(data) - 4, 4)
        if struct.unpack_from("<I", data, i)[0] == jal
    ]
    require(hits == [JAL_29810], f"sole jal 29810 {hits}")
    require(WORDS[89] == 0x00001021, "return 0 park")
    require(WORDS[90] == 0x8F830090, "lw gp+0x90")
    require(WORDS[92] == 0x2463FFF4, "rewind gp+0x90 by 12")
    require(WORDS[93] == 0xAF830090, "sw rewinded pc")
    require(jal_target(load_u32(data, HP_JAL)) == HP_CUT, "29810 jal 293F4")
    require(load_u32(data, HP_DELAY) == 0x00002021, "293F4 a0=0")
    require(WORDS[-2] == 0x03E00008, "jr $ra")
    print("PASS: func_800144FC 102/102 + JT + sole jal 29810 + 293F4(0)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
