#!/usr/bin/env python3
"""Independent B54K-L oracle for the first D_80093126 group."""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_word_map

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD_BASE = 0x8000F800
START = 0x8006B16C
END = 0x8006B220
WINDOW_SHA256 = "5d86f9b22d6c98cf5a33e780bf92c431097c02dc39a7a654533f87645c1fecd6"

WORDS = exe_word_map(0x8006B16C, 45)


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
    require(len(window) == 0xB4 and len(window) // 4 == 45,
            "window size is not 0xB4 / 45 words")
    require(hashlib.sha256(window).hexdigest() == WINDOW_SHA256,
            "window SHA-256")

    def rom(address: int) -> int:
        return struct.unpack_from("<I", data, address - LOAD_BASE)[0]

    require(len(WORDS) == 45, "oracle word map is not complete")
    for address in range(START, END, 4):
        require(rom(address) == WORDS[address], f"word mismatch at {address:#x}")
    print("  OK retail identity and complete comparison: 45/45 words")

    require(jal_target(rom(0x8006B188)) == 0x8006E6A8 and
            rom(0x8006B178) == 0x8EA50188 and
            rom(0x8006B18C) == 0x00C23023,
            "3126 issue ABI differs")
    require(branch_target(0x8006B190, rom(0x8006B190)) == 0x8006B178,
            "issue retry edge differs")
    print("  OK D_80093126 issue: +0x188, end-start, immediate -1 retry")

    require(rom(0x8006B1A4) == 0x8EB4016C and
            rom(0x8006B1AC) == 0x8E820004 and
            rom(0x8006B1B8) == 0x8E620028,
            "E0 archive base/metadata/header path differs")
    require(jal_target(rom(0x8006B1DC)) == 0x8006E1C0 and
            branch_target(0x8006B1F4, rom(0x8006B1F4)) == 0x8006B1D8 and
            rom(0x8006B1F8) == 0x26100014,
            "entry call/loop/stride differs")
    require(branch_target(0x8006B19C, rom(0x8006B19C)) == 0x8006B204 and
            rom(0x8006B1FC) == 0x24100001,
            "entry-walk once gate differs")
    print("  OK E0 walk: packed count/offset, func_8006E1C0, 0x14 stride, once")

    require(branch_target(0x8006B204, rom(0x8006B204)) == 0x8006B16C,
            "timeout descriptor-rebuild edge differs")
    require(jal_target(rom(0x8006B20C)) == 0x8006E7E8 and
            rom(0x8006B214) == 0x00409021,
            "completion poll/result differs")
    require(branch_target(0x8006B218, rom(0x8006B218)) == 0x8006B19C,
            "positive-poll edge differs")
    print("  OK wait topology: timeout rebuilds; positive skips completed walk")

    require(rom(START - 4) == 0x00008021, "prior boundary word")
    require(rom(END) == 0x8EB40188, "first excluded +0x188 archive load")
    require(rom(END - 4) == 0x3C02003F,
            "poll branch delay-slot mask materialization")
    require(END - 0x8006AD40 == 0x4E0 and 0x4E0 // 4 == 312,
            "implemented prefix arithmetic")
    print("  OK cut: delay-slot lui included; next lw s4,0x188(s5)")
    print("  OK prefix arithmetic: 0x4E0 bytes / 312 words")
    print("\nB54K-L oracle: 6 check groups passed.")
    return 0


if __name__ == "__main__":
    sys.exit(run())
