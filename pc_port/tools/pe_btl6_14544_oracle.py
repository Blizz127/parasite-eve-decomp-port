#!/usr/bin/env python3
"""PE-BTL6 independent oracle: 144FC state 0 and 0x37 / 42EDC.

Pins SHA-1-exact EXE words. State 0 at 0x80014544 stores 0x37 when
(+0xE & 3)==0, oris overlay[0] bit 0x800000 (lw/sw 0($s0)),
returns 0 via 0x80014660.
State 0x37 jals 42EDC when overlay word bit 0x400000 is clear, then
sb 0x38 and re-dispatches. Does not import production C. Does not
claim 0x55 completion, 6D60C/6914C success, or mode 7.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_words

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
JT = 0x800100A0
STATE0 = 0x80014544
STATE37 = 0x80014570
STATE38 = 0x8001459C
FN_42EDC = 0x80042EDC
FN_42EDC_END = 0x80042F20
JOIN0 = 0x80014660
REDISPATCH = 0x80014518

STATE0_WORDS = exe_words(0x80014544, 11)

STATE37_WORDS = exe_words(0x80014570, 11)

FN_42EDC_WORDS = exe_words(0x80042EDC, 17)


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def j_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")

    require(load_u32(data, JT + 0) == STATE0, "JT[0]")
    require(load_u32(data, JT + 0x37 * 4) == STATE37, "JT[0x37]")
    require(load_u32(data, JT + 0x38 * 4) == STATE38, "JT[0x38]")
    require(STATE0 + len(STATE0_WORDS) * 4 == STATE37, "state 0 then 0x37")
    require(STATE37 + len(STATE37_WORDS) * 4 == STATE38, "0x37 then 0x38")

    for i, want in enumerate(STATE0_WORDS):
        got = load_u32(data, STATE0 + i * 4)
        require(got == want, f"state0 @{STATE0 + i * 4:#x}")
    require(STATE0_WORDS[0] == 0x9202000E, "state0 lbu +0xE")
    require(STATE0_WORDS[2] == 0x30420003, "state0 andi 3")
    require(STATE0_WORDS[4] == 0x24020037, "state0 li 0x37")
    require(STATE0_WORDS[5] == 0xA20200F4, "state0 sb +0xF4")
    require(STATE0_WORDS[7] == 0x3C030080, "state0 lui 0x80")
    require(j_target(STATE0_WORDS[9]) == JOIN0, "state0 j 14660")
    require(load_u32(data, JOIN0) == 0x00001021, "14660 v0=0")

    for i, want in enumerate(STATE37_WORDS):
        got = load_u32(data, STATE37 + i * 4)
        require(got == want, f"state37 @{STATE37 + i * 4:#x}")
    require(jal_target(STATE37_WORDS[6]) == FN_42EDC, "0x37 jal 42EDC")
    require(j_target(STATE37_WORDS[9]) == REDISPATCH, "0x37 j 14518")
    require(STATE37_WORDS[10] == 0xA20200F4, "0x37 delay sb +0xF4")
    require(load_u32(data, REDISPATCH) == 0x920300F4, "14518 lbu +0xF4")

    require((FN_42EDC_END - FN_42EDC) // 4 == 17, "42EDC 17 words")
    for i, want in enumerate(FN_42EDC_WORDS):
        got = load_u32(data, FN_42EDC + i * 4)
        require(got == want, f"42EDC @{FN_42EDC + i * 4:#x}")
    require(FN_42EDC_WORDS[1] == 0x9042D024, "lbu D_800BD024")
    require(FN_42EDC_WORDS[8] == 0x28420021, "slti 33")
    require(FN_42EDC_WORDS[13] == 0x24020020, "clamp 32")

    jal = 0x0C000000 | ((FN_42EDC & 0x0FFFFFFF) >> 2)
    hits = [
        0x80010000 + offset - 0x800
        for offset in range(0, len(data) - 4, 4)
        if struct.unpack_from("<I", data, offset)[0] == jal
    ]
    require(hits == [0x80014588, 0x8005D5F8], f"42EDC jals {hits}")

    require(jal_target(load_u32(data, STATE38)) == 0x8006D60C, "0x38 jal 6D60C")
    require(load_u32(data, STATE38 + 4) == 0x24040001, "0x38 a0=1")

    print(
        "PASS: 144FC state 0 @ 14544 sb 0x37 + ori overlay[0] 0x800000 + v0=0; "
        "state 0x37 jal 42EDC then sb 0x38; 42EDC 17w clamp BD024; "
        "next 0x38 jal 6D60C(1)"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
