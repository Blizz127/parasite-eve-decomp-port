#!/usr/bin/env python3
"""Survey original GTE initialization with scripted BIOS and COP0 providers.

The original helper executes; BIOS entry/exit, BIOS B(56h), BIOS A(44h) and
COP0 status are explicit test-only providers. No original bytes are emitted.
"""
import hashlib
import random
import struct
from pathlib import Path

from pe_battle_hud_oracle import execute
from pe_cd_ack_oracle import Stops

ROOT = Path(__file__).resolve().parents[2]
EXPECTED_CONTROL = {24: 0, 25: 0, 26: 1000, 27: 0xffffef9e,
                    28: 0x01400000, 29: 0x155, 30: 0x100}
EXPECTED_CALLS = [(0x80072714, None), (0xb0, 0x56), (0xa0, 0x44),
                  (0x80072724, None)]


def main():
    exe = (ROOT / 'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest() == '452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    rng = random.Random(0x80077f7c)
    for case in range(24):
        ram = bytearray(0x200000)
        ram[0x10000:0x10000 + len(exe) - 0x800] = exe[0x800:]
        struct.pack_into('<I', ram, 0xa3450, case % 3)
        before = bytes(ram)
        control = {i: rng.getrandbits(32) for i in range(32)}
        data = {i: rng.getrandbits(32) for i in range(32)}
        initial_control, initial_data = control.copy(), data.copy()
        status = initial_status = rng.getrandbits(32)
        pc, state, calls = 0x80077f7c, None, []
        for _ in range(16):
            stop = Stops((0x80072714, 0x80072724, 0x80077f98, 0x80077fa4, 0xb0, 0xa0))
            stop.last = 0
            final = {}
            regs = execute(ram, pc, initial_regs=state, initial_cop_control=control,
                           initial_cop_data=data, final_gte=final, stop_at=stop)
            if not stop.last:
                break
            if stop.last in (0xb0, 0xa0):
                calls.append((stop.last, regs[9]))
                regs[2], pc = 1, regs[31]
            elif stop.last in (0x80072714, 0x80072724):
                calls.append((stop.last, None))
                regs[2], pc = (1 if stop.last == 0x80072714 else 0), regs[31]
            elif stop.last == 0x80077f98:
                regs[2], pc = status, stop.last + 4
            else:
                status, pc = regs[2], stop.last + 4
            state = dict(enumerate(regs))
            control = dict(enumerate(final['control']))
            data = dict(enumerate(final['data']))
        else:
            raise AssertionError('provider call budget exceeded')
        assert calls == EXPECTED_CALLS, (case, calls)
        assert status == (initial_status | 0x40000000), case
        assert final['data'] == list(initial_data.values()), case
        for reg in range(32):
            assert final['control'][reg] == EXPECTED_CONTROL.get(reg, initial_control[reg]), (case, reg)
        assert struct.unpack_from('<I', ram, 0xa3450)[0] == 0x80077f8c, case
        changed = [i for i, (a, b) in enumerate(zip(before, ram)) if a != b and i < 0x1fe000]
        assert all(0xa3450 <= address < 0xa3454 for address in changed), (case, changed[:8])
    print('PASS 24 original GTE initialization cases; BIOS and COP0 scripted, helper executed')


if __name__ == '__main__':
    main()
