#!/usr/bin/env python3
"""Execute retail Aya equipment binding and all its callees for regression data.

Uses original instructions, inventory lookup, stat tables, and ability jump
tables. No production function or rewritten equipment formula is the oracle.
"""
from pathlib import Path
import hashlib
import struct
from pe_battle_hud_oracle import execute, ROOT

RANGES = ((0x140000, 4), (0xB8A20, 0x70), (0xB0CB0, 0x18),
          (0x9D1B0, 8), (0xC0E06, 8), (0xA1B30, 28),
          (0x9D018, 4), (0x9D048, 4), (0x9D050, 4),
          (0x9D058, 4), (0x9D064, 4))


def fixture(exe, case):
    ram = bytearray(0x200000)
    ram[0x10000:0x10000+len(exe)-0x800] = exe[0x800:]
    def sw(a, v): struct.pack_into('<I', ram, a, v & 0xFFFFFFFF)
    def sh(a, v): struct.pack_into('<H', ram, a, v & 65535)
    for base, length in RANGES:
        ram[base:base+length] = bytes((i*37+19) & 255 for i in range(length))
    # All persistent globals begin as the native fixture's ResetTestState does.
    for a in (0x9D018, 0x9D048, 0x9D04C, 0x9D050, 0x9D054, 0x9D058, 0x9D064): sw(a, 0)
    for a in range(0xA1B30, 0xA1B4C, 4): sw(a, 0)
    sw(0x9D254, 0x80140000)
    sw(0xA8038, 0x150000-0xA8028); sw(0xA803C, 0x152000-0xA8028)
    for t in range(7):
        sh(0xC0E28+t*2, 20)
        for j in range(128): sw(0x150000+t*512+j*4, j*10)
    for j in range(99):
        base = 0x152000+j*24
        sh(base, 45+j); sh(base+2, 20+j); sh(base+4, 20+j)
        ram[base+6:base+8] = bytes((25+j, 10+j))
        sw(base+8, 80*65536); sw(base+12, 3*65536); sw(base+16, 2*65536)
        sh(base+20, 7+j); sh(base+22, 9+j)
    ram[0xC0E0A], ram[0xC0E20], ram[0xC0E22] = case, 0, 1
    if case == 32: ram[0xC0E22] = 255  # no armor
    sh(0xC0E48, 0x100); sh(0xC0E4A, 0x101)
    for n in range(2):
        a = 0xC0EAC+n*32
        ram[a:a+32] = bytes(32)
        ram[a+6:a+10] = bytes((case % 24, 10+case, 80+case, 75+case))
        sh(a+10, 0x4321+case)
        sh(a+14, (1100 if case % 3 == 0 else -100 if case % 3 == 1 else 15))
        sh(a+16, (1100 if case % 4 == 0 else -100 if case % 4 == 1 else 15))
        sh(a+18, (1100 if case % 5 == 0 else -100 if case % 5 == 1 else 0))
        abilities = [case] if case < 32 else [1, 6, 8, 16, 17, 19, 20, 31]
        ram[a+20] = len(abilities)
        ram[a+21:a+21+len(abilities)] = bytes(abilities)
    # Retail weapon fire count table is an explicit input to both fixtures.
    ram[0x923D0:0x923D6] = bytes((0, 1, 2, 3, 5, 10))
    return ram


def fingerprint(ram):
    h = 14695981039346656037
    for start, length in RANGES:
        for b in ram[start:start+length]: h = ((h ^ b)*1099511628211) & 0xFFFFFFFFFFFFFFFF
    return h


def main():
    exe = (ROOT/'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest() == '452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    for case in range(33):
        ram = fixture(exe, case)
        execute(ram, 0x8002F76C, (0x80140000,))
        print(f'UINT64_C(0x{fingerprint(ram):016X}), /* {case} */')
    print('PASS: full retail equipment binding, stat updates and ability commands')


if __name__ == '__main__': main()
