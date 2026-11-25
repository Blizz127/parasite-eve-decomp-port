#!/usr/bin/env python3
"""Observe ResetGraph's helper only until it leaves its three-word span.

The owner supplies the verified executable. BIOS services and any target
outside this span are not executed, including held SDK bodies.
"""
import argparse
import hashlib
from pathlib import Path

import pe_cd_leaf_vectors as c

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--exe', required=True, type=Path)
args = parser.parse_args()
exe = args.exe.read_bytes()
assert hashlib.sha1(exe).hexdigest() == '452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
base = bytes(c.v.fixture(exe, 0, 0, 0))

class Outside:
    def __init__(self):
        self.hit = None

    def __contains__(self, pc):
        if not 0x80077a54 <= pc < 0x80077a60:
            self.hit = pc
            return True
        return False

for value in (0, 1, 0x140000, 0xffffff, 0xffffffff):
    ram, _, _, _ = c.prep(base, 1)
    stop = Outside()
    result = c.v.execute(ram, 0x80077a54, (), stop_at=stop,
                         initial_regs={4: value, 31: c.v.RET}, instruction_budget=256)
    assert stop.hit == 0xa0 and result[4] & 0xffffffff == value
    assert result[9] & 0xffffffff == 0x49
    print(f'arg={value:08x} next_pc={stop.hit:08x} selector={result[9]:08x}')
print('PASS five bounded cases; BIOS service and all out-of-span targets not executed')
