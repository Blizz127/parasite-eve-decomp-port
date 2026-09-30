#!/usr/bin/env python3
"""Independent B54K-K oracle for func_8006AD40's D_800930E0 group.

No production code is imported.  This authenticates the retail executable,
compares the complete 38-word window, and checks issue, lookup, retry, poll,
store, boundary, and prefix geometry directly from retail words.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_word_map

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD_BASE = 0x8000F800
START = 0x8006B0D4
END = 0x8006B16C
WINDOW_SHA256 = "d0f529015f25ed5164d48b005ba3d5f8de273a9d776b44f8128cec57391bed0c"

WORDS = exe_word_map(0x8006B0D4, 38)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"FAIL: {message}")


def find_exe() -> pathlib.Path:
    here = pathlib.Path(__file__).resolve()
    for candidate in (
        here.parent.parent / "build" / "disc1.candidate.exe",
        here.parent.parent.parent / "build" / "disc1.candidate.exe",
        pathlib.Path("build/disc1.candidate.exe"),
        pathlib.Path("pc_port/build/disc1.candidate.exe"),
    ):
        if candidate.is_file():
            return candidate
    raise SystemExit("FAIL: could not locate disc1.candidate.exe")


def branch_target(pc: int, word: int) -> int:
    offset = word & 0xFFFF
    if offset & 0x8000:
        offset -= 0x10000
    return pc + 4 + offset * 4


def jal_target(word: int) -> int:
    return 0x80000000 | ((word & 0x03FFFFFF) << 2)


def run() -> int:
    path = find_exe()
    data = path.read_bytes()
    require(hashlib.sha1(data).hexdigest() == EXE_SHA1, "executable SHA-1")
    window = data[START - LOAD_BASE:END - LOAD_BASE]
    require(len(window) == 0x98 and len(window) // 4 == 38,
            "window size is not 0x98 / 38 words")
    require(hashlib.sha256(window).hexdigest() == WINDOW_SHA256,
            "window SHA-256")
    print("  OK retail identity: exact executable and 38-word window hash")

    def rom(address: int) -> int:
        return struct.unpack_from("<I", data, address - LOAD_BASE)[0]

    require(len(WORDS) == 38, "oracle word map is not complete")
    for address in range(START, END, 4):
        require(rom(address) == WORDS[address],
                f"word {address:#010x}: {rom(address):08X} != {WORDS[address]:08X}")
    print("  OK complete retail comparison: 38/38 words")

    require(jal_target(rom(0x8006B0F0)) == 0x8006E6A8,
            "issue call is not func_8006E6A8")
    require(rom(0x8006B0E0) == 0x8EA5016C and
            rom(0x8006B0E4) == 0x96220000 and
            rom(0x8006B0E8) == 0x96260002 and
            rom(0x8006B0F4) == 0x00C23023,
            "E0 issue arguments are not +0x16C/start/end/end-start")
    require(branch_target(0x8006B0F8, rom(0x8006B0F8)) == 0x8006B0E0,
            "issue -1 retry does not target the E0 issue")
    print("  OK E0 issue: +0x16C, end-start sectors, -1 retries issue only")

    for pc in (0x8006B118, 0x8006B12C, 0x8006B140):
        require(jal_target(rom(pc)) == 0x8006E498,
                f"lookup call at {pc:#010x} is not func_8006E498")
    require((rom(0x8006B10C), rom(0x8006B110),
             rom(0x8006B120), rom(0x8006B128),
             rom(0x8006B134), rom(0x8006B13C)) ==
            (0x3C05C4B5, 0x34A5BA04, 0x3C05CAAD,
             0x34A50704, 0x3C055EAF, 0x34A56804),
            "lookup keys differ")
    require((rom(0x8006B130), rom(0x8006B144), rom(0x8006B148)) ==
            (0xAEA2011C, 0xAEA20120, 0xAEA20124),
            "lookup result stores differ")
    require(branch_target(0x8006B108, rom(0x8006B108)) == 0x8006B14C and
            rom(0x8006B11C) == 0x24100001,
            "lookup-once s0 gate differs")
    print("  OK three lookups: exact keys, +0x11C/+0x120/+0x124, once per group")

    require(branch_target(0x8006B14C, rom(0x8006B14C)) == 0x8006B0D4,
            "timeout edge does not rebuild the E0 descriptor")
    require(jal_target(rom(0x8006B154)) == 0x8006E7E8 and
            rom(0x8006B15C) == 0x00409021,
            "completion poll/result retention differs")
    require(branch_target(0x8006B160, rom(0x8006B160)) == 0x8006B108,
            "positive poll does not return through the lookup-done gate")
    require(rom(0x8006B168) == 0x00008021,
            "zero completion does not clear s0")
    print("  OK wait topology: timeout rebuilds; positive repolls; zero exits")

    require(rom(START - 4) == 0x00008021,
            "prior F0 completion boundary word")
    require(rom(END) == 0x3C118009,
            "first excluded D_80093126 setup word")
    require((END - 0x8006AD40) == 0x42C and 0x42C // 4 == 267,
            "implemented prefix arithmetic")
    print("  OK cut geometry: prior clear-s0; next lui s1,%hi(D_80093126)")
    print("  OK prefix arithmetic: 0x42C bytes / 267 words")
    print("\nB54K-K oracle: 7 check groups passed.")
    return 0


if __name__ == "__main__":
    sys.exit(run())
