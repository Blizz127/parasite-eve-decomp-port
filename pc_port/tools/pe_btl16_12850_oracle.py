#!/usr/bin/env python3
"""PE-BTL16 independent oracle: 12850 ALU and 1731C skip-if-false.

Pins SHA-1-exact EXE. 12850 is 244 words with a 24-entry table at
0x80010000. 1731C is 16 words. Does not import production C. Does
not invent bytecode. 17588 / 0x190 are not this cut.
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

from pe_exe_words import exe_word

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FN_9 = 0x80012850
END_9 = 0x80012C20
SHA_9 = "cec98712a4912a71ce016253f41e89b305029ac3bbc21dda24c608075b3cb37a"
JTBL = 0x80010000
FN_5 = 0x8001731C
END_5 = 0x8001735C
SHA_5 = "20d208e039ce2d4ef6a4a83d08c4f8151244b16f0aa105e4540c9051777b0b1f"
FN_3708C = 0x8003708C
END_3708C = 0x800370A8
SHA_3708C = "25935c1bd1c640de4ae2d963c79bfed6f1c938570ed54b0821fc1b644b78499b"
FN_370A8 = 0x800370A8
END_370A8 = 0x800370BC
SHA_370A8 = "23b727893c554ed869e76489d2b9d68edbab338828adbfd4b9af03e911620ec1"

# Case targets of the 0x18-entry jump table at 0x80010000 (.rdata head),
# read from the disc EXE.
CASES = {i: exe_word(0x80010000 + 4 * i) for i in range(0x18)}


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def window_sha(data: bytes, start: int, end: int) -> str:
    return hashlib.sha256(data[exe_off(start) : exe_off(end)]).hexdigest()


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = exe.read_bytes()
    require(hashlib.sha1(data).hexdigest() == SHA1, "EXE SHA-1")
    require((END_9 - FN_9) // 4 == 244, "12850 244 words")
    require(window_sha(data, FN_9, END_9) == SHA_9, "12850 sha")
    require(load_u32(data, 0x800910A0 + 0x9 * 4) == FN_9, "table[0x09]")
    require(load_u32(data, 0x80012870) == 0x2C620018, "sltiu 24")
    require(load_u32(data, 0x80012C08) == 0x24020001, "epilogue v0=1")
    require(load_u32(data, 0x800129E8) == 0x0044102A, "sub 0x09 slt b,a")
    for sub, va in CASES.items():
        require(load_u32(data, JTBL + sub * 4) == va, f"jtbl[{sub:#x}]")
    require(jal_target(load_u32(data, 0x80012B6C)) == FN_3708C, "jal 3708C")
    require(jal_target(load_u32(data, 0x80012B90)) == FN_370A8, "jal 370A8")
    require((END_3708C - FN_3708C) // 4 == 7, "3708C 7 words")
    require(window_sha(data, FN_3708C, END_3708C) == SHA_3708C, "3708C sha")
    require(load_u32(data, FN_3708C) == 0x00850018, "3708C mult")
    require((END_370A8 - FN_370A8) // 4 == 5, "370A8 5 words")
    require(window_sha(data, FN_370A8, END_370A8) == SHA_370A8, "370A8 sha")
    require(load_u32(data, FN_370A8) == 0x00052A03, "370A8 sra 8")
    require((END_5 - FN_5) // 4 == 16, "1731C 16 words")
    require(window_sha(data, FN_5, END_5) == SHA_5, "1731C sha")
    require(load_u32(data, 0x800910A0 + 0x5 * 4) == FN_5, "table[0x05]")
    require(load_u32(data, 0x8001732C) == 0x14400009, "1731C bnez")
    require(load_u32(data, 0x80017350) == 0xAF830090, "1731C sw gp+0x90")
    require(load_u32(data, 0x80017358) == 0x24020001, "1731C v0=1")
    print(
        "PASS: 12850 244w 24-entry ALU; 3708C 7w 16.16; 370A8 5w; "
        "1731C 16w skip-if-false v0=1"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
