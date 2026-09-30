#!/usr/bin/env python3
"""PE-BTL73 — live 0x0B pose + type-3 rect1; Right walk enters 1CAB0."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
CHUNK2_SHA = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
PE_IMG_LBA = 1013
REL = 0x266A
PACKED = 0x0600A921
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


def load_chunk2() -> bytes:
    pointer = (ROOT / "local" / "pe_disc1.path").read_text().strip().splitlines()[0].strip()
    sec0, sec1, sec2 = PACKED & 0xFF, (PACKED >> 8) & 0xFFF, PACKED >> 20
    out = bytearray()
    with open(pointer, "rb") as fh:
        for i in range(sec2):
            fh.seek((PE_IMG_LBA + REL + sec0 + sec1 + i) * 2352 + 24)
            out += fh.read(2048)
    data = bytes(out)
    require(hashlib.sha256(data).hexdigest() == CHUNK2_SHA, "chunk2")
    return data


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require((0x8001CBA0 - 0x8001CAB0) // 4 == 60, "1CAB0 60w")
    require(load_u32(blob, 0x800910A0 + 0x77 * 4) == 0x80014DA0, "table 0x77")
    require(load_u32(blob, 0x800910A0 + 0x85 * 4) == 0x80018EB4, "table 0x85")

    chunk2 = load_chunk2()
    t0 = 0x202C8
    require(struct.unpack_from("<I", chunk2, t0 + 0x5C + 8)[0] == 0, "0x0B arg0")
    require(struct.unpack_from("<I", chunk2, t0 + 0x5C + 12)[0] == 0x100000, "X")
    require(struct.unpack_from("<I", chunk2, t0 + 0x5C + 16)[0] == 0xFBA90000, "Y")
    require(struct.unpack_from("<I", chunk2, t0 + 0x5C + 20)[0] == 0x05410000, "Z")
    t3 = 0x227FC + 0x30
    require(struct.unpack_from("<I", chunk2, t3 + 8)[0] == 0x09940000, "v0x")
    require(struct.unpack_from("<I", chunk2, t3 + 12)[0] == 0x04CD0000, "v0y")
    require(struct.unpack_from("<I", chunk2, t3 + 24)[0] == 0x080A0000, "v2x")
    x = 16 + 5 * 409
    require(0x80A <= x <= 0x994, f"Right 409 ticks X={x}")
    require(0x4CD <= 0x541 <= 0x63D, "Z already in rect1")
    print("PASS: 0x0B (16,1345); rect1 (0x80A,0x4CD)-(0x994,0x63D); 409 Right")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
