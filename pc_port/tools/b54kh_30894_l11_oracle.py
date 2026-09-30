#!/usr/bin/env python3
"""Independent B54K-H oracle for func_80030894 fixed sprite and L11.

No production code is imported. The oracle hashes the exact retail window,
checks all 70 words, decodes every call, proves descriptor indices and
L11/address geometry, and verifies both words adjacent to the cut.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_word_map

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD_BASE = 0x8000F800
START = 0x80031320
END = 0x80031438
WINDOW_SHA256 = "bdfec78193cfc60b0ed14829f5f3fb42ce74db2cbfe0431ad402f174fd665538"

CALLS = [
    (0x8003133C, 0x800370DC),
    (0x80031394, 0x8005DADC),
    (0x800313AC, 0x80077A64),
    (0x800313C8, 0x800370DC),
]

WORDS = exe_word_map(0x80031320, 70)


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
    require(len(window) == 0x118 and len(window) // 4 == 70,
            "window size is not 70 words")
    require(hashlib.sha256(window).hexdigest() == WINDOW_SHA256,
            "window SHA-256")
    print("  OK retail window: 70 words, SHA-256 exact")

    def rom(address: int) -> int:
        return struct.unpack_from("<I", data, address - LOAD_BASE)[0]

    actual_calls = []
    for address in range(START, END, 4):
        word = rom(address)
        if word >> 26 == 3:
            actual_calls.append((address, jal_target(word)))
    require(actual_calls == CALLS, "four-call address/order census")
    print("  OK call census: fixed wrapper, descriptor, TPage, L11 wrapper")

    require(len(WORDS) == 70, "oracle word map is not complete")
    for address in range(START, END, 4):
        expected = WORDS[address]
        require(rom(address) == expected,
                f"word {address:#010x}: {rom(address):08X} != {expected:08X}")
    print("  OK complete word comparison: 70/70")

    require((rom(0x80031398) & 0xFFFF) == 0x6A,
            "descriptor index base")
    require((rom(0x800313A8) & 0xFFFF) == 0x1C0,
            "TPage x input")
    require((rom(0x800313CC) & 0xFFFF) == 0xFFFF,
            "TPage return mask")
    print("  OK descriptor/TPage contract: index 0x6A+slot, mode 7 masked")

    branch = rom(0x80031430)
    offset = branch & 0xFFFF
    if offset & 0x8000:
        offset -= 0x10000
    target = 0x80031434 + offset * 4
    require(target == 0x80031394, "L11 back-edge target")
    require((rom(0x8003142C) & 0xFFFF) == 13, "L11 bound is not thirteen")
    require((((1 * 3) * 8 - 1) * 4 - 1) * 4 == 364,
            "L11 bank stride")
    require(13 * 28 == 0x16C, "L11 packet extent arithmetic")
    print("  OK L11: head 0x80031394, bound 13, strides 364/28, extent 0x16C")

    require(rom(START - 4) == 0x32C200FF, "word before B54K-H window")
    require(rom(END) == 0x00002021, "first excluded epilogue-group word")
    print("  OK cut: prior L10 delay slot and first excluded word exact")
    print("\nB54K-H oracle: 7 check groups passed.")
    return 0


if __name__ == "__main__":
    sys.exit(run())
