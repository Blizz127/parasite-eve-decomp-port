#!/usr/bin/env python3
"""PE-BTL53 independent oracle: 69594 / 6F8EC / D4704 and 299CC idle gate."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def window_sha(data: bytes, start: int, end: int) -> str:
    return hashlib.sha256(data[exe_off(start) : exe_off(end)]).hexdigest()


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")

    require((0x8006965C - 0x80069594) // 4 == 50, "69594 50w")
    require(
        window_sha(data, 0x80069594, 0x8006965C)
        == "b182bcd8f13145cb14a8f6f2bfb8d0b71f502b05e5b16ec1871265379201ac47",
        "69594 sha",
    )
    require(load_u32(data, 0x800695BC) == 0x0C01BE3B, "jal 6F8EC")
    require(load_u32(data, 0x80035B2C) == 0x0C01A565, "35558 jal 69594")
    require(load_u32(data, 0x800355E8) == 0x0C00A673, "35558 jal 299CC")

    require((0x8006F9F0 - 0x8006F8EC) // 4 == 65, "6F8EC 65w")
    require(
        window_sha(data, 0x8006F8EC, 0x8006F9F0)
        == "659a9a3e22b42a66673d89c32f04928a5f0e62a462e776001daa71d4d0056563",
        "6F8EC sha",
    )
    require(load_u32(data, 0x8006F900) == 0x2402FFF0, "OOB -16")
    require(load_u32(data, 0x8006F9C0) == 0x8C42000C, "lw entry+0xC")

    require((0x800D4850 - 0x800D4704) // 4 == 83, "D4704 83w")
    require(
        window_sha(data, 0x800D4704, 0x800D4850)
        == "4090a71a250272914789be846d8cf6ae63a655ce28291eb9edd324f86b0b62ef",
        "D4704 sha",
    )
    require(load_u32(data, 0x800D472C) == 0xAC3032D0, "sw slot F32D0")
    require(load_u32(data, 0x800D4770) == 0x3402FFFF, "ori FFFF")
    require(load_u32(data, 0x800D4828) == 0x00001021, "v0=0 after FFFF loop")

    require(load_u32(data, 0x80029A20) == 0x9383010C, "lbu edge")
    require(load_u32(data, 0x80029A7C) == 0x1040035E, "beqz 4D4 → 2A7F8")
    require(load_u32(data, 0x80029464) == 0xA38204D4, "293F4(1) sb 4D4")
    require(load_u32(data, 0x80029488) == 0xA38004D4, "293F4(0) clear 4D4")
    require(load_u32(data, 0x8002A4FC) == 0x0C0074D0, "jal 1D340")
    require(load_u32(data, 0x8001F5A0) == 0x08007D75, "1F4D4 j sb-4")

    print("PASS: 69594/6F8EC/D4704 + 299CC 4D4 idle + 1D340 rec=4 gate")
    return 0


if __name__ == "__main__":
    sys.exit(main())
