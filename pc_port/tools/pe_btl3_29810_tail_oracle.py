#!/usr/bin/env python3
"""PE-BTL3 independent oracle: 29810 tail through first actor command.

Checks four SHA-1-exact EXE windows without importing production C:
the post-293F4 tail, func_8001A680's command-store prefix, the 0x55
state-0x3B overlay gate, and the retail mode-7 gate/store.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_words

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
GP = 0x8009CD70

WINDOWS = {
    (0x80029918, 0x800299CC): exe_words(0x80029918, 45),
    (0x8001A680, 0x8001A704): exe_words(0x8001A680, 33),
    (0x80014630, 0x80014658): exe_words(0x80014630, 10),
    (0x8002CEE0, 0x8002CF2C): exe_words(0x8002CEE0, 19),
}


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")

    for (start, end), words in WINDOWS.items():
        require(len(words) == (end - start) // 4, f"window size {start:#x}")
        for index, want in enumerate(words):
            got = load_u32(data, start + index * 4)
            require(got == want, f"{start + index * 4:#x}: {got:#010x}")

    require(jal_target(load_u32(data, 0x80029918)) == 0x800209F0,
            "post-HP call 209F0")
    require(load_u32(data, 0x80029928) == 0x8C620008, "record lw +0x08")
    require(load_u32(data, 0x80029940) == 0xAC620008, "record sw +0x08")
    require(load_u32(data, 0x80029954) == 0x240200F0, "value 240")
    require(load_u32(data, 0x80029958) == 0xAC640008, "cap sw +0x08")
    require(load_u32(data, 0x8002995C) == 0xAC620034, "sw 240 +0x34")
    require(load_u32(data, 0x80029980) == 0xAC620194, "callback sw +0x194")
    require(load_u32(data, 0x80029984) == 0x8C820018, "source lw +0x18")
    require(load_u32(data, 0x80029994) == 0xA422D27C, "sh D_8009D27C")
    require(jal_target(load_u32(data, 0x80029998)) == 0x800339A0,
            "encounter call")
    require(load_u32(data, 0x800299AC) == 0x90450012,
            "first command lbu record+0x12")
    require(jal_target(load_u32(data, 0x800299B0)) == 0x8001A680,
            "first command call")

    require(load_u32(data, 0x8001A6D0) == 0xA232000E,
            "command sb actor+0x0E")
    require(load_u32(data, 0x8001A6D4) == 0xAE200014, "clock sw actor+0x14")
    require(load_u32(data, 0x8001A6D8) == 0xAE200018, "clock sw actor+0x18")
    require(load_u32(data, 0x8001A6DC) == 0xAE2201B0,
            "resource sw actor+0x1B0")
    require(load_u32(data, 0x8001A6EC) == 0xAE220098,
            "flags sw actor+0x98")
    require(load_u32(data, 0x8001A6FC) == 0xA222000F,
            "frame sb actor+0x0F")

    require(load_u32(data, 0x80014630) == 0x9202000E,
            "0x55 wait lbu D_800B0CD8+0xE")
    require(load_u32(data, 0x80014638) == 0x30420003, "wait mask 3")
    require(load_u32(data, 0x8001464C) == 0xA20000F4, "state clear")
    require(load_u32(data, 0x80014654) == 0xAE020000, "inhibit-bit clear")

    require(jal_target(load_u32(data, 0x8002CEE0)) == 0x8006914C,
            "mode-7 overlay wait call")
    require(load_u32(data, 0x8002CF1C) == 0xA0400049, "pre-mode sb +0x49")
    require(load_u32(data, 0x8002CF20) == 0xA0400048, "pre-mode sb +0x48")
    require(load_u32(data, 0x8002CF28) == 0xAF82051C, "mode 7 store")
    require(GP + 0x51C == 0x8009D28C, "mode address")

    print("PASS: 29810 tail 45 + 1A680 command 33 + 0x55 wait 10 + mode7 gate 19")
    return 0


if __name__ == "__main__":
    sys.exit(main())
