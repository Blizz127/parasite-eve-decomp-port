#!/usr/bin/env python3
"""Phase 6E-DRW1 — DrawOTag (753B4) window and dispatch immediates."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
ROOT = pathlib.Path(__file__).resolve().parents[2]
CANDIDATE = ROOT / "build" / "disc1.candidate.exe"
EXTRACTED = ROOT / "build" / "extracted" / "disc1" / "SLUS_006.62"
TADDR = 0x80010000
HDR = 0x800


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(va: int) -> int:
    return va - TADDR + HDR


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, exe_off(va))[0]


def window_sha(blob: bytes, start: int, end: int) -> str:
    return hashlib.sha256(blob[exe_off(start) : exe_off(end)]).hexdigest()


def main() -> int:
    options = ([pathlib.Path(sys.argv[1])] if len(sys.argv) > 1
               else [CANDIDATE, EXTRACTED])
    blob = None
    used = None
    for exe in options:
        if exe.is_file() and hashlib.sha1(exe.read_bytes()).hexdigest() == SHA1:
            blob = exe.read_bytes()
            used = exe
            break
    require(blob is not None, f"no SHA-1-exact EXE among {options}")
    print(f"authority: {used}")
    require((0x80075424 - 0x800753B4) // 4 == 28, "753B4 28w")
    require(
        window_sha(blob, 0x800753B4, 0x80075424)
        == "5455e73f2dbfa57426753baf94cdffc9ec5c4dc5327d856d3c46f786ff53c45c",
        "753B4 sha",
    )
    require(
        blob[exe_off(0x80011928) : exe_off(0x80011928) + 20]
        == b"DrawOTag(%08x)...\n\x00\x00",
        "debug name",
    )
    require(
        blob[exe_off(0x80011954) : exe_off(0x80011954) + 28]
        == b"DrawOTagEnv(%08x,&08x)...\n\x00\x00",
        "drawotagenv name",
    )
    # jtb[6] worker slot (+0x18) and jtb[2] dispatch slot (+0x8).
    require(load_u32(blob, 0x80075400) & 0xFFFF == 0x0018, "worker slot")
    require(load_u32(blob, 0x80075404) & 0xFFFF == 0x0008, "dispatch slot")
    print("PASS: DrawOTag window, names, dispatch slots")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
