#!/usr/bin/env python3
"""Selected original CD data-sync observations; test-only, no runtime CPU."""
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

    for wrapper in (0x8007A910, 0x80080B24):
        for mode in (0, 1, 0x12345678):
            regs = execute(memory(), wrapper, (mode,), stop_at=(0x8007BDDC,),
                           instruction_budget=200)
            assert regs[4] == mode, (hex(wrapper), mode, regs[4])

    for mode, busy, expected in ((0, 0, 0), (1, 0, 0), (1, 0x1000000, 1)):
        ram = memory()
        # Test-only synthetic VSync provider: return 5 without touching a device.
        struct.pack_into('<II', ram, 0x73A44, 0x03E00008, 0x24020005)
        struct.pack_into('<I', ram, 0x9B2C0, 0x8015FA00)
        struct.pack_into('<I', ram, 0x15FA00, busy)
        regs = execute(ram, 0x8007BDDC, (mode,), instruction_budget=2000)
        assert regs[2] == expected, (mode, busy, regs[2])
        assert struct.unpack_from('<III', ram, 0xA3478) == (0x3C5, 1, 0x80011C20)

    for mode in (0, 1):
        ram = memory()
        # Synthetic VSync advances 1024 on each call to force the deadline arm.
        struct.pack_into('<IIIIII', ram, 0x73A44, 0x3C088015, 0x8D021000,
                         0x24420400, 0xAD021000, 0x03E00008, 0)
        for target in (0x73C5C, 0x71A74):
            struct.pack_into('<II', ram, target, 0x03E00008, 0)
        struct.pack_into('<I', ram, 0x9B2C0, 0x8015FA00)
        struct.pack_into('<I', ram, 0x15FA00, 0x1000000)
        regs = execute(ram, 0x8007BDDC, (mode,), instruction_budget=2000)
        assert regs[2] == 0xFFFFFFFF, (mode, regs[2])
        assert struct.unpack_from('<III', ram, 0xA3478) == (0x7C0, 0, 0x80011C20)

    print('PASS: 11 selected CD data-sync original-code observations')


if __name__ == '__main__':
    main()
