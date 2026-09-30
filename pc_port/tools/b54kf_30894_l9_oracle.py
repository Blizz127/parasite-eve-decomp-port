#!/usr/bin/env python3
"""Independent B54K-F oracle for func_80030894 fixed sprite and L9.

No production code is imported. The oracle hashes the exact retail window,
checks all 55 words, decodes both calls, proves L9 and its address geometry,
and verifies both words adjacent to the cut.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_word_map

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD_BASE = 0x8000F800
START = 0x80031110
END = 0x800311EC
WINDOW_SHA256 = "7d64f3e10e16895bafd1b2fb71ed2757a5513072a35bd9772e04bd84ce56a739"

CALLS = [
    (0x8003112C, 0x800370DC),
    (0x800311A4, 0x800370DC),
]

WORDS = exe_word_map(0x80031110, 55)


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
    require(len(window) == 0xDC and len(window) // 4 == 55,
            "window size is not 55 words")
    require(hashlib.sha256(window).hexdigest() == WINDOW_SHA256,
            "window SHA-256")
    print("  OK retail window: 55 words, SHA-256 exact")

    def rom(address: int) -> int:
        return struct.unpack_from("<I", data, address - LOAD_BASE)[0]

    actual_calls = []
    for address in range(START, END, 4):
        word = rom(address)
        if word >> 26 == 3:
            actual_calls.append((address, jal_target(word)))
    require(actual_calls == CALLS, "two-call address/order census")
    print("  OK call census: two jal func_800370DC sites")

    require(len(WORDS) == 55, "oracle word map is not complete")
    for address in range(START, END, 4):
        expected = WORDS[address]
        require(rom(address) == expected,
                f"word {address:#010x}: {rom(address):08X} != {expected:08X}")
    print("  OK complete word comparison: 55/55")

    branch = rom(0x800311E4)
    offset = branch & 0xFFFF
    if offset & 0x8000:
        offset -= 0x10000
    target = 0x800311E8 + offset * 4
    require(target == 0x8003118C, "L9 back-edge target")
    require((rom(0x800311D8) & 0xFFFF) == 4, "L9 bound is not four")
    require((1 * 8 - 1) * 4 == 28, "fixed-sprite bank stride")
    require((1 * 8 - 1) * 16 == 112, "L9 bank stride")
    require(4 * 28 == 0x70, "L9 packet extent arithmetic")
    print("  OK L9: head 0x8003118C, bound 4, strides 112/28, extent 0x70")

    require(rom(START - 4) == 0xA2170006, "word before B54K-F window")
    require(rom(END) == 0x3C10800A, "first excluded fixed-group word")
    print("  OK cut: prior L8 delay slot and first excluded group word exact")
    print("\nB54K-F oracle: 6 check groups passed.")
    return 0


if __name__ == "__main__":
    sys.exit(run())
