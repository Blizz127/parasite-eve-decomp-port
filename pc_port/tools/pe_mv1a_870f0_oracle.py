#!/usr/bin/env python3
"""Phase 6E-MV1a - func_800870F0 movie-frame scaler latch (42 words).

Retail bytes prove: the [D_8009D2C0] & 2 branch predicate, the
sll/addu/subu/srl scale chain, the four sb stores per arm, and the
jal to func_8007A88C (the MV1a strict frontier). A pure-python
emulation of the integer chain over all 256 film-id bytes proves the
((2903 * a0) >> 13) & 0xFF transcription, wrapping included.
"""
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
    # 42 words, 0x800870F0..0x80087198.
    require((0x80087198 - 0x800870F0) // 4 == 42, "870F0 42w")
    require(
        window_sha(blob, 0x800870F0, 0x80087198)
        == "0d87ed18f452399a2a6a7fff5ba185844d637062c35d051dd73f4c6aa543faf5",
        "870F0 sha",
    )
    # Branch predicate: lw D_8009D2C0, andi 2, beqz to the clear arm.
    require(load_u32(blob, 0x800870F4) & 0xFFFF == 0xD2C0, "D2C0 lo16")
    require(load_u32(blob, 0x800870FC) == 0x30420002, "andi 2")
    require(load_u32(blob, 0x80087100) == 0x10400014, "beqz to 87154")
    # Scale chain shape: sll1/addu/sll2/subu/sll5/addu/sll3/subu/srl13.
    # SPECIAL-format fields (rs, rt, rd, sa, funct); v0=2 v1=3 a0=4.
    for k, (rs, rt, rd, sa, fn) in enumerate((
        (0, 4, 2, 1, 0x00), (2, 4, 2, 0, 0x21),    # sll v0,a0,1 ; addu v0,v0,a0
        (0, 2, 2, 2, 0x00), (2, 4, 2, 0, 0x23),    # sll v0,v0,2 ; subu v0,v0,a0
        (0, 2, 3, 5, 0x00), (2, 3, 2, 0, 0x21),    # sll v1,v0,5 ; addu v0,v0,v1
        (0, 2, 2, 3, 0x00), (2, 4, 2, 0, 0x23),    # sll v0,v0,3 ; subu v0,v0,a0
        (0, 2, 2, 13, 0x02),                       # srl v0,v0,13
    )):
        va = 0x80087108 + 4 * k
        want = (rs << 21) | (rt << 16) | (rd << 11) | (sa << 6) | fn
        require(load_u32(blob, va) == want, f"chain word {va:#x}")
    # Four sb stores per arm, all to D_8009D1C8..D_8009D1CB.
    for va in (0x80087130, 0x80087138, 0x80087140, 0x80087148,
               0x8008715C, 0x80087164, 0x8008716C, 0x80087174):
        require(load_u32(blob, va) & 0xFFFF0000 in (0xA0220000, 0xA0200000),
                f"sb store {va:#x}")
        require(load_u32(blob, va) & 0xFFFF in (0xD1C8, 0xD1C9, 0xD1CA, 0xD1CB),
                f"store addr {va:#x}")
    # Tail calls func_8007A88C with &D_8009D1C8.
    require(jal_target(load_u32(blob, 0x80087180)) == 0x8007A88C,
            "7A88C call")
    require(load_u32(blob, 0x8008717C) & 0xFFFF == 0xD1C8, "D1C8 arg lo16")
    # The integer chain equals ((2903 * a0) >> 13) & 0xFF for every byte.
    for a0 in range(256):
        m = 0xFFFFFFFF
        v = a0
        v = ((v << 1) + a0) & m
        v = ((v << 2) - a0) & m
        v = ((v + ((v << 5) & m)) << 3) - a0 & m
        got = (v >> 13) & 0xFF
        want = ((2903 * a0) >> 13) & 0xFF
        require(got == want, f"scale math a0={a0}")
    print("PASS: 870F0 window, predicate, chain, stores, 7A88C call, "
          "scale math x256")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
