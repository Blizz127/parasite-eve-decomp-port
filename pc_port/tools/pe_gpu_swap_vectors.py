#!/usr/bin/env python3
"""Black-box probe/vector engine for guest 0x80076C34 (clean-room, 2026-11-25).
The original code (user's EXE) is only driven under the test-only
interpreter of pe_gpu_rows_vectors.py; this module adds a catch-all stop:
every transfer of control outside [0x80076C34, 0x80076EE4) except the
sentinel return address and the BIOS vectors is logged as a call
`C:PC_xxxxxxxx a0 a1 a2 a3` (so jal, jalr and jr to the library
jump-table words show up by target address) and returns a scripted value.
"""
import sys
sys.path.insert(0, __file__.rsplit('/', 1)[0])
import pe_gpu_rows_vectors as g

LO, HI = 0x80076C34, 0x80076EE4
RET = g.RET
rets_default = {}


class Stops2:
    def __init__(self):
        self.hit = None

    def __contains__(self, pc):
        if pc == RET or pc in (0xA0, 0xB0, 0xC0) or not (LO <= pc < HI):
            self.hit = pc
            return True
        return False


def run2(ram, args, model=None, rets=None, budget=60000, max_calls=40, entry=LO, names=None, after=None):
    """rets: {callee pc: [values]} consumed per call, last repeating (default [0])."""
    rets = rets or {}
    g.EVENTS.clear()
    g.MODEL.update(stat=[0], chcr=[0], t1=(0, 1), n={})
    if model:
        g.MODEL.update(model)
    used = {}
    regs = {i + 4: a for i, a in enumerate(args)}
    regs[31] = RET
    pc = entry
    for _ in range(max_calls + 1):
        st = Stops2()
        try:
            r = g.execute(ram, pc, (), stop_at=st, initial_regs=regs, instruction_budget=budget)
        except AssertionError as e:
            return list(g.EVENTS), 'SPIN' if str(e) == 'instruction budget' else 'FAULT'
        if st.hit == RET:
            return list(g.EVENTS), r[2] & 0xFFFFFFFF
        if st.hit in (0xA0, 0xB0, 0xC0):
            import struct
            sp = r[29] & 0x1FFFFF
            stk = struct.unpack_from('<4I', ram, sp + 16)
            g.EVENTS.append(('C', 'BIOS_%s%02X' % ({0xA0: 'A', 0xB0: 'B', 0xC0: 'C'}[st.hit], r[9] & 0xFF),
                             r[4] & 0xFFFFFFFF, r[5] & 0xFFFFFFFF, r[6] & 0xFFFFFFFF, r[7] & 0xFFFFFFFF)
                            + (('stk', stk[0], stk[1]) if (r[9] & 0xFF) == 0x3F else ()))
            r[2] = 0
            regs = {i: x for i, x in enumerate(r)}
            pc = r[31]
            continue
        name = (names or {}).get(st.hit) or 'PC_%08X' % st.hit
        k = used.get(st.hit, 0)
        used[st.hit] = k + 1
        lst = rets.get(st.hit, [0])
        g.EVENTS.append(('C', name, r[4] & 0xFFFFFFFF, r[5] & 0xFFFFFFFF, r[6] & 0xFFFFFFFF, r[7] & 0xFFFFFFFF))
        r[2] = lst[min(k, len(lst) - 1)] & 0xFFFFFFFF
        if after and st.hit in after:
            after[st.hit](k, ram)
        regs = {i: x for i, x in enumerate(r)}
        pc = r[31]
    return list(g.EVENTS), 'CALLS'


# ----------------------------------------------------------------- vectors
NAMES = {0x800773D0: 'G298C', 0x80073E10: 'SIM', 0x80073CF4: 'DMACB', 0x80077404: 'TO', 0x80076EE4: 'KICK'}
ZONE = g.ZONE
Q, R = 0x80095874, 0x80095878
BUSY, READY = 0x01000000, 0x04000000


def swap_cases(base, count=12000):
    out = []
    for k in range(count):
        seed = 0x76C34000 + k
        rng = g.Rng(seed)
        ram = bytearray(base)
        dbg = rng.pick((0, 0, 1, 1, 1, 2, 0x7F, 0x80, 0xFF))
        kind = rng.next() % 7
        r0 = rng.pick((0, 1, 2, 5, 61, 62, 63, rng.next() % 64))
        if kind == 0:                    # full ring (loop path)
            q0 = (r0 - 1) % 64
            if rng.next() & 1:
                r0 += 64 * (rng.next() % 3)
        elif kind <= 2:                  # empty
            q0 = r0
            if rng.next() % 4 == 0:
                q0 += 64 * (rng.next() % 3)
        else:
            q0 = (r0 + rng.pick((1, 1, 2, 3, 5, rng.next() % 64))) % 64
        g.put32(ram, Q, q0)
        g.put32(ram, R, r0)
        g.put32(ram, 0x80095758, rng.pick((0, 0, 0, 1, 0x100, 0x80000000, rng.word())))
        g.put32(ram, 0x80095754, rng.pick((0, 1, rng.word())))
        ram[0x9574D & 0x1FFFFF] = dbg
        g.put32(ram, 0x8009587C, rng.word())
        if rng.next() % 5 == 0:          # register pointers redirected into plain RAM
            g.put32(ram, 0x80095854, ZONE + 0x100)
            g.put32(ram, 0x80095860, ZONE + 0x200)
            g.put32(ram, ZONE + 0x100, READY | rng.word() if rng.next() & 1 else rng.pick((0, 0x08000000)))
            g.put32(ram, ZONE + 0x200, rng.pick((0, BUSY, 0xFFFFFFFF, rng.word())))
        argp = rng.pick((0x80100000, ZONE + 0x400, ZONE + 0x440))
        for j in range(0x60):
            g.put32(ram, ZONE + 0x400 + 4 * j, rng.word())
        copy = rng.pick((0, 1, 2, 3, 4, 5, 6, 7, 8, 0xC, 0x10, 0x20, 0x40, 0x50, 0x54, 0x58, 0x60, 0x7C, 0x100,
                         0xFFFFFFFC, 0xFFFFFFFF, 0x80000000)) if rng.next() % 4 else rng.next() % 0x60
        worker = rng.pick((0x80130000, 0x80130040, 0x80130100, rng.word() | 0x80000000))
        aux = rng.word()
        stat = [rng.pick((0, 0x08000000, 0x10000000, 0x1C000000, 0xFFFFFFFB)) for _ in range(rng.next() % 4)] + \
               [rng.pick((READY, 0x1C000000, 0xFFFFFFFF, READY | (rng.word() & 0xFBFFFFFF)))]
        chcr = [rng.pick((0, 0, BUSY, 0xFFFFFFFF, 0x00FFFFFF, 0x7F000000, rng.word())) for _ in range(rng.next() % 3 + 1)]
        to = [0] * (rng.next() % 3) + [rng.pick((0, 0xFFFFFFFF, 1, 0xAA, rng.word()))]
        rets = {'G298C': [rng.word()], 'SIM': [rng.word() & 0xFFFF], 'DMACB': [rng.word()], 'TO': to,
                'KICK': [rng.word()], 'PC_%08X' % worker: [rng.word()]}
        by_pc = {0x800773D0: 'G298C', 0x80073E10: 'SIM', 0x80073CF4: 'DMACB', 0x80077404: 'TO', 0x80076EE4: 'KICK',
                 worker: 'PC_%08X' % worker}
        rets_pc = {pc: rets[n] for pc, n in by_pc.items()}
        nmax = rng.next() % 3

        def kick_after(n, ram, q0=q0, r0=r0, nmax=nmax, rng=rng):
            if n + 1 >= nmax:             # the ring drains (consumer advances) after the nmax-th kick
                g.put32(ram, R, (q0 + rng.pick((0, 0, 1, 2))) & 0xFFFFFFFF)
        args = [worker, argp, copy, aux]
        ev, res = run2(ram, args, {'stat': stat, 'chcr': chcr, 't1': [0]}, rets_pc, budget=30000, max_calls=40,
                       names=by_pc, after={0x80076EE4: kick_after})
        if isinstance(res, str):
            continue
        bad = [e for e in ev if e[0] == 'S' and not (e[1] < 0x80200000 or 0x1F800000 <= e[1] < 0x1F802000)]
        if bad or any(e[0] == 'L' and e[1] >= 0x80200000 for e in ev):
            continue
        out.append(g.emit(LO, seed, args, rets, res, ev))
    return out


if __name__ == '__main__':
    base = g.fixture()
    n = int(sys.argv[2]) if len(sys.argv) > 2 else 12000
    lines = swap_cases(base, n)
    open(sys.argv[1], 'w').write('\n'.join(lines) + '\n')
    print('wrote', len(lines), 'of', n)
