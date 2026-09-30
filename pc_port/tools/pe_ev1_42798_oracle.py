#!/usr/bin/env python3
"""Phase 6E-EV1 — func_80042798 event-record cleanup walk."""
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


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


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
    require((0x80042848 - 0x80042798) // 4 == 44, "42798 44w")
    require(
        window_sha(blob, 0x80042798, 0x80042848)
        == "ddf5ae1d7847e65ef43d55b58bce77386cda966e8440d3470d46ddae9570758b",
        "42798 sha",
    )
    require(jal_target(load_u32(blob, 0x80042800)) == 0x80072774, "72774 call")
    # Load-bearing immediates, decoded from the retail words themselves.
    require(load_u32(blob, 0x800427CC) & 0xFFFF == 8, "tag const 8")
    require(load_u32(blob, 0x800427D0) & 0xFFFF == 10, "tag const 10")
    require(load_u32(blob, 0x800427D4) & 0xFFFF == 0xFFFF, "handle const -1")
    require(load_u32(blob, 0x800427D8) & 0xFFFF == 12, "tag const 12")
    require(load_u32(blob, 0x80042810) & 0xFFFF == 0x0418, "stride")
    require(load_u32(blob, 0x800427DC) & 0xFFFF == 1, "cursor base+1")
    require(load_u32(blob, 0x800427E0) & 0xFFFF == 0x0831, "bound")
    require(load_u32(blob, 0x800427A4) & 0xFFFF == 0x0830, "range guard")
    require(load_u32(blob, 0x800427FC) & 0xFFFF == 0x000B, "word offset")
    print("PASS: 42798 window, 72774 call, tag/stride/bound immediates")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
