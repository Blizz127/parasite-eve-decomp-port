#!/usr/bin/env python3
"""PE-BTL68 — 3999C *codep index + 710A4/7136C/78934 ROM contract."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
ROOT = pathlib.Path(__file__).resolve().parents[2]
EXE = ROOT / "build" / "disc1.candidate.exe"
TADDR = 0x80010000
HDR = 0x800


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(va: int) -> int:
    return va - TADDR + HDR


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, exe_off(va))[0]


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def window_sha(blob: bytes, start: int, end: int) -> str:
    return hashlib.sha256(blob[exe_off(start) : exe_off(end)]).hexdigest()


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require(load_u32(blob, 0x800399AC) == 0x00C09021, "s2=a2")
    require(load_u32(blob, 0x800399C0) == 0x8E420000, "lw 0(s2)")
    require(load_u32(blob, 0x800399A4) == 0x00809821, "s3=a0 actor")
    require((0x8007136C - 0x800710A4) // 4 == 178, "710A4 178w")
    require((0x800716A4 - 0x8007136C) // 4 == 206, "7136C 206w")
    require((0x80078A94 - 0x80078934) // 4 == 88, "78934 88w")
    require(
        window_sha(blob, 0x800710A4, 0x8007136C)
        == "c6b38210f83bbb91513566a824a810527f1d67d0b63d6d52aa71b6e113c7bcbe",
        "710A4 sha",
    )
    require(
        window_sha(blob, 0x8007136C, 0x800716A4)
        == "0798de323cd08042f6f7892b92513db33bdf69565da9ecd086f4b7d40aaf0ea3",
        "7136C sha",
    )
    require(
        window_sha(blob, 0x80078934, 0x80078A94)
        == "704b7d8ebd6995ceb4134b7c68371678756cff1becec9850d930302d8aeabd02",
        "78934 sha",
    )
    require(load_u32(blob, 0x800943C0 + 21 * 4) == 0x800942F0, "row21")
    require(load_u32(blob, 0x800942F0) == 0x3, "row21 flags")
    require(load_u32(blob, 0x800942FC) == 0x8007136C, "row21 7136C")
    require(load_u32(blob, 0x80094300) == 0x2, "row21 rec1 flags")
    require(load_u32(blob, 0x8009430C) == 0x800710A4, "row21 710A4")
    require(jal_target(load_u32(blob, 0x800710D0)) == 0x8007136C, "710A4 jal")
    require(jal_target(load_u32(blob, 0x80071310)) == 0x80078934, "710A4 78934")
    require(jal_target(load_u32(blob, 0x80071644)) == 0x80078934, "7136C 78934")
    print("PASS: *codep index; row21 7136C/710A4; 78934")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
