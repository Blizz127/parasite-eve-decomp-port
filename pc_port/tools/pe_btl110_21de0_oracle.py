#!/usr/bin/env python3
"""PE-BTL110 — 21DE0 → 21F38 → 2312C → 23008."""
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


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, va - TADDR + HDR)[0]


def jal_target(word: int) -> int:
    return 0x80000000 | ((word & 0x3FFFFFF) << 2)


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require(jal_target(load_u32(blob, 0x8002A4B8)) == 0x80021DE0, "299CC")
    require(jal_target(load_u32(blob, 0x80021E3C)) == 0x80021F38, "21DE0")
    require(jal_target(load_u32(blob, 0x8002205C)) == 0x8002312C, "21F38")
    require(jal_target(load_u32(blob, 0x800235AC)) == 0x80023008, "2312C")
    require(load_u32(blob, 0x8001078C) == 0x8002317C, "jtbl 6")
    require(load_u32(blob, 0x80021F30) == 0x03E00008, "21DE0 jr")
    print("PASS: 21DE0 → 21F38 → 2312C → 23008")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
