#!/usr/bin/env python3
"""PE-BTL81 — 6EC08 two-byte status."""
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
    require((0x8006EC6C - 0x8006EC08) // 4 == 25, "6EC08 25w")
    require(jal_target(load_u32(blob, 0x80070EEC)) == 0x8006EC08, "70EEC")
    require(load_u32(blob, 0x8006EC0C) == 0x80420DBA, "lb B0DBA")
    require(load_u32(blob, 0x8006EC28) == 0x94420DBC, "lhu B0DBC")
    require(load_u32(blob, 0x8006EC44) == 0x80630DBB, "lb B0DBB")
    require(
        window_sha(blob, 0x8006EC08, 0x8006EC6C)
        == "9169b21f6777fecf6a13ffb235c562b0c2b695949cb671d1e01624e50341eeed",
        "6EC08 sha",
    )
    print("PASS: 6EC08 25w B0DBA/BB/BC")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
