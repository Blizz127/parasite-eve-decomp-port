#!/usr/bin/env python3
"""Independent B54K-J oracle for the D_800930F0 wait/reissue gate.

No production code is imported. The oracle authenticates the retail
executable, compares all eight new words, decodes the sole call and both
back edges, and checks the two cut-side boundary words.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_word_map

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD_BASE = 0x8000F800
START = 0x8006B0B4
END = 0x8006B0D4
WINDOW_SHA256 = "a8aa4d36f4fc116a2a4b3f5ca9ac8338beb625d749bde2ed82cc72b512993505"

WORDS = exe_word_map(0x8006B0B4, 8)


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
    require(len(window) == 0x20 and len(window) // 4 == 8,
            "window size is not 0x20 / 8 words")
    require(hashlib.sha256(window).hexdigest() == WINDOW_SHA256,
            "window SHA-256")
    print("  OK retail identity: exact executable and 8-word window hash")

    def rom(address: int) -> int:
        return struct.unpack_from("<I", data, address - LOAD_BASE)[0]

    require(len(WORDS) == 8, "oracle word map is not complete")
    for address in range(START, END, 4):
        require(rom(address) == WORDS[address],
                f"word {address:#010x}: {rom(address):08X} != {WORDS[address]:08X}")
    print("  OK complete retail comparison: 8/8 words")

    require(branch_target(0x8006B0B4, rom(0x8006B0B4)) == 0x8006B064,
            "timeout reissue edge does not target D_800930F0 issue")
    require(branch_target(0x8006B0C8, rom(0x8006B0C8)) == 0x8006B098,
            "positive-poll edge does not target s0 gate")
    print("  OK retry topology: -1 -> 0x8006B064; positive -> 0x8006B098")

    require(jal_target(rom(0x8006B0BC)) == 0x8006E7E8,
            "sole call is not func_8006E7E8")
    require(rom(0x8006B0C4) == 0x00409021,
            "poll result is not retained in s2")
    require(rom(0x8006B0D0) == 0x00008021,
            "completion does not clear s0")
    print("  OK live poll: func_8006E7E8 -> s2; zero exit clears s0")

    require(rom(START - 4) == 0x00000000,
            "prior jal delay-slot boundary word")
    require(rom(END) == 0x3C118009,
            "first excluded D_800930E0 setup word")
    print("  OK cut geometry: prior nop; next lui s1,%hi(D_800930E0)")

    require((END - 0x8006AD40) == 0x394 and 0x394 // 4 == 229,
            "implemented prefix arithmetic")
    print("  OK prefix arithmetic: 0x394 bytes / 229 words")
    print("\nB54K-J oracle: 6 check groups passed.")
    return 0


if __name__ == "__main__":
    sys.exit(run())
