#!/usr/bin/env python3
"""Black-box vectors for ResetCallback (0x80073C94) / InterruptCallback (0x80073CC4).

Runs the original code from the user's EXE under the test-only interpreter
(pe_battle_hud_oracle.execute, loaded with its stores routed through a trace
hook; the shared tool is not edited).  Hardware registers alias into low RAM
in that interpreter; the hook maps 0x1000-0x1FFF back to 1F801000h-1F801FFFh
and applies the psx-spx I_STAT rule (a write acknowledges: I_STAT &= value).
BIOS services the code reaches are trapped and logged, with psx-spx results:
A(13h) setjmp, A(72h) CdRemove, B(19h) HookEntryInt, B(5Bh) ChangeClearPad,
C(0Ah) ChangeClearRCnt, and the SYSCALL(01h)/(02h) veneers at
0x80072714/0x80072724.  Anything else stops the case loudly.

Stores to the dispatch tables (0x800945E4..0x8009461B, 0x8009568C..0x800956DF)
are also kept in event order.  Each JSON line is one case: initial I_MASK/I_STAT, whether the library block
starts dirty, the call sequence, the ordered event list (MMIO stores, BIOS
calls, returns) and every guest RAM word written outside the stack and the
setjmp buffer (address -> final value).  The setjmp buffer (0x8009461C, 0x30
bytes) holds interpreter-specific ra/sp/gp and is excluded by contract.
Also writes OUT.txt in the line format read by pe_irq_callback_vectors_check.c
(BIOS services the host does not have - setjmp, HookEntryInt, CdRemove - are
left out there).  Output stays outside the repo.
Usage: pe_irq_callback_vectors.py OUT.jsonl
"""
import inspect
import itertools
import json
import struct
import sys

import pe_battle_hud_oracle as _hud
from pe_battle_hud_oracle import ROOT

RESET, INTR = 0x80073C94, 0x80073CC4
ENTER, EXIT = 0x80072714, 0x80072724
RET = 0x00000F00
JMPBUF = (0x9461C, 0x9461C + 0x30)
LIB_BLOCK = (0x945E4, 0x945E4 + 0x41A * 4)

EVENTS = []
RAM_WRITES = {}


def _store(fmt, buf, a, v):
    n = struct.calcsize(fmt)
    if 0x1000 <= a < 0x2000:
        if a == 0x1070:                        # I_STAT: write acknowledges
            old = struct.unpack_from('<H', buf, a)[0]
            struct.pack_into('<H', buf, a, old & v & 0xFFFF)
        else:
            struct.pack_into(fmt, buf, a, v)
        EVENTS.append(['mmio', 0x1F800000 + a, n, v & 0xFFFFFFFF])
        return
    struct.pack_into(fmt, buf, a, v)
    if 0x945E4 <= a < 0x9461C or 0x9568C <= a < 0x956E0:   # dispatch tables: keep order
        EVENTS.append(['ram', 0x80000000 + a, n, v & ((1 << (8 * n)) - 1)])
    if a < 0x1F0000 and not JMPBUF[0] <= a < JMPBUF[1] and a >= 0x10000:
        for i in range(n):
            RAM_WRITES[0x80000000 + a + i] = None


_src = inspect.getsource(_hud.execute).replace('struct.pack_into(', 'STORE(')
_ns = dict(vars(_hud))
_ns['STORE'] = _store
exec(compile(_src, _hud.__file__, 'exec'), _ns)
execute = _ns['execute']


class Stops:
    def __init__(self):
        self.hit = None

    def __contains__(self, pc):
        if pc in (0xA0, 0xB0, 0xC0, RET, ENTER, EXIT):
            self.hit = pc
            return True
        return False


class Bios:
    def __init__(self):
        self.clear = [0, 0, 0, 0]
        self.irq_on = 0

    def service(self, tab, fn, r, ram):
        if tab == 'A' and fn == 0x13:          # setjmp(buf); returns 0
            struct.pack_into('<12I', ram, r[4] & 0x1FFFFF, r[31], r[29], r[30],
                             *r[16:24], r[28])
            return 0
        if tab == 'A' and fn == 0x72:          # CdRemove()
            return 0
        if tab == 'B' and fn == 0x19:          # HookEntryInt(addr)
            return 0
        if tab == 'B' and fn == 0x5B:          # ChangeClearPad(int)
            return 0
        if tab == 'C' and fn == 0x0A:          # ChangeClearRCnt(t,flag): old flag
            t = r[4] & 3
            old, self.clear[t] = self.clear[t], r[5]
            return old
        raise AssertionError('unmodelled BIOS %s(%02Xh)' % (tab, fn))


def call(ram, bios, entry, args):
    regs = {i: v for i, v in enumerate(args, 4)}
    regs[31] = RET
    pc = entry
    for _ in range(200):
        st = Stops()
        r = execute(ram, pc, (), stop_at=st, initial_regs=regs, instruction_budget=500000)
        if st.hit == RET:
            EVENTS.append(['ret', r[2] & 0xFFFFFFFF])
            return
        if st.hit in (ENTER, EXIT):
            enter = st.hit == ENTER
            EVENTS.append(['bios', 'SYS(%02Xh)' % (1 if enter else 2), []])
            old = bios.irq_on
            bios.irq_on = 0 if enter else 1
            r[2] = old if enter else 0
        else:
            tab = {0xA0: 'A', 0xB0: 'B', 0xC0: 'C'}[st.hit]
            EVENTS.append(['bios', '%s(%02Xh)' % (tab, r[9]), [x & 0xFFFFFFFF for x in r[4:6]]])
            r[2] = bios.service(tab, r[9], r, ram) & 0xFFFFFFFF
        regs = {i: v for i, v in enumerate(r)}
        pc = r[31]
    raise AssertionError('too many service calls')


def fixture(exe, imask, istat, dirty):
    t_addr, t_size = struct.unpack_from('<II', exe, 0x18)
    ram = bytearray(0x200000)
    ram[t_addr & 0x1FFFFF:(t_addr & 0x1FFFFF) + t_size] = exe[0x800:0x800 + t_size]
    if dirty:
        for a in range(LIB_BLOCK[0], LIB_BLOCK[1]):
            ram[a] = 0xA5
        struct.pack_into('<H', ram, LIB_BLOCK[0], 0)   # guard clear: reset must run
    struct.pack_into('<H', ram, 0x1074, imask)
    struct.pack_into('<H', ram, 0x1070, istat)
    return ram


def run_case(exe, case):
    EVENTS.clear()
    RAM_WRITES.clear()
    ram = fixture(exe, case['imask'], case['istat'], case['dirty'])
    initial = bytes(ram)
    bios = Bios()
    try:
        for entry, args in case['calls']:
            if entry == 'POKE_IMASK':          # I_MASK changed by other code between calls
                struct.pack_into('<H', ram, 0x1074, args[0])
                continue
            call(ram, bios, entry, args)
        case['fault'] = None
    except AssertionError as e:
        case['fault'] = str(e)
    case['events'] = list(EVENTS)
    words = sorted({a & ~3 for a in RAM_WRITES})
    case['ram'] = [[a, struct.unpack_from('<I', ram, a & 0x1FFFFF)[0],
                    struct.unpack_from('<I', initial, a & 0x1FFFFF)[0]] for a in words]
    return case


def cases():
    H1, H2 = 0x80130000, 0x80130100
    seqs = ((H1,), (H1, H1), (H1, 0), (0,), (H1, H2), (0, H1))
    for imask, istat, dirty in ((0, 0, 0), (0x0001, 0x0009, 0), (0x07FF, 0x07FF, 1), (0xFFFF, 0, 0)):
        base = dict(imask=imask, istat=istat, dirty=dirty)
        yield dict(base, calls=[[RESET, []]])
        yield dict(base, calls=[[RESET, []], [RESET, []]])
        for source, seq in itertools.product(range(12), seqs):
            yield dict(base, calls=[[RESET, []]] + [[INTR, [source, h]] for h in seq])
        yield dict(base, calls=[[RESET, []], [INTR, [1, H1]], [INTR, [2, H2]], [INTR, [1, 0]],
                                [INTR, [5, H1]], [INTR, [2, 0]]])
        # I_MASK rewritten between calls: read-back rule vs table rule.
        for poke, source, h in ((0x0001, 2, H1), (0x07FF, 2, 0), (0x0000, 4, H1), (0x0040, 1, H1)):
            yield dict(base, calls=[[RESET, []], [INTR, [6, H2]], ['POKE_IMASK', [poke]],
                                    [INTR, [source, h]]])
        yield dict(base, calls=[[RESET, []], [INTR, [11, H1]], [INTR, [2, H2]]])
        # Registration before any ResetCallback (guard clear), then reset.
        for source in (0, 2, 3, 5):
            yield dict(base, calls=[[INTR, [source, H1]], [INTR, [source, 0]], [RESET, []]])


COLLAPSED = {'A(13h)', 'B(19h)', 'A(72h)'}
HOST_NAME = {'B(5Bh)': 'CLEARPAD', 'C(0Ah)': 'CLEARRCNT', 'SYS(01h)': 'ENTER', 'SYS(02h)': 'EXIT'}


def text_case(k, c):
    out = ['CASE %d %04X %04X %d' % (k, c['imask'], c['istat'], c['dirty'])]
    for e, a in c['calls']:
        if e == 'POKE_IMASK':
            out.append('POKE %04X' % a[0])
        else:
            out.append('CALL %s %s' % ('RESET' if e == RESET else 'INTR',
                                       ' '.join('%08X' % x for x in (a + [0, 0])[:2])))
    for ev in c['events']:
        if ev[0] == 'mmio':
            out.append('MMIO %08X %d %08X' % tuple(ev[1:]))
        elif ev[0] == 'ret':
            out.append('RET %08X' % ev[1])
        elif ev[0] == 'ram':
            out.append('STORE %08X %d %08X' % tuple(ev[1:]))
        elif ev[1] not in COLLAPSED:
            out.append('BIOS %s %s' % (HOST_NAME[ev[1]], ' '.join('%08X' % x for x in (ev[2] + [0, 0])[:2])))
    out.extend('RAM %08X %08X %08X' % tuple(w) for w in c['ram'])
    out.append('END')
    return '\n'.join(out) + '\n'


def main():
    exe = (ROOT / 'build/disc1.candidate.exe').read_bytes()
    n = faults = 0
    txt = sys.argv[1].rsplit('.', 1)[0] + '.txt'
    with open(sys.argv[1], 'w') as out, open(txt, 'w') as tout:
        for k, case in enumerate(cases()):
            run_case(exe, case)
            n += 1
            faults += case['fault'] is not None
            out.write(json.dumps(case) + '\n')
            tout.write(text_case(k, case))
    print('cases', n, 'faults', faults)
    return 1 if faults else 0


if __name__ == '__main__':
    sys.exit(main())
