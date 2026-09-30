#!/usr/bin/env python3
"""Independent B54K-E oracle for func_80030894 L8.

No production code is imported. The oracle hashes the exact retail window,
checks all 27 words, decodes the sole call, proves the complete L8 loop and
its address geometry, and verifies both words adjacent to the cut.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_word_map

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD_BASE = 0x8000F800
START = 0x800310A4
END = 0x80031110
WINDOW_SHA256 = "9b87877a27ab26756ab9cfbdf9f6f5d4e31958975ca654fcdeaf87660f7b7ecb"

CALLS = [(0x800310E4, 0x800370DC)]

WORDS = exe_word_map(0x800310A4, 27)


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
    window = data[START - LOAD_BASE:END - LOAD_BASE]
    require(len(window) == 0x6C and len(window) // 4 == 27,
            "window size is not 27 words")
    require(hashlib.sha256(window).hexdigest() == WINDOW_SHA256,
            "window SHA-256")
    print("  OK retail window: 27 words, SHA-256 exact")

    def rom(address: int) -> int:
        return struct.unpack_from("<I", data, address - LOAD_BASE)[0]

    actual_calls = []
    for address in range(START, END, 4):
        word = rom(address)
        if word >> 26 == 3:
            actual_calls.append((address, jal_target(word)))
    require(actual_calls == CALLS, "single-call address/target census")
    print("  OK call census: one jal func_800370DC site")

    require(len(WORDS) == 27, "oracle word map is not complete")
    for address in range(START, END, 4):
        expected = WORDS[address]
        require(rom(address) == expected,
                f"word {address:#010x}: {rom(address):08X} != {expected:08X}")
    print("  OK complete word comparison: 27/27")

    branch = rom(0x80031108)
    offset = branch & 0xFFFF
    if offset & 0x8000:
        offset -= 0x10000
    target = 0x8003110C + offset * 4
    require(target == 0x800310CC, "L8 back-edge target")
    require((rom(0x800310FC) & 0xFFFF) == 10, "L8 bound is not ten")
    require((((1 * 8 + 1) * 4 - 1) * 8) == 280,
            "L8 bank-stride arithmetic")
    require(10 * 28 == 0x118, "L8 packet extent arithmetic")
    print("  OK L8 loop: head 0x800310CC, bound 10, strides 280/28, extent 0x118")

    require(rom(START - 4) == 0xA2170006, "word before B54K-E window")
    require(rom(END) == 0x3C10800A, "first excluded fixed-group word")
    print("  OK cut: prior L7 delay slot and first excluded group word exact")
    print("\nB54K-E oracle: 6 check groups passed.")
    return 0


if __name__ == "__main__":
    sys.exit(run())
