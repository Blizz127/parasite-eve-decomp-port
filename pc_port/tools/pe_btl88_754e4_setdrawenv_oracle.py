#!/usr/bin/env python3
"""PE-BTL88 — 754E4 DrawOTagEnv software (75EE0 + 71A34)."""
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
    require((0x800755BC - 0x800754E4) // 4 == 54, "754E4 54w")
    require(jal_target(load_u32(blob, 0x80070F84)) == 0x800754E4, "70F84")
    require(jal_target(load_u32(blob, 0x80075544)) == 0x80075EE0, "75EE0")
    require(jal_target(load_u32(blob, 0x80075598)) == 0x80071A34, "71A34")
    require(load_u32(blob, 0x80071A34) == 0x240A00A0, "A0 trampoline")
    require(load_u32(blob, 0x80071A3C) == 0x2409002A, "A(0x2A) memcpy")
    require((0x80076150 - 0x80075EE0) // 4 == 156, "75EE0 156w")
    require(
        window_sha(blob, 0x800754E4, 0x800755BC)
        == "628eb6546959fc2e0c229194b76f3e23c65758b80b26311e6a54311d137f9b55",
        "754E4 sha",
    )
    require(
        window_sha(blob, 0x80075EE0, 0x80076150)
        == "f9d8e1e65e51a8fe80c3af30e184c576e9599f1d745d735cdc04913c1aea08a1",
        "75EE0 sha",
    )
    print("PASS: 754E4 75EE0 SetDrawEnv + A(0x2A) memcpy; 76C34 not this cut")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
