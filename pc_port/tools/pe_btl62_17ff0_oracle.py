#!/usr/bin/env python3
"""PE-BTL62 — opcode 0x89 / 17FF0 ROM contract."""
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


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require(load_u32(blob, 0x800910A0 + 0x89 * 4) == 0x80017FF0, "table[0x89]")
    require((0x80018004 - 0x80017FF0) // 4 == 5, "5 words")
    sha = hashlib.sha256(blob[exe_off(0x80017FF0) : exe_off(0x80018004)]).hexdigest()
    require(
        sha == "117504bf5c4fa6434b3703c4e26d350c077de797af2ab6dbbb56366c64e9115c",
        f"sha {sha}",
    )
    require(load_u32(blob, 0x80017FF0) == 0x24020006, "addiu v0,6")
    require(load_u32(blob, 0x80017FF8) == 0xAC22D28C, "sw D28C")
    require(load_u32(blob, 0x80018000) == 0x24020001, "v0=1")
    require(load_u32(blob, 0x8002CF24) == 0x24020007, "only imm-7 writer")
    require(load_u32(blob, 0x8002CF28) == 0xAF82051C, "2CF24 sw gp+0x51C")
    print("PASS: 0x89/17FF0 stores 6; sole imm-7 writer is 2CF24")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
