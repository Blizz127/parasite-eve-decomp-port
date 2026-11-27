#!/usr/bin/env python3
"""Black-box vectors for batch cdgen43: the libetc veneers / callback helpers
and the retail interrupt dispatchers.

Retail targets (public header bindings; names from include/pe_plat/*.h):
  0x800726C4 FlushCache (BIOS A(44h))          0x80073C74 ChangeClearPAD (B(5Bh))
  0x80073C84 ChangeClearRCnt (C(0Ah))          0x80072714 / 0x80072724 Enter / ExitCriticalSection (SYSCALL 1 / 2)
  0x80073E10 SetIntrMask (I_MASK exchange)     0x80073D24 VSyncCallback  0x80073D58 VSyncCallbacks
  0x80074478 VSyncCallbacks' table writer
  0x80073F00 CPU interrupt dispatcher (exits through B(17h) ReturnFromException)
  0x80074520 DMA interrupt handler (DICR walk)
This file reuses the trace machinery of pe_irq_callback_vectors.py (test-only
interpreter, I_STAT write-acknowledges, DICR flag rules after psx-spx, BIOS
services trapped and logged with a per-case state model) and adds new case
families: random (not hand-picked) interrupt controller states, handler
registrations, call sequences per entry, argument edges.  It must run in its
own process: both generator stacks patch the interpreter hooks.

Output (OUT.txt): the CASE / CALL / POKE... / MMIO / BIOS / CB / RET / STORE /
RAM / END line format read by pe_cdx_cdgen43_check.c (a superset of the
pe_irq_callback_vectors_check.c format).  Usage: pe_cdx_irq_vectors.py OUT.txt [-n N]
"""
import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pe_irq_callback_vectors as iv  # noqa: E402

_orig_service = iv.Bios.service


def _service(self, tab, fn, r, ram):
    if (tab, fn) == ('A', 0x44):        # FlushCache(): no result
        return 0
    return _orig_service(self, tab, fn, r, ram)


iv.Bios.service = _service
# libetc family: the BIOS model starts with interrupts enabled (psx-spx: EnterCriticalSection returns 1
# when interrupts were enabled).  The shared harness starts disabled; only this tool's libetc family changes it.
_orig_init = iv.Bios.__init__
BIOS_START_ENABLED = [False]


def _init(self):
    _orig_init(self)
    if BIOS_START_ENABLED[0]:
        self.irq_on = 1


iv.Bios.__init__ = _init
VOID_KINDS = {'FLUSH', 'CPAD', 'EXIT'}
KINDS = {
    0x800726C4: 'FLUSH', 0x80073C74: 'CPAD', 0x80073C84: 'CRCNT', 0x80072714: 'ENTER', 0x80072724: 'EXIT',
    0x80073E10: 'SETMASK', 0x80073D24: 'VSCB', 0x80073D58: 'VSCBS', 0x80074478: 'VSTBL',
    iv.DISP: 'DISP', iv.DMAH: 'DMAH', iv.RESET: 'RESET', iv.INTR: 'INTR', iv.DMACB: 'DMACB',
}
NARGS = {'FLUSH': 0, 'CPAD': 1, 'CRCNT': 2, 'ENTER': 0, 'EXIT': 0, 'SETMASK': 1, 'VSCB': 1, 'VSCBS': 2,
         'VSTBL': 2, 'DISP': 0, 'DMAH': 0, 'RESET': 0, 'INTR': 2, 'DMACB': 2}
ADDR = {v: k for k, v in KINDS.items()}
BIOS_NAME = {'B(5Bh)': 'CLEARPAD', 'C(0Ah)': 'CLEARRCNT', 'SYS(01h)': 'ENTER', 'SYS(02h)': 'EXIT',
             'A(3Fh)': 'PRINTF', 'A(44h)': 'FLUSH'}
COLLAPSED = {'A(13h)', 'B(17h)', 'B(18h)', 'B(19h)', 'A(72h)'}
E32 = (0, 1, 2, 3, 4, 7, 8, 0xF, 0x10, 0xFF, 0x100, 0x7FF, 0x800, 0xFFFF, 0x10000, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)
STUB = [0x80130000 + 0x40 * i for i in range(16)]            # callbacks: stubs in [CB_LO, CB_HI)
EFFECT = (0x80130800, 0x80130840, 0x80130880, 0x801308C0, 0x80130900)   # stubs with side effects (iv.CB_*)


def pick(rng, seq):
    return seq[rng.randrange(len(seq))]


def word(rng):
    return pick(rng, E32) if rng.random() < 0.5 else rng.getrandbits(32)


FAM = [""]


def seq_case(rng, entry_calls, **kw):
    kw.setdefault("fam", FAM[0])
    base = dict(imask=rng.choice((0, 1, 0x7FF, 0xFFFF, rng.getrandbits(16))), istat=rng.choice((0, 0x7FF, rng.getrandbits(11))),
                dirty=rng.choice((0, 0, 1)) if kw.get('fam') != 'libetc' else 0, dicr=rng.choice((0, rng.getrandbits(32) & 0x7FFFFFFF)))
    kw.pop('fam', None)
    base.update(kw)
    base['calls'] = entry_calls
    return base


def gen_cases(n, fam):
    rng = random.Random(0xC0FFEE43)
    out = []
    # libetc family: fresh state, no ResetCallback precondition (the native under test is only libetc);
    # irq family: dispatcher traces after ResetCallback
    pre = (lambda: [[iv.RESET, []]]) if fam == 'vsync' else (lambda: [])    # noqa: E731
    fam_libetc = fam == 'libetc'
    for _ in range(n if fam_libetc else 0):   # FlushCache: argument-less
        out.append(('FlushCache', seq_case(rng, pre() + [[0x800726C4, []]] * rng.randint(1, 3))))
    for _ in range(n if fam_libetc else 0):   # ChangeClearPAD(flag)
        out.append(('ChangeClearPAD', seq_case(rng, pre() + [[0x80073C74, [word(rng)]] for _ in range(rng.randint(1, 3))])))
    for _ in range(n if fam_libetc else 0):   # ChangeClearRCnt(counter, flag): per-counter state, returns the old flag
        out.append(('ChangeClearRCnt', seq_case(rng, pre() + [[0x80073C84, [pick(rng, (0, 1, 2, 3, 0, 1, 2, 3)),
                                                                          word(rng)]] for _ in range(rng.randint(1, 5))])))
    for _ in range(n if fam_libetc else 0):   # Enter / ExitCriticalSection: any order, nesting
        out.append(('Enter/ExitCriticalSection', seq_case(rng, pre() + [[pick(rng, (0x80072714, 0x80072724)), []] for _ in range(rng.randint(1, 6))])))
    for _ in range(n if fam_libetc else 0):   # SetIntrMask: returns the previous 16-bit mask
        calls = pre()
        for _k in range(rng.randint(1, 3)):
            if rng.random() < 0.3:
                calls.append(['POKE_IMASK', [rng.getrandbits(16)]])
            calls.append([0x80073E10, [word(rng)]])
        out.append(('SetIntrMask', seq_case(rng, calls)))
    for _ in range(0 if fam_libetc or fam == 'irq' else n):   # VSyncCallback(handler) / VSyncCallbacks(slot, handler)
        calls = pre()
        for _k in range(rng.randint(1, 4)):
            h = pick(rng, (0, 0, STUB[rng.randrange(16)], word(rng)))
            if rng.random() < 0.4:
                calls.append([0x80073D24, [h]])
            else:
                calls.append([0x80073D58, [pick(rng, (0, 1, 2, 3, 4, 5, 6, 7, rng.randrange(0x10))), h]])
        out.append(('VSyncCallback/VSyncCallbacks', seq_case(rng, calls)))
    for _ in range(0 if fam_libetc or fam == 'irq' else n):   # table writer 0x80074478(slot, handler)
        calls = pre()
        for _k in range(rng.randint(1, 4)):
            calls.append([0x80074478, [pick(rng, (0, 1, 2, 3, 4, 5, 6, 7, rng.randrange(8))),
                                       pick(rng, (0, STUB[rng.randrange(16)], word(rng)))]])
        out.append(('0x80074478', seq_case(rng, calls)))
    if fam in ('libetc', 'vsync'):
        return out
    out = []
    for _ in range(4 * n):   # CPU dispatcher: random registrations, pending sources, mask, watchdog counter
        calls = [[iv.RESET, []]] if rng.random() < 0.9 else []
        for src in rng.sample(range(11), rng.randint(0, 6)):
            calls.append([iv.INTR, [src, pick(rng, (STUB[src % 8] + 0x400, STUB[rng.randrange(16)], EFFECT[3], EFFECT[4], 0, STUB[1]))]])
        if rng.random() < 0.3:
            calls.append(['POKE_IMASK', [rng.getrandbits(16)]])
        if rng.random() < 0.3:
            calls.append(['POKE_W', [0x8009567C, pick(rng, (0, 1, 5, 0x7FF, 0x800, 0x801, 0xFFFFFFFF, rng.getrandbits(32)))]])
        if rng.random() < 0.1:
            calls.append(['POKE_W', [0x800945E8 + 4 * rng.randrange(11), 0]])
        calls.append(['POKE_ISTAT', [rng.getrandbits(11) if rng.random() < 0.8 else 0]])
        calls.append([iv.DISP, []])
        out.append(('0x80073F00 dispatcher', seq_case(rng, calls)))
    for _ in range(4 * n):   # DMA handler: random channel registrations, random DICR state
        calls = [[iv.RESET, []]]
        chans = rng.sample(range(7), rng.randint(0, 5))
        for ch in chans:
            calls.append([iv.DMACB, [ch, pick(rng, (STUB[ch], STUB[ch], EFFECT[0], EFFECT[1], EFFECT[2], 0))]])
        flags = sum(1 << (24 + c) for c in chans if rng.random() < 0.7) | (1 << 24 + rng.randrange(7) if rng.random() < 0.3 else 0)
        en = sum(1 << (16 + c) for c in range(7) if rng.random() < 0.7)
        dicr = flags | en | (rng.choice((0, 1 << 23, 1 << 23, 1 << 15)) if rng.random() < 0.9 else rng.getrandbits(16)) | \
            (rng.getrandbits(6) if rng.random() < 0.2 else 0)
        calls.append(['POKE_DICR', [dicr & 0xFFFFFFFF]])
        calls.append([iv.DMAH, []])
        out.append(('0x80074520 DMA handler', seq_case(rng, calls)))
    return out


def text_case(k, name, c):
    o = ['CASE %d %04X %04X %d %08X' % (k, c['imask'], c['istat'], c['dirty'], c.get('dicr', 0))]
    void_seq = [KINDS.get(e) in VOID_KINDS for e, a in c['calls'] if e in KINDS]
    for e, a in c['calls']:
        if e == 'POKE_IMASK':
            o.append('POKE %04X' % a[0])
        elif e == 'POKE_DICR':
            o.append('POKED %08X' % a[0])
        elif e == 'POKE_DPCR':
            o.append('POKEP %08X' % a[0])
        elif e == 'POKE_ISTAT':
            o.append('POKEI %04X' % a[0])
        elif e == 'POKE_W':
            o.append('POKEW %08X %08X' % (a[0], a[1]))
        else:
            kind = KINDS[e]
            o.append('CALL %s %s' % (kind, ' '.join('%08X' % x for x in (a + [0, 0])[:2])))
    for ev in c['events']:
        if ev[0] == 'mmio':
            o.append('MMIO %08X %d %08X' % tuple(ev[1:]))
        elif ev[0] == 'ret':
            o.append('RETV' if void_seq.pop(0) else 'RET %08X' % ev[1])
        elif ev[0] == 'ram':
            o.append('STORE %08X %d %08X' % tuple(ev[1:]))
        elif ev[0] == 'cb':
            o.append('CB %08X %08X %08X' % (ev[1], ev[2], ev[3]))
        elif ev[1] not in COLLAPSED:
            nargs = {'B(5Bh)': 1, 'C(0Ah)': 2, 'A(3Fh)': 4}.get(ev[1], 0)
            o.append('BIOS %s %s' % (BIOS_NAME[ev[1]], ' '.join('%08X' % x for x in (ev[2] + [0, 0, 0, 0])[:nargs])))
    o.extend('RAM %08X %08X %08X' % tuple(w) for w in c['ram'])
    o.append('END %s' % name.replace(' ', '_'))
    return '\n'.join(o) + '\n'


def main():
    argv = sys.argv[1:]
    fam = 'libetc'
    if '-fam' in argv:
        i = argv.index('-fam')
        fam = argv[i + 1]
        del argv[i:i + 2]
    n = 100
    if '-n' in argv:
        i = argv.index('-n')
        n = int(argv[i + 1])
        del argv[i:i + 2]
    exe = (iv.ROOT / 'build/disc1.candidate.exe').read_bytes()
    BIOS_START_ENABLED[0] = fam == 'libetc'
    FAM[0] = fam
    counts, faults = {}, {}
    with open(argv[0], 'w') as out:
        for k, (name, case) in enumerate(gen_cases(n, fam)):
            iv.run_case(exe, case)
            counts[name] = counts.get(name, 0) + 1
            if case['fault'] is not None:
                faults[name] = faults.get(name, 0) + 1
                continue            # a case the original itself cannot run (e.g. jump to address 0) is not a vector
            out.write(text_case(k, name, case))
    for name in counts:
        print('%-32s cases=%d faulted(dropped)=%d' % (name, counts[name], faults.get(name, 0)))


if __name__ == '__main__':
    main()
