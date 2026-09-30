#!/usr/bin/env python3
"""Phase 6E-B558 — func_8007FCFC issue wrapper, func_8007B558 controller,
func_8007B010 poll prefix, func_80073C5C BIOS trampoline, func_80073DE8
getter, func_8007B9EC latch block."""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

from pe_exe_words import exe_word

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


WINDOWS = [
    (0x8007FCFC, 0x8007FE24, 74,
     "4bfd1c4982def754dbf78f982d594f8dd3659c93860a42b8e1e283c1c1229097"),
    (0x8007B558, 0x8007B964, 259,
     "4bbb224bcc04bcc21b6f1a7a931c46e916397e3cba4ece0735c379efef169ba0"),
    (0x8007B010, 0x8007B290, 160,
     "1dcf8dd320f2805cf0517141de33888ffea983621556942d9fb628c8f0b3279a"),
    (0x80073C5C, 0x80073C68, 3,
     "859ccf6879a1def400ddcd0fab5063a5b8ee05bd985e429028e3322b6e149fd9"),
    (0x80073DE8, 0x80073DF8, 4,
     "654978977236a51127fd4ae7076f9fa4e6895973af46c97566cf75b905fdc4c0"),
    (0x8007B9EC, 0x8007BAC0, 53,
     "7eb7df6063c7c885f0e1e494d68f0ecdc10c157c160e25a1b857389da3e19a1f"),
]

# (site, target): every jal in the six windows.  Asm comments print
# bytes MSB-first; targets below were decoded from the LE words.
JALS = [
    (0x8007FD14, 0x8007B9EC), (0x8007FD34, 0x80080950),
    (0x8007FDE4, 0x8007B558),
    (0x8007B5B4, 0x80071A74), (0x8007B608, 0x80071A74),
    (0x8007B618, 0x8007B010), (0x8007B714, 0x80073A44),
    (0x8007B76C, 0x80073A44), (0x8007B7BC, 0x80073C5C),
    (0x8007B808, 0x80071A74), (0x8007B810, 0x8007B9EC),
    (0x8007B82C, 0x80073DE8), (0x8007B854, 0x8007AAB4),
    (0x8007B044, 0x80073A44), (0x8007B090, 0x80073A44),
    (0x8007B0E0, 0x80073C5C), (0x8007B12C, 0x80071A74),
    (0x8007B134, 0x8007B9EC), (0x8007B150, 0x80073DE8),
    (0x8007B178, 0x8007AAB4),
]

# (site, word): load-bearing immediates as true LE words.
WORDS = [
    (0x8007FD70, 0x2403001E), (0x8007FD74, 0x240303C0),
    (0x8007FD80, 0x24020007), (0x8007FD8C, 0x24020008),
    (0x8007FD94, 0x24030001), (0x8007FD50, 0x9082FFC0),
    (0x8007FDCC, 0xAC80FFC8), (0x8007FDA0, 0x9083FFF3),
    (0x8007FDFC, 0x0801FF83), (0x8007FE04, 0xAE000044),
    (0x8007B71C, 0x244203C0), (0x8007B7A8, 0x3C02003C),
    (0x8007B928, 0x24020005), (0x8007B624, 0x24020002),
    (0x8007B62C, 0x2402000E), (0x8007B8F8, 0x24030007),
    (0x8007B8FC, 0x2405FFFF),
    # func_8007B558 prologue + argument moves, 0x8007B558..0x8007B5A0
    # (words read from the disc EXE; the SHA-1 check proves them).
    *((a, exe_word(a)) for a in range(0x8007B558, 0x8007B5A4, 4)),
    (0x8007B5DC, 0x1600000E),
    (0x8007B5C0, 0x00021880),
    (0x8007B06C, 0x244203C0), (0x8007B0CC, 0x3C02003C),
    (0x8007B068, 0x24130002), (0x8007B208, 0x304600FF),
    (0x8007B210, 0x24020005), (0x8007B230, 0x24030007),
    (0x8007B234, 0x2407FFFF), (0x8007B92C, 0x014620003),
    (0x80073C5C, 0x240A00B0), (0x80073C60, 0x01400008),
    (0x80073C64, 0x2409003F),
    (0x80073DE8, 0x3C028009), (0x80073DEC, 0x944245E6),
    (0x8007BAB4, 0x24021325), (0x8007BA1C, 0x24030007),
    (0x8007BA18, 0x24040001),
]


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
    for start, end, words, sha in WINDOWS:
        require((end - start) // 4 == words, f"{start:#x} size")
        require(window_sha(blob, start, end) == sha, f"{start:#x} sha")
    for site, target in JALS:
        require(jal_target(load_u32(blob, site)) == target,
                f"{site:#x} jal")
    for site, word in WORDS:
        require(load_u32(blob, site) == word, f"{site:#x} word")
    print("PASS: B558 windows, full jal chain, load-bearing immediates")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
