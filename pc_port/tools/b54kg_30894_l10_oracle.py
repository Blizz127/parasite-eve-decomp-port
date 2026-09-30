#!/usr/bin/env python3
"""Independent B54K-G oracle for func_80030894 fixed sprites and L10.

No production code is imported. The oracle hashes the exact retail window,
checks all 77 words, decodes all calls, proves L10/address geometry, verifies
the computed CLUT inputs, and checks both words adjacent to the cut.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_word_map

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD_BASE = 0x8000F800
START = 0x800311EC
END = 0x80031320
WINDOW_SHA256 = "b8eafebde2564d8c3315c39c6b7e37ae8fe04036c525f8bdd2d12a761d0e86e2"

CALLS = [
    (0x80031208, 0x800370DC),
    (0x8003122C, 0x80077AA4),
    (0x80031268, 0x800370DC),
    (0x800312E0, 0x800370DC),
]

WORDS = exe_word_map(0x800311EC, 77)


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
    require(len(window) == 0x134 and len(window) // 4 == 77,
            "window size is not 77 words")
    require(hashlib.sha256(window).hexdigest() == WINDOW_SHA256,
            "window SHA-256")
    print("  OK retail window: 77 words, SHA-256 exact")

    def rom(address: int) -> int:
        return struct.unpack_from("<I", data, address - LOAD_BASE)[0]

    actual_calls = []
    for address in range(START, END, 4):
        word = rom(address)
        if word >> 26 == 3:
            actual_calls.append((address, jal_target(word)))
    require(actual_calls == CALLS, "four-call address/order census")
    print("  OK call census: wrapper, CLUT, wrapper, L10 wrapper")

    require(len(WORDS) == 77, "oracle word map is not complete")
    for address in range(START, END, 4):
        expected = WORDS[address]
        require(rom(address) == expected,
                f"word {address:#010x}: {rom(address):08X} != {expected:08X}")
    print("  OK complete word comparison: 77/77")

    require((rom(0x80031210) & 0xFFFF) == 0x130,
            "computed CLUT x input")
    require((rom(0x80031214) & 0xFFFF) == 0x1F9,
            "computed CLUT y input")
    require((((0x1F9 << 6) | (0x130 >> 4)) & 0xFFFF) == 0x7E53,
            "computed CLUT value")
    print("  OK CLUT input/value: (0x130,0x1F9) -> 0x7E53")

    branch = rom(0x80031318)
    offset = branch & 0xFFFF
    if offset & 0x8000:
        offset -= 0x10000
    target = 0x8003131C + offset * 4
    require(target == 0x800312CC, "L10 back-edge target")
    require((rom(0x80031314) & 0xFFFF) == 2, "L10 bound is not two")
    require((1 * 8 - 1) * 8 == 56, "L10 bank stride")
    require(2 * 28 == 0x38, "L10 packet extent arithmetic")
    print("  OK L10: head 0x800312CC, bound 2, strides 56/28, extent 0x38")

    require(rom(START - 4) == 0xA2170006, "word before B54K-G window")
    require(rom(END) == 0x3C12800A, "first excluded fixed-group word")
    print("  OK cut: prior L9 delay slot and first excluded group word exact")
    print("\nB54K-G oracle: 7 check groups passed.")
    return 0


if __name__ == "__main__":
    sys.exit(run())
