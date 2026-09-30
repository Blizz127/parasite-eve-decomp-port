#!/usr/bin/env python3
"""Original 14E30 movie-loader windows; disc/SDK/overlay callees are stops."""
import hashlib
import json
import struct
from pe_battle_hud_oracle import ROOT, execute


def main():
    ex = (ROOT / 'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(ex).hexdigest() == '452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    pin = hashlib.sha256(ex[0x14E30-0xF800:0x14FD8-0xF800]).hexdigest()
    assert pin == '3e11c6977f606fbb2b3ecf6cbce00a02ba1c6721419914bb01a2e791f84cd4a5'
    rows = []

    def ram():
        r = bytearray(0x200000)
        r[0x10000:0x10000+len(ex)-2048] = ex[2048:]
        return r

    def sw(r, a, v):
        struct.pack_into('<I', r, a & 0x1FFFFF, v & 0xFFFFFFFF)

    def lw(r, a):
        return struct.unpack_from('<I', r, a & 0x1FFFFF)[0]

    for flags in (0, 2, 0xFFFF7DFF, 0xFFFFFFFF):
        r = ram(); sw(r, 0xB0CD8, flags)
        regs = execute(r, 0x80014E30, (0x80150000,), stop_at=(0x80074DC0,))
        assert regs[4] == 0 and lw(r, 0xB0CD8) == flags | 0x8200
        rows.append(dict(window='display_disable_prefix', flags=flags, after=lw(r, 0xB0CD8)))
    for start, table, dest_global in ((0x80014E8C, 0x8009315E, 0x8001160C),
                                      (0x80014EFC, 0x80093160, 0x80011610)):
        for first, end in ((0, 1), (100, 107), (65530, 65535), (10, 9)):
            r = ram(); struct.pack_into('<HH', r, table & 0x1FFFFF, first, end)
            sw(r, 0xB0DD8, 12345); sw(r, dest_global, 0x80123400)
            regs = execute(r, start, stop_at=(0x8006E6A8,),
                           initial_regs={16:table, 18:0x800B0CD8})
            assert regs[4:7] == [12345+first, 0x80123400, (end-first)&0xFFFFFFFF]
            rows.append(dict(window=f'{start:08X}', first=first, end=end, arguments=regs[4:7]))
    for start, restart, ready, poll in ((0x80014EC0, 0x80014E80, 0x80014ED8, 0x80014EB8),
                                        (0x80014F30, 0x80014EF0, 0x80014F48, 0x80014F28)):
        for status in (0, 1, 2, 0xFFFFFFFF, 0x80000000):
            expected = ready if status == 0 else restart if status == 0xFFFFFFFF else poll
            # Guard unselected provider boundaries with invalid instructions.
            # Only the expected boundary may be reached; no provider is run.
            r = ram()
            for other in (restart, ready, poll):
                if other != expected: sw(r, other, 0xFFFFFFFF)
            execute(r, start, stop_at=(expected,), initial_regs={2:status, 16:0xFFFFFFFF})
            rows.append(dict(window=f'{start:08X}', status=status, stop=f'{expected:08X}'))
    for start, retry, poll in ((0x80014EAC, 0x80014E8C, 0x80014EB8),
                                (0x80014F1C, 0x80014EFC, 0x80014F28)):
        for result in (0, 1, 2, 0xFFFFFFFF, 0x80000000):
            expected = retry if result == 0xFFFFFFFF else poll
            r = ram(); sw(r, poll if expected == retry else retry, 0xFFFFFFFF)
            execute(r, start, stop_at=(expected,), initial_regs={2:result, 17:0xFFFFFFFF})
            rows.append(dict(window=f'{start:08X}', result=result, stop=f'{expected:08X}'))
    for first, end in ((0, 1), (100, 107), (65530, 65535)):
        r = ram(); struct.pack_into('<HH', r, 0x93160, first, end); sw(r, 0x11610, 0x80120000)
        regs = execute(r, 0x80014F60, stop_at=(0x801216C4,))
        assert regs[4] == 1 and regs[5] == 0x801FF010
        assert lw(r, regs[5]) == (0x80120000 + ((end-first)<<11)) & 0xFFFFFFFF
        assert lw(r, regs[5]+4) == 0
        rows.append(dict(window='overlay_initialize', first=first, end=end, descriptor=lw(r, regs[5])))
    for movie in (0, 8, 9, 10, 32767, 32768, 65535):
        r = ram(); sw(r, 0x150000, 0x80150010); struct.pack_into('<H', r, 0x150010, movie)
        regs = execute(r, 0x80014F98, stop_at=(0x80121C04,), initial_regs={19:0x80150000})
        assert regs[4] == (movie if movie < 32768 else movie-65536) & 0xFFFFFFFF
        rows.append(dict(window='movie_select', encoded=movie, argument=regs[4]))
    out = dict(source_sha256=pin, scope='closed original loader windows; external calls are boundaries, no disc/media execution', cases=rows)
    (ROOT / 'local/live/movie-loader-contract-132.json').write_text(json.dumps(out, indent=2)+'\n')
    print(f'PASS {len(rows)} original movie-loader contract cases')


if __name__ == '__main__':
    main()
