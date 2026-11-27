#!/usr/bin/env python3
"""Black-box vectors for the CD-stream / IRQ-delivery / libetc / libsnd /
save-manager family (batches cdgen42 .. cdgen46; cdgen46 = func_80085644 stream bring-up, needs DIV/DIVU in the interpreter).

Original code from the user's EXE runs under the test-only interpreter that
pe_cd_leaf_vectors.py builds on (this file imports it and reuses its case
runner conventions, record zone, register models and line format; the
other generators are not edited).  Nothing here reads a provider.

What is new relative to pe_cd_leaf_vectors.py:
 * every entry is described by a spec (argument kinds, the set of stopped
   callees with their argument counts, BIOS A/B/C veneers, SYSCALL stops);
 * the guest words an entry reads are DISCOVERED by executing it with
   load logging over pilot cases (a probe), then seeded per case from a value
   class table (0 / 1 / 2 / byte / edges / zone pointer / random / EXE value),
   so every state word the function reads is exercised, not just the ones
   the author thought of;
 * callee returns are scripted per case (0, 1, -1, random, plus byte/-N sweeps);
 * a stopped BIOS veneer (jump to 0xA0 / 0xB0 / 0xC0) is recorded as the
   pseudo callee "A00000nn" / "B00000nn" / "C00000nn" (nn = $t1) and a SYSCALL
   instruction as "5C0000nn" (nn = $a0);
 * a case may script a hardware read model (CD index/interrupt flags, a
   timer counter, a plain-memory wait word); every register read is replayed
   by the checker in order and every register store is compared in order.

Line format (same as pe_cd_leaf_vectors.py, one case per line):
  ENTRY seed 0 A args RET r L loads S stores RETS r1,r2,.. CALL tok@n ...
CALL tokens are `fn,arg..@n`: fn is the callee entry (or pseudo callee), the
arguments are only those the spec says the callee takes (leftover argument
registers are not compared) and n is how many stores the original had made
before the call (the checker compares distinct bytes).

Usage: pe_cdx_vectors.py OUT.txt BATCH [ENTRY ...] [-n N] [-regonly] [-edge]
       (-edge: cdgen46 only; the retry callees return arbitrary nonzero success values instead of the documented 1 / 0xF1000000+n)
       (-regonly: the register-level-only entries of the batch, which have no smoke checker; see REPORT)
       BATCH in cdgen42, cdgen44, cdgen45, cdgen46 (cdgen43 is split between this file
       for the libetc veneers and pe_cdx_irq_vectors.py for the IRQ dispatcher).
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pe_cd_leaf_vectors as c   # noqa: E402  (monkey-patches the shared interpreter hooks)
from pe_battle_hud_oracle import MipsBreak   # noqa: E402  (BREAK reached: the original traps; not reproducible)

gv = c.gv
pl = c.pl
v = c.v
ZONE = c.ZONE
POOL_RET = (0, 1, 0xFFFFFFFF)
BIOS_STOPS = (0xA0, 0xB0, 0xC0)
CLASS32 = (0, 1, 2, 3, 0x7F, 0x80, 0xFF, 0x100, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0x10000, 0x12345678)
EDGE = (0, 1, 2, 3, 0x7F, 0x80, 0xFF, 0x100, 0x101, 0x7FFF, 0x8000, 0xFFFF, 0x10000, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)

# ------------------------------------------------ public-header aliases
# `#define func_X pe_plat_name` in include/pe_plat/*.h: several retail entries bound to one
# host symbol are one callee as far as a provider can tell; vectors name it by the lowest address.
def _load_aliases():
    import glob
    import re
    root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'include', 'pe_plat')
    by = {}
    for f in glob.glob(os.path.join(root, '*.h')):
        for line in open(f):
            m = re.match(r'\s*#\s*define\s+func_([0-9A-Fa-f]{8})\s+(\w+)\s*$', line)
            if m:
                by.setdefault(m.group(2), set()).add(int(m.group(1), 16))
    out = {}
    for addrs in by.values():
        lo = min(addrs)
        for a in addrs:
            out[a] = lo
    return out


ALIAS = _load_aliases()


def canon(a):
    return ALIAS.get(a, a)


# ----------------------------------------------------------------- specs
# kinds: one char per argument: b byte, h halfword, w word, p zone pointer,
#        e edge word, n small count (0..40 / edges)
# callees: {entry: arity} stopped and recorded; arity None = take no arguments
SPEC = {}
HOOKS = {}


def spec(entry, name, kinds, callees=None, bios=False, **kw):
    SPEC[entry] = dict(name=name, kinds=kinds, callees=dict(callees or {}), bios=bios, **kw)


def syscall_pcs(base, entry):
    out = []
    for a in range(entry, c._func_end(entry), 4):
        w = struct.unpack_from('<I', base, a & 0x1FFFFF)[0]
        if w & 0xFC00003F == 0x0C:
            out.append(a)
    return out


# ------------------------------------------------------------ the runner
class Stops:
    def __init__(self, pcs):
        self.pcs, self.hit = pcs, None

    def __contains__(self, pc):
        if pc in self.pcs:
            self.hit = pc
            return True
        return False


def run_case(ram, entry, args, rets, sp, sysc, models=None):
    """Run entry; stop at callees, BIOS veneers and SYSCALLs.  Returns
    (ret, loads, stores, calls[(token_fields, stores_before)])."""
    pl.LOADS.clear()
    pl.STORES.clear()
    pl.CUR[0] = entry
    calls = []
    regs = {i: a for i, a in enumerate(args[:4], 4)}
    for i, a in enumerate(args[4:]):
        struct.pack_into('<I', ram, 0x1FF010 + 4 * i, a & 0xFFFFFFFF)
    regs[31] = v.RET
    pc = entry
    silent = [0]
    xkw = {'div_zero_traps': False} if sp.get('mips_div') else {}   # R3000 divide-by-zero (no trap), cdgen46
    pcs = set(sp['callees']) | {v.RET} | set(sysc)
    if sp['bios']:
        pcs |= set(BIOS_STOPS)
    for _ in range(c.CALL_CAP[0]):
        st = Stops(pcs)
        r = v.execute(ram, pc, (), stop_at=st, initial_regs=regs, instruction_budget=100000, **xkw)
        hit = st.hit
        if hit == v.RET or hit is None:
            return r[2] & 0xFFFFFFFF, list(pl.LOADS), list(pl.STORES), calls
        if hit in sysc:
            fn, n = 0x5C000000 | (r[4] & 0xFF), 1
            rec = [fn, r[4] & 0xFFFFFFFF]
            calls.append((rec[:1 + 0], len(pl.STORES)))     # arguments: none beyond the number
            r[2] = rets.get(0x5C000000, 0)
            regs = dict(enumerate(r))
            pc = hit + 4
            continue
        if hit == 0xA0 and (r[9] & 0xFF) == 0x28:
            # BIOS A(28h) bzero(addr, size): a memory effect, not a call.  Modelled here so the zero stores are
            # logged (as word stores) and appear in the expected RAM image; a provider writes the zeros itself.
            a, n = r[4] & 0x1FFFFF, r[5] & 0xFFFFFFFF
            if a + n > len(ram):
                raise IndexError('bzero out of RAM')
            ram[a:a + n] = bytes(n)
            for o in range(0, n, 4):
                pl.STORES.append((0x80000000 + a + o, min(4, n - o), 0))
            r[2] = r[4]
            regs = dict(enumerate(r))
            pc = r[31]
            continue
        if hit in BIOS_STOPS and (hit, r[9] & 0xFF) in sp.get('bios_silent', ()):
            silent[0] += 1
            if silent[0] > 3:
                raise AssertionError('instruction budget (diagnostic loop)')
            r[2] = 0                       # diagnostic text (printf / puts): not an observable
            regs = dict(enumerate(r))
            pc = r[31]
            continue
        if hit in BIOS_STOPS:
            fn = {0xA0: 0xA0000000, 0xB0: 0xB0000000, 0xC0: 0xC0000000}[hit] | (r[9] & 0xFF)
            n = sp['callees'].get(fn, sp.get('bios_arity', {}).get(fn, 0))
            calls.append(([fn] + [x & 0xFFFFFFFF for x in r[4:4 + n]], len(pl.STORES)))
            r[2] = rets.get(fn, 0)
            regs = dict(enumerate(r))
            pc = r[31]
            continue
        n = sp['callees'][hit]
        rec = [canon(hit)] + [x & 0xFFFFFFFF for x in r[4:4 + min(n or 0, 4)]]
        for k in range(4, n or 0):          # o32: arguments 5.. are in the caller's outgoing area at sp+0x10 (cdgen46)
            rec.append(struct.unpack_from('<I', ram, (r[29] + 0x10 + 4 * (k - 4)) & 0x1FFFFF)[0])
        m = (models or {}).get(hit)
        calls.append((rec, len(pl.STORES)))
        r[2] = (m(ram, r) if m else rets.get(canon(hit), 0)) & 0xFFFFFFFF
        regs = dict(enumerate(r))
        pc = r[31]
    raise AssertionError('instruction budget (callees)')


# ------------------------------------------------- discovery and seeding
DISC = {}
BREAKS = {}     # per entry: cases dropped because the original reached BREAK (reported by main)


def fmt_ret(val):
    """RETS token value: one word, or a per-call sequence `a/b/c` (the last value repeats once exhausted)."""
    return '/'.join('%X' % q for q in val) if isinstance(val, list) else '%X' % val


def _is_state(a):
    """A discovered load worth seeding: data words and registers (not code, not the record zone, not stack)."""
    if 0x1F801000 <= a < 0x1F803000:
        return True
    return 0x80090000 <= a < 0x80100000 or 0x800BE000 <= a < 0x80100000 and False


def discover(base, entry, sp, pilots=100):
    """Probe: run pilot cases and collect the guest words the function reads."""
    seen = {}
    sysc = syscall_pcs(base, entry)
    for seed in range(1, pilots + 1):
        s = seed * 104729 ^ entry
        ram, x, ring, size = c.prep(base, s)
        args = []
        for k in sp['kinds']:
            x = gv.xs32(x)
            args.append(_arg(k, x))
        rets = {canon(cc): (0, 1, 0xFFFFFFFF, gv.xs32(x + 7 * i))[(x >> i) % 4] for i, cc in enumerate(sp['callees'])}
        c.UNFAITHFUL[0] = False
        hook = HOOKS.get(entry)
        models = None
        if hook:
            x, models = hook(ram, x, args, rets, seed)
        try:
            r = run_case(ram, entry, args, rets, sp, sysc, models)
        except (AssertionError, struct.error, IndexError):   # includes MipsBreak
            continue
        finally:
            c.MEMSEQ.clear()
            c.CdIrq.active = False
        for key2 in {((a & ~3) if sz < 4 else a, sz) for a, sz, _val in r[1] if _is_state(a)}:
            seen.setdefault(key2[0], {}).setdefault(key2[1], 0)
            seen[key2[0]][key2[1]] += 1
    # keep words read in >= 20% of the pilots (state words, not pointer-chased noise)
    DISC[entry] = sorted((a, set(d)) for a, d in seen.items() if sum(d.values()) >= pilots // 5)
    return DISC[entry]


def _arg(k, x):
    if k == 'b':
        return x & 0xFF if x & 1 else EDGE[(x >> 8) % len(EDGE)] & 0xFF
    if k == 'h':
        return x & 0xFFFF if x & 1 else EDGE[(x >> 8) % len(EDGE)] & 0xFFFF
    if k == 'w':
        return EDGE[(x >> 8) % len(EDGE)] if x & 1 else gv.xs32(x)
    if k == 'e':
        return EDGE[(x >> 8) % len(EDGE)]
    if k == 'n':
        return (x >> 8) % 41 if x & 3 else EDGE[(x >> 8) % len(EDGE)]
    return ZONE + (x % 0x400) * 4 if x % 4 else gv.xs32(x)       # 'p'


def seed_state(ram, x, entry):
    """Seed every discovered word from a value class (EXE value kept 1 time in 6).
    A word whose EXE value is a register / RAM / code address is a pointer or a
    callback slot (the CD register pointers 0x8009B27C.., hook tables): it keeps its
    EXE value, since the register map is a hardware fact, not state."""
    for a, sizes in DISC.get(entry, ()):
        x = gv.xs32(x)
        if x % 6 == 0:
            continue
        sz = max(sizes)
        off = (a & 0x1FFFFF) if a >= 0x80000000 else (a & 0x1FFF)
        if sz == 4 and a >= 0x80000000:
            cur0 = struct.unpack_from('<I', ram, off & ~3)[0]
            if 0x1F800000 <= cur0 < 0x1F810000 or 0x80000000 <= cur0 < 0x80200000:
                continue
        k = (x >> 4) % 6
        if sz == 1:
            val = (0, 1, 2, 0x7F, 0x80, 0xFF, x >> 12 & 0xFF)[(x >> 8) % 7]
            ram[off] = val
        elif sz == 2:
            val = (0, 1, 2, 0x7FFF, 0x8000, 0xFFFF, x >> 12 & 0xFFFF)[(x >> 8) % 7]
            struct.pack_into('<H', ram, off & ~1, val)
        else:
            val = (ZONE + (x >> 10) % 0x400 * 4) if k == 5 else (x >> 8 & 0x3F) if k == 4 else \
                CLASS32[(x >> 8) % len(CLASS32)] if k < 4 else x
            struct.pack_into('<I', ram, off & ~3, val & 0xFFFFFFFF)
    return x


def cases(base, entry, n=200, extra=None):
    sp0 = SPEC[entry]
    sp = SPEC[entry]
    sysc = syscall_pcs(base, entry)
    c._check_callees_complete(base, entry, tuple(sp['callees']) + tuple(sp.get('inline', ())),
                              inline=tuple(sp.get('inline', ())))
    discover(base, entry, sp)
    out, spun, dropped = [], 0, 0
    seed = 0
    while len(out) < n and seed < 8 * n:
        seed += 1
        s = seed * 7919 ^ entry
        ram, x, ring, size = c.prep(base, s)
        x = seed_state(ram, x, entry)
        args = []
        for k in sp['kinds']:
            x = gv.xs32(x)
            args.append(_arg(k, x))
        x = gv.xs32(x)
        rets = {}
        for i, cc in enumerate(sp['callees']):
            x = gv.xs32(x)
            cc = canon(cc)
            rets[cc] = (0, 1, 0xFFFFFFFF, gv.xs32(x))[x % 4]
            if seed % 3 == 0:
                k = (seed // 3 + 37 * i) % 0x110
                rets[cc] = k if k < 0x100 else (0xFF - k) & 0xFFFFFFFF
        if sysc:
            rets[0x5C000000] = (0, 1, 0xFFFFFFFF)[x % 3]
        if sp['bios']:
            for key in tuple(f for f in sp['callees'] if f >> 28 in (0xA, 0xB, 0xC)):
                x = gv.xs32(x)
                rets[key] = (0, 1, 0xFFFFFFFF, gv.xs32(x))[x % 4]
        hook = HOOKS.get(entry)
        models = None
        if hook:
            x, models = hook(ram, x, args, rets, seed)
        c.UNFAITHFUL[0] = False
        try:
            res = run_case(ram, entry, args, rets, sp, sysc, models)
        except (struct.error, IndexError):
            dropped += 1                   # wild pointer outside the 2 MiB RAM image
            continue
        except MipsBreak:
            BREAKS[entry] = BREAKS.get(entry, 0) + 1   # retail raises a BREAK exception (e.g. divide by a zero word)
            dropped += 1
            continue
        except AssertionError as e:
            if 'instruction budget' not in str(e):
                raise
            spun += 1
            continue
        finally:
            c.MEMSEQ.clear()
            c.CdIrq.active = False
        if c.UNFAITHFUL[0]:
            dropped += 1
            continue
        ret, loads, stores, calls = res
        text = ' '.join(','.join('%08X' % y for y in rec) + '@%d' % m for rec, m in calls) or '-'
        order = tuple(dict.fromkeys(canon(k) for k in sp['callees'])) + tuple(k for k in rets if k not in {canon(q) for q in sp['callees']})
        out.append(gv.line(entry, s, 0, args, (ret, loads, stores)) +
                   ' RETS %s CALL %s' % (','.join('%08X=%s' % (k, fmt_ret(rets[k])) for k in order) or '-', text))
    return out, spun, dropped


def main():
    argv = sys.argv[1:]
    n = 200
    if '-n' in argv:
        i = argv.index('-n')
        n = int(argv[i + 1])
        del argv[i:i + 2]
    EDGE46 = '-edge' in argv          # cdgen46: success values other than the documented ones (see pe_cdx_specs)
    if EDGE46:
        argv.remove('-edge')
        globals()['EDGE46'] = True
    regonly = '-regonly' in argv
    if regonly:
        argv.remove('-regonly')
    outp, batch = argv[0], argv[1]
    from pe_cdx_specs import load_specs   # per-batch tables
    load_specs(sys.modules[__name__], batch)
    entries = [int(a, 16) for a in argv[2:]] or [e for e, s in SPEC.items() if s['batch'] == batch
                                                  and bool(s.get('regonly')) == regonly and not s.get('unsupported')]
    exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
    base = bytes(v.fixture(exe, 0, 0, 0))
    lines, report = [], []
    for e in entries:
        o, spun, dropped = cases(base, e, max(n, SPEC[e].get('n', 0)))
        lines += o
        report.append('%08X %s cases=%d spun=%d dropped=%d (break=%d) discovered_words=%d' % (
            e, SPEC[e]['name'], len(o), spun, dropped, BREAKS.get(e, 0), len(DISC[e])))
    open(outp, 'w').write('\n'.join(lines) + '\n')
    print('\n'.join(report))


if __name__ == '__main__':
    main()
