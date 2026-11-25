#!/usr/bin/env python3
"""Check finite idle/request-ready DMA issuer cases against independent expectations.

Original instructions run in the test-only interpreter. The hardware registers are
synthetic MMIO; this is not a device or permanent-stall test.
"""
import argparse
import hashlib
import json
import random
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'pc_port/tools'))
from pe_movie_init_oracle import execute
from pe_cd_ack_oracle import Stops


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--channel3-ready-output', type=Path, help='write synthetic original outcomes to a local JSON file')
    parser.add_argument('--samples', type=int, default=12, help='random samples per channel and interrupt value')
    options = parser.parse_args()
    if options.samples < 1:
        parser.error('--samples must be positive')
    rows = []
    exe = (ROOT / 'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest() == '452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    rng = random.Random(0x8007ceac)
    failures = []
    cases = 0
    for ch in range(7):
        for interrupt in (0, 1, 2, 0x100, 0x101, 0xffffffff):
            for _ in range(options.samples):
                dicr, dpcr, dst, blocks, words, control = (rng.getrandbits(32) for _ in range(6))
                ram = bytearray(0x200000)
                ram[0x10000:0x10000 + len(exe) - 2048] = exe[2048:]

                def store(address, value):
                    struct.pack_into('<I', ram, address, value & 0xffffffff)

                store(0x9b348, 0x80130000)
                store(0x130000, dicr)
                store(0x9b344, 0x80130004)
                store(0x130004, dpcr)
                store(0x9b32c, 0x80130008)
                ram[0x130008] = 0x40
                store(0x1ff010, control)
                store(0x1ff014, interrupt)
                store(0x1ff018, 0)
                busy_delay, ready_delay = (0, 0) if options.channel3_ready_output else (cases % 5, (cases // 5) % 5)
                busy_reads, ready_reads = 0, 0
                original_unpack = struct.unpack_from

                def delayed_unpack(fmt, buf, offset=0):
                    nonlocal ready_reads
                    if buf is ram and offset == 0x130008 and fmt in ('<B', '<b'):
                        ready_reads += 1
                        return (0x40 if ready_reads > ready_delay else 0,)
                    return original_unpack(fmt, buf, offset)

                pc, initial = 0x8007ceac, None
                args = (ch, dst, blocks, words)
                mmio = {0x1f801088 + (ch << 4): 0}
                stops = Stops((0x8007ceec, 0x8007cf10, 0x8007cfdc, 0x8007cfe4, 0x8007d024, 0x8007d028))
                struct.unpack_from = delayed_unpack
                try:
                    while True:
                        stops.last = 0
                        regs = execute(ram, pc, args=args, initial_regs=initial, stop_at=stops)
                        if not stops.last:
                            break
                        at = stops.last
                        if at in (0x8007ceec, 0x8007cf10):
                            busy_reads += 1
                            regs[2] = 0x01000000 if busy_reads <= busy_delay else mmio[regs[2] + 0x1088]
                        elif at == 0x8007cfdc:
                            mmio[regs[5]] = regs[18]
                        elif at in (0x8007cfe4, 0x8007d024):
                            mmio[regs[5]] = regs[2]
                        else:
                            regs[2] = mmio[regs[5]]
                        pc, initial, args = at + 4, dict(enumerate(regs)), ()

                finally:
                    struct.unpack_from = original_unpack
                if busy_reads != busy_delay + 1 or ready_reads != ready_delay + 1:
                    failures.append(dict(channel=ch, interrupt=interrupt,
                                         busy_reads=busy_reads, ready_reads=ready_reads,
                                         busy_delay=busy_delay, ready_delay=ready_delay))
                mask = 1 << ch
                enable = (dicr >> 16) & 255
                enable = (enable | mask) if (interrupt & 255) == 1 else (enable & ~mask)
                expected = ((dicr & 0xff00ffff) | (enable << 16),
                            dpcr | (1 << (ch * 4 + 3)), dst,
                            ((blocks << 16) | words) & 0xffffffff, control)
                observed = (struct.unpack_from('<I', ram, 0x130000)[0],
                            struct.unpack_from('<I', ram, 0x130004)[0],
                            mmio[0x1f801080 + (ch << 4)],
                            mmio[0x1f801084 + (ch << 4)],
                            mmio[0x1f801088 + (ch << 4)])
                if options.channel3_ready_output and ch == 3:
                    rows.append([dicr, dpcr, dst, blocks, words, control, interrupt, *observed])
                cases += 1
                if observed != expected:
                    failures.append(dict(channel=ch, interrupt=interrupt,
                                         observed=observed, expected=expected))
    if options.channel3_ready_output:
        options.channel3_ready_output.write_text(json.dumps(rows) + '\n')
    print(json.dumps(dict(cases=cases, failures=failures[:3], failure_count=len(failures),
                          scope=('original DMA issuer; immediately ready synthetic MMIO' if options.channel3_ready_output
                                 else 'original DMA issuer; finite busy/readiness delays 0..4 reads, synthetic MMIO, seven channels')), indent=2))
    return bool(failures)


if __name__ == '__main__':
    sys.exit(main())
