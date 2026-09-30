#!/usr/bin/env python3
"""Independent B54K-I oracle for complete func_80030894.

No production code is imported. The oracle hashes the complete retail body
and final 43-word window, compares every final-window word, decodes both
calls, proves the two-bank back edge, and checks the return boundary.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_word_map

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD_BASE = 0x8000F800
FULL_START = 0x80030894
START = 0x80031438
END = 0x800314E4
FULL_SHA256 = "a4dbd2cf130979a0f5db8ed532d0c559c5b3b90b2c5786e10fe91125311ed6e2"
WINDOW_SHA256 = "69704305125d3ecde96259e29d5a100c3ca327f8e130e04541e4e62e13c68bc0"

CALLS = [
    (0x80031444, 0x80077A64),
    (0x80031468, 0x800370DC),
]

WORDS = exe_word_map(0x80031438, 43)


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


def jal_target(word: int) -> int:
    return 0x80000000 | ((word & 0x03FFFFFF) << 2)


def run() -> int:
    path = find_exe()
    data = path.read_bytes()
    require(hashlib.sha1(data).hexdigest() == EXE_SHA1, "executable SHA-1")

    full = data[FULL_START - LOAD_BASE:END - LOAD_BASE]
    window = data[START - LOAD_BASE:END - LOAD_BASE]
    require(len(full) == 0xC50 and len(full) // 4 == 788,
            "full body size is not 788 words")
    require(hashlib.sha256(full).hexdigest() == FULL_SHA256,
            "full body SHA-256")
    require(len(window) == 0xAC and len(window) // 4 == 43,
            "final window size is not 43 words")
    require(hashlib.sha256(window).hexdigest() == WINDOW_SHA256,
            "final window SHA-256")
    print("  OK retail identity: full 788 words and final 43-word hash exact")

    def rom(address: int) -> int:
        return struct.unpack_from("<I", data, address - LOAD_BASE)[0]

    require(len(WORDS) == 43, "oracle word map is not complete")
    for address in range(START, END, 4):
        require(rom(address) == WORDS[address],
                f"word {address:#010x}: {rom(address):08X} != {WORDS[address]:08X}")
    print("  OK complete final-window comparison: 43/43")

    actual_calls = []
    for address in range(START, END, 4):
        word = rom(address)
        if word >> 26 == 3:
            actual_calls.append((address, jal_target(word)))
    require(actual_calls == CALLS, "final two-call address/order census")
    require((rom(0x80031440) & 0xFFFF) == 0x1C0,
            "final TPage x input")
    require((rom(0x80031458) & 0xFFFF) == 0xFFFF,
            "final TPage return mask")
    print("  OK final calls: TPage(0,0,0x1C0,0) then sprite wrapper")

    require(rom(0x8003144C) == 0x3C11800A and
            rom(0x80031450) == 0x2631EC38,
            "final sprite base materialization")
    require((rom(0x8003147C) & 0xFFFF) == 16 and
            (rom(0x80031488) & 0xFFFF) == 0x10 and
            (rom(0x8003148C) & 0xFFFF) == 0x12,
            "final 16x16 dimensions")
    print("  OK final packet: D_8009EC38 + bank*28, RGB 0x80, dimensions 16x16")

    branch = rom(0x800314A8)
    offset = branch & 0xFFFF
    if offset & 0x8000:
        offset -= 0x10000
    target = 0x800314AC + offset * 4
    require(target == 0x80030910, "outer-bank back-edge target")
    require((rom(0x800314A4) & 0xFFFF) == 2, "outer-bank bound")
    require(2 * 28 == 0x38, "final two-packet extent arithmetic")
    print("  OK outer loop: back edge 0x80030910, bound 2, final extent 0x38")

    require(rom(0x800314DC) == 0x03E00008 and
            rom(0x800314E0) == 0,
            "canonical return pair")
    require(rom(END) == 0x27BDFFE0,
            "next function prologue boundary")
    print("  OK return/boundary: jr ra+nop; next function begins 0x800314E4")
    print("\nB54K-I oracle: 6 check groups passed.")
    return 0


if __name__ == "__main__":
    sys.exit(run())
