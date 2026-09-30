#!/usr/bin/env python3
"""PE-BTL47 independent oracle: 0x12 / 131E8 script-task fork."""
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

    require((0x80013300 - 0x800131E8) // 4 == 70, "131E8 70w")
    require(
        window_sha(data, 0x800131E8, 0x80013300)
        == "ae11241b3248c74e215fdb22736bc4bf80c77d0e8de4eedcb13e02c2f0133e1a",
        "131E8 sha",
    )
    require(load_u32(data, 0x800910A0 + 0x12 * 4) == 0x800131E8, "table[0x12]")
    require(load_u32(data, 0x800131E8) == 0x8F860590, "lw D300")
    require(load_u32(data, 0x800131F8) == 0x30420003, "andi task+8 3")
    require(load_u32(data, 0x80013210) == 0x8F88008C, "lw CDFC")
    require(load_u32(data, 0x80013278) == 0xAC4800A8, "sw A8 prepend")
    require(load_u32(data, 0x800132FC) == 0x24020001, "v0=1")

    print("PASS: 0x12 131E8 forks via CDFC; &3 selects +0x24 vs A8")
    return 0


if __name__ == "__main__":
    sys.exit(main())
