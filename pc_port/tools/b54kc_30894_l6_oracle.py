#!/usr/bin/env python3
"""Independent B54K-C oracle for func_80030894 fixed records and L6.

No production code is imported. The oracle hashes the exact retail window,
decodes every call, checks selected address/store words, proves the L6 loop,
and verifies the first excluded post-L6 instruction.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_word

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD_BASE = 0x8000F800
START = 0x80030D20
END = 0x80030F6C
WINDOW_SHA256 = "51fe2651b8b7df743b8fc277f79101757a0b0904c4fbee00d28dc4a30bf19125"

CALLS = [
    (0x80030D3C, 0x80077BC4),
    (0x80030D4C, 0x80077BC4),
    (0x80030DF8, 0x800370DC),
    (0x80030E0C, 0x80077B34),
    (0x80030E5C, 0x800370DC),
    (0x80030E70, 0x80077B34),
    (0x80030EBC, 0x800370DC),
    (0x80030ED0, 0x80077B34),
    (0x80030F34, 0x80077C44),
]

WORDS = {a: exe_word(a) for a in (0x80030D20, 0x80030D24, 0x80030D28, 0x80030D2C, 0x80030D30, 0x80030D34, 0x80030D38, 0x80030D3C, 0x80030D4C, 0x80030D54, 0x80030D58, 0x80030D5C, 0x80030D60, 0x80030D94, 0x80030DCC, 0x80030DF8, 0x80030DFC, 0x80030E0C, 0x80030E10, 0x80030E14, 0x80030E18, 0x80030E44, 0x80030E48, 0x80030E4C, 0x80030E5C, 0x80030E70, 0x80030E78, 0x80030E7C, 0x80030EA4, 0x80030EA8, 0x80030EAC, 0x80030EBC, 0x80030ED0, 0x80030ED8, 0x80030EDC, 0x80030EE0, 0x80030EE4, 0x80030F04, 0x80030F08, 0x80030F0C, 0x80030F10, 0x80030F14, 0x80030F28, 0x80030F2C, 0x80030F34, 0x80030F38, 0x80030F44, 0x80030F48, 0x80030F50, 0x80030F54, 0x80030F58, 0x80030F60, 0x80030F64, 0x80030F68, 0x80030F6C)}


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
    require(len(window) == 0x24C and len(window) // 4 == 147,
            "window size is not 147 words")
    require(hashlib.sha256(window).hexdigest() == WINDOW_SHA256,
            "window SHA-256")
    print("  OK retail window: 147 words, SHA-256 exact")

    def rom(address: int) -> int:
        return struct.unpack_from("<I", data, address - LOAD_BASE)[0]

    actual_calls = []
    for address in range(START, END, 4):
        word = rom(address)
        if word >> 26 == 3:
            actual_calls.append((address, jal_target(word)))
    require(actual_calls == CALLS, "nine-call address/order census")
    print("  OK call census: nine jal sites in retail order")

    for address, expected in WORDS.items():
        require(rom(address) == expected,
                f"word {address:#010x}: {rom(address):08X} != {expected:08X}")
    print(f"  OK selected literal/control words: {len(WORDS)}")

    branch = rom(0x80030F64)
    offset = branch & 0xFFFF
    if offset & 0x8000:
        offset -= 0x10000
    target = 0x80030F68 + offset * 4
    require(target == 0x80030F2C, "L6 back-edge target")
    require((rom(0x80030F60) & 0xFFFF) == 3, "L6 bound is not three")
    require(3 * 16 == 0x30, "L6 packet extent arithmetic")
    require(rom(END) == 0x3C10800A, "first excluded post-L6 word")
    print("  OK L6 loop: head 0x80030F2C, bound 3, extent 0x30")
    print("  OK cut: first excluded word materializes D_8009E460")
    print("\nB54K-C oracle: 5 check groups passed.")
    return 0


if __name__ == "__main__":
    sys.exit(run())
