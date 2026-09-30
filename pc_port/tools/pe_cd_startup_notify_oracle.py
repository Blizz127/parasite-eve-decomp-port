#!/usr/bin/env python3
"""Original startup-notification callback prefix; unknown targets stop at entry."""
import itertools, struct, sys
from pe_movie_init_oracle import load, ROOT, execute
from pe_cd_ack_oracle import Stops

def main():
    ex, _ = load()
    rows = []
    for target, status, response in itertools.product((0, 0x80170000), (0, 2, 5, 0x102, 0xFFFFFFFF), (0, 0x80140000)):
        ram = bytearray(0x200000)
        ram[0x10000:0x10000+len(ex)-2048] = ex[2048:]
        struct.pack_into('<I', ram, 0xB8AB8, target)
        before = ram[:0x1FE000]
        stops = Stops((0x80170000,))
        regs = execute(ram, 0x8007F960, args=(status, response), stop_at=stops)
        assert ram[:0x1FE000] == before
        assert bool(stops.last) == bool(target)
        rows.append((target, status, response, regs[4] if target else 0, regs[5] if target else 0))
    out = '/* Authenticated original 7F960 callback contracts. */\nstatic const struct { uint32_t target,status,response,arg0,arg1; } CDNOTIFY_cases[]={\n'
    out += ''.join('{' + ','.join(f'0x{x:X}u' for x in row) + '},\n' for row in rows) + '};\n'
    path = ROOT/'pc_port/tests/retail_cd_startup_notify_cases.h'
    if '--check' in sys.argv:
        assert path.read_text() == out
    else:
        path.write_text(out)
    print(f'PASS {len(rows)} original startup-notification graphs (10 complete,10 callback prefixes)')
if __name__ == '__main__':
    main()
