#!/usr/bin/env python3
"""Read-only B54K-M readiness oracle for func_8006AD40's final suffix.

This does not import production code.  It authenticates the exact executable,
compares every remaining retail word, and verifies the loop, SDK calls, state
writes, branches, and return needed by a future completion rung.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_hex_text

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD_BASE = 0x8000F800
FUNC_START = 0x8006AD40
START = 0x8006B220
END = 0x8006B35C
FUNC_SHA256 = "36c7674380e42a7cb5c13f3802689b936bf6e48b67994112c510abbeaaef5539"
WINDOW_SHA256 = "1aed4d0f3bcaa751da044557dccd15e0f15a002addd7f0333ecaa635885749d8"

WORDS_HEX = exe_hex_text(0x8006B220, 79, 'value', True)
WORDS = [int(word, 16) for word in WORDS_HEX.split()]

CALLS = {
    0x8006B254: (0x8006E1C0, "func_8006E1C0", "translated retail helper"),
    0x8006B274: (0x80087024, "func_80087024", "native stream F1 provider"),
    0x8006B27C: (0x80074DC0, "func_80074DC0", "host DrawSync shim"),
    0x8006B284: (0x80074A44, "func_80074A44", "native ResetGraph"),
    0x8006B28C: (0x80073A44, "func_80073A44", "host VSync shim"),
    0x8006B2B4: (0x800755F0, "func_800755F0", "host PutDispEnv shim"),
    0x8006B2BC: (0x80074D28, "func_80074D28", "host SetDispMask shim"),
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
    body = data[FUNC_START - LOAD_BASE:END - LOAD_BASE]
    require(len(WORDS) == 79 and len(window) == 0x13C,
            "suffix geometry is not 79 words / 0x13C")
    require(hashlib.sha256(window).hexdigest() == WINDOW_SHA256,
            "suffix SHA-256")
    require(hashlib.sha256(body).hexdigest() == FUNC_SHA256,
            "full-function SHA-256")

    def rom(address: int) -> int:
        return struct.unpack_from("<I", data, address - LOAD_BASE)[0]

    for i, expected in enumerate(WORDS):
        address = START + i * 4
        require(rom(address) == expected, f"word mismatch at {address:#x}")
    print("  OK identities: exact executable, full body, suffix, 79/79 words")

    require(rom(0x8006B220) == 0x8EB40188 and
            rom(0x8006B228) == 0x8E830004 and
            rom(0x8006B234) == 0x8E630028,
            "+0x188 archive base/metadata/header differs")
    require(branch_target(0x8006B244, rom(0x8006B244)) == 0x8006B274,
            "zero-count edge does not skip the entry loop")
    require(branch_target(0x8006B26C, rom(0x8006B26C)) == 0x8006B250 and
            rom(0x8006B270) == 0x26100014,
            "entry loop back edge/0x14 stride differs")
    print("  OK +0x188 archive walk: packed header, zero bypass, 0x14 stride")

    for pc, (target, name, classification) in CALLS.items():
        require(jal_target(rom(pc)) == target, f"{name} target at {pc:#x}")
        print(f"  OK call {pc:#010x}: {name} ({classification})")

    require((rom(0x8006B294), rom(0x8006B298), rom(0x8006B2A0),
             rom(0x8006B2A4), rom(0x8006B2A8), rom(0x8006B2B0),
             rom(0x8006B2B8)) ==
            (0x3C02800A, 0x8C42CDDC, 0x00022080,
             0x00822021, 0x00042080, 0x2442CE80, 0x00822021),
            "PutDispEnv index*20 address derivation differs")
    print("  OK display env: D_800BCE80 + D_800ACDDC*20")

    required_stores = {
        0x8006B2D0: 0xA6A30006, 0x8006B2D4: 0xA2A0000B,
        0x8006B2D8: 0xA2A4000C, 0x8006B2DC: 0xA2A40009,
        0x8006B2E0: 0xA6A300E8, 0x8006B2E4: 0xA2A000EB,
        0x8006B2F0: 0xA2A000EA, 0x8006B2F4: 0xA2A400DA,
        0x8006B2F8: 0xA2A400DD, 0x8006B2FC: 0xA2A400DC,
        0x8006B314: 0xA2A400DB, 0x8006B318: 0xA2A400DF,
        0x8006B31C: 0xA2A400DE, 0x8006B32C: 0xAEA30000,
    }
    for pc, word in required_stores.items():
        require(rom(pc) == word, f"state store at {pc:#x}")
    require(branch_target(0x8006B2EC, rom(0x8006B2EC)) == 0x8006B300 and
            branch_target(0x8006B30C, rom(0x8006B30C)) == 0x8006B320,
            "0x40/0x80 conditional-field branches differ")
    require((rom(0x8006B320), rom(0x8006B324), rom(0x8006B328),
             rom(0x8006B32C)) ==
            (0x8EA30000, 0x2404FFFE, 0x00641824, 0xAEA30000),
            "final flags &= ~1 differs")
    print("  OK state reset: exact widths/order, 0x40/0x80 gates, bit 0 clear")

    require(rom(0x8006B354) == 0x03E00008 and
            rom(0x8006B358) == 0x00000000,
            "return is not jr ra + nop")
    require(rom(END) == 0x27BDFFE8,
            "next function boundary is not addiu sp,-0x18")
    print("  OK completion geometry: retail epilogue and next-function boundary")
    print("\nB54K-M readiness oracle: PASS (all 7 callees available; no code changed).")
    return 0


if __name__ == "__main__":
    sys.exit(run())
