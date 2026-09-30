#!/usr/bin/env python3
"""Original 8006EC6C execution (sll 16 / sra 14 = (int16)a1 * 4)."""
import hashlib
import struct
import sys

from pe_battle_hud_oracle import ROOT, execute

SHA_EXE = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"


def main():
    exe = (ROOT / "build/disc1.candidate.exe").read_bytes()
    assert hashlib.sha1(exe).hexdigest() == SHA_EXE
    r = bytearray(0x200000)
    r[0x10000:0x10000 + len(exe) - 0x800] = exe[0x800:]
    struct.pack_into("<I", r, 0x141000, 0x0000000C)
    assert (execute(r, 0x8006EC6C, (0x80141000, 0))[2] & 0xFFFFFFFF) == 0x8014100C
    struct.pack_into("<I", r, 0x14100C, 0x20)
    assert (execute(r, 0x8006EC6C, (0x80141000, 3))[2] & 0xFFFFFFFF) == 0x80141020
    struct.pack_into("<I", r, 0x140FFC, 0x40)
    assert (execute(r, 0x8006EC6C, (0x80141000, 0xFFFFFFFF))[2] & 0xFFFFFFFF) == 0x80141040
    print("PASS: 3 original 6EC6C cases")


if __name__ == "__main__":
    main()
