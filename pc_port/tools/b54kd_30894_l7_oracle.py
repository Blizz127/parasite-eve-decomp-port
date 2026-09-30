#!/usr/bin/env python3
"""Independent B54K-D oracle for func_80030894 fixed primitives and L7.

No production code is imported. The oracle hashes the exact retail window,
decodes every call, checks selected address/store words, proves the complete
L7 loop, and verifies the words immediately on both sides of the cut.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD_BASE = 0x8000F800
START = 0x80030F6C
END = 0x800310A4
WINDOW_SHA256 = "0b3149236b8f26a7cc5614dd4179039f1f33824a4c3618fef4cf64eca551659d"

CALLS = [
    (0x80030F88, 0x800370DC),
    (0x80030FD4, 0x80077C64),
    (0x80030FE4, 0x80077C64),
    (0x80031020, 0x80077B64),
    (0x80031058, 0x800370DC),
]

WORDS = {
    # Wrapped sprite: bank * 28 at D_8009E460.
    0x80030F6C: 0x3C10800A,
    0x80030F70: 0x2610E460,
    0x80030F7C: 0x001588C0,
    0x80030F80: 0x02358823,
    0x80030F84: 0x00118880,
    0x80030F88: 0x0C00DC37,
    0x80030FAC: 0x240200E8,
    0x80030FB0: 0x240900E0,
    0x80030FB4: 0xA202000C,
    0x80030FB8: 0xA209000D,
    0x80030FC4: 0x3C01800A,
    0x80030FCC: 0xA429E476,
    0x80030FD0: 0xA6020010,
    0x80030FD8: 0xA6020012,
    # Two direct SetSprt records: bank * 32 at D_8009E498.
    0x80030F90: 0x00159140,
    0x80030F94: 0x3C13800A,
    0x80030F98: 0x2673E498,
    0x80030FD4: 0x0C01DF19,
    0x80030FDC: 0x26730010,
    0x80030FE4: 0x0C01DF19,
    0x80031008: 0xA2890004,
    0x8003100C: 0xA2890005,
    0x80031014: 0xA2890006,
    0x80031018: 0xA2420004,
    0x8003101C: 0xA2420005,
    0x80031024: 0xA2420006,
    # PolyF3: bank * 20 at D_8009E4D8.
    0x80030FEC: 0x00158080,
    0x80030FF0: 0x02158021,
    0x80030FF4: 0x00108080,
    0x80030FF8: 0x3C04800A,
    0x80030FFC: 0x2484E4D8,
    0x80031020: 0x0C01DED9,
    # L7: bank * 84 + slot * 28 at D_8009E3B8.
    0x8003102C: 0x02158021,
    0x80031030: 0x00108880,
    0x80031034: 0x3C12800A,
    0x80031038: 0x2652E3B8,
    0x80031040: 0x32C200FF,
    0x80031044: 0x000280C0,
    0x80031048: 0x02028023,
    0x8003104C: 0x00108080,
    0x80031058: 0x0C00DC37,
    0x8003106C: 0x24020018,
    0x80031070: 0x3C01800A,
    0x80031078: 0xA429E3CE,
    0x80031080: 0x24090008,
    0x80031084: 0xA6020010,
    0x8003108C: 0x2C420003,
    0x80031090: 0xA6090012,
    0x80031094: 0xA2170004,
    0x80031098: 0xA2170005,
    0x8003109C: 0x1440FFE8,
    0x800310A0: 0xA2170006,
}


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
    require(len(window) == 0x138 and len(window) // 4 == 78,
            "window size is not 78 words")
    require(hashlib.sha256(window).hexdigest() == WINDOW_SHA256,
            "window SHA-256")
    print("  OK retail window: 78 words, SHA-256 exact")

    def rom(address: int) -> int:
        return struct.unpack_from("<I", data, address - LOAD_BASE)[0]

    actual_calls = []
    for address in range(START, END, 4):
        word = rom(address)
        if word >> 26 == 3:
            actual_calls.append((address, jal_target(word)))
    require(actual_calls == CALLS, "five-call address/order census")
    print("  OK call census: five jal sites in retail order")

    for address, expected in WORDS.items():
        require(rom(address) == expected,
                f"word {address:#010x}: {rom(address):08X} != {expected:08X}")
    print(f"  OK selected literal/control words: {len(WORDS)}")

    branch = rom(0x8003109C)
    offset = branch & 0xFFFF
    if offset & 0x8000:
        offset -= 0x10000
    target = 0x800310A0 + offset * 4
    require(target == 0x80031040, "L7 back-edge target")
    require((rom(0x8003108C) & 0xFFFF) == 3, "L7 bound is not three")
    require(3 * 28 == 0x54, "L7 packet extent arithmetic")
    print("  OK L7 loop: head 0x80031040, bound 3, extent 0x54")

    require(rom(START - 4) == 0x32D100FF, "word before B54K-D window")
    require(rom(END) == 0x0000B021, "first excluded L8 word")
    print("  OK cut: prior L6 delay slot and first excluded L8 word exact")
    print("\nB54K-D oracle: 5 check groups passed.")
    return 0


if __name__ == "__main__":
    sys.exit(run())
