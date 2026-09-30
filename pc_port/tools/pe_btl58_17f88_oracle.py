#!/usr/bin/env python3
"""PE-BTL58 — opcode 0x52/0x53/0xA6 ROM contract."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
EXE = ROOT / "build" / "disc1.candidate.exe"
TADDR = 0x80010000
HDR = 0x800


def exe_off(va: int) -> int:
    return va - TADDR + HDR


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, exe_off(va))[0]


def main() -> int:
    blob = EXE.read_bytes()
    table = 0x800910A0
    cases = (
        (0x52, 0x80017F88, 10, "ed1c623197818f92f87ae1fc3b82baa3d14f84a7bb66e9472a1876bc1348bb2b"),
        (0x53, 0x80017FB0, 11, "3594551d87ad74ca023d6f16c06a295719ee38c4a07dc9edaa698b6e315f259f"),
        (0xA6, 0x80019484, 11, "731901885ba4878c7e07bcaf57d1106489875f5a4dcafb7b08a690dd78b64a3f"),
    )
    for op, va, words, sha in cases:
        got = load_u32(blob, table + op * 4)
        if got != va:
            print(f"FAIL: table[{op:#x}]={got:#x} want {va:#x}", file=sys.stderr)
            return 1
        end = va + words * 4
        digest = hashlib.sha256(blob[exe_off(va) : exe_off(end)]).hexdigest()
        if digest != sha:
            print(f"FAIL: {va:#x} sha {digest}", file=sys.stderr)
            return 1
    if load_u32(blob, 0x80017FAC) != 0x24020001:
        print("FAIL: 17FAC v0=1", file=sys.stderr)
        return 1
    if load_u32(blob, 0x80017FD8) != 0x24020001:
        print("FAIL: 17FD8 v0=1", file=sys.stderr)
        return 1
    if load_u32(blob, 0x80019498) != 0x0C010E30:
        print("FAIL: 19498 jal 438C0", file=sys.stderr)
        return 1
    print("PASS: 0x52/17F88 D1A0-or + 0x53/17FB0 and-not + 0xA6/19484 438C0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
