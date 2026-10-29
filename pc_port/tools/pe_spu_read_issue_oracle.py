#!/usr/bin/env python3
"""Selected original SPU read-issue observations; test-only, no runtime CPU."""
import argparse
import hashlib
import struct
from pathlib import Path

from pe_battle_hud_oracle import execute


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--exe', type=Path, default=Path('build/disc1.candidate.exe'))
    exe = parser.parse_args().exe.read_bytes()
    if hashlib.sha1(exe).hexdigest() != '452fb033f2eaa4b18aa20a5bca60b8125af3a37b':
        raise SystemExit('original EXE hash mismatch')

    def memory():
        ram = bytearray(0x200000)
        ram[0x10000:0x10000 + len(exe) - 0x800] = exe[0x800:]
        return ram

    for size, control, priority in ((1, 0xFFFF, 0xFFFFFFFF),
                                    (64, 0xA5A5, 0x12345678),
                                    (65, 0, 0), (0, 0, 0)):
        ram = memory()
        struct.pack_into('<I', ram, 0x9B3FC, 0x8015FA00)
        struct.pack_into('<I', ram, 0x9B424, 3)
        struct.pack_into('<I', ram, 0x9B410, 0x8015FC00)
        struct.pack_into('<H', ram, 0x15FBAA, control)
        struct.pack_into('<I', ram, 0x15FC00, priority)
        regs = execute(ram, 0x8007DA7C, (0x80150000, size), instruction_budget=3000)
        blocks = (size + 63) // 64
        assert regs[2] == size
        assert struct.unpack_from('<III', ram, 0x9B44C) == (1, 0x80150000, blocks)
        assert struct.unpack_from('<H', ram, 0x15FBAA)[0] == (control & 0xFFCF) | 0x30
        assert struct.unpack_from('<I', ram, 0x15FC00)[0] == (priority & 0xF0FFFFFF) | 0x22000000
        assert struct.unpack_from('<III', ram, 0x10C0) == (0x80150000, (blocks << 16) | 0x10, 0x01000200)

    for size, callback in ((64, 0), (64, 0x80150000), (0x90000, 0)):
        ram = memory()
        # Test-only low-level stub returns its second argument.
        struct.pack_into('<II', ram, 0x7DA7C, 0x03E00008, 0x00A01021)
        struct.pack_into('<I', ram, 0x9B434, callback)
        struct.pack_into('<I', ram, 0x9B430, 0x12345678)
        regs = execute(ram, 0x80085DF4, (0x80150000, size), instruction_budget=1000)
        assert regs[2] == min(size, 0x7EFF0)
        assert struct.unpack_from('<I', ram, 0x9B430)[0] == (0x12345678 if callback else 0)

    print('PASS: 7 selected SPU read-issue and wrapper observations')


if __name__ == '__main__':
    main()
