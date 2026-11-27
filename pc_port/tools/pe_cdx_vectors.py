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

Usage: pe_cdx_vectors.py OUT.txt BATCH [ENTRY ...] [-n N] [-regonly] [-edge] [-variant NAME]
       (-variant: cdgen48 only; NAME in '' (inline, register level), bound (80085F44 / 80085E54 stopped), fault (stuck TSA / sticky SPUCNT hardware models))
       (-edge: cdgen46 only; the retry callees return arbitrary nonzero success values instead of the documented 1 / 0xF1000000+n)
       (-regonly: the register-level-only entries of the batch, which have no smoke checker; see REPORT)
       BATCH in cdgen42, cdgen42b, cdgen44, cdgen45, cdgen46, cdgen48 (cdgen43 is split between this file
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
PILOT = [False]       # True while discover() runs its pilot cases (hooks use random, not structured, seeding)

# ---- stack-window loads (cdgen42b fix, resolves cdgen42 lines 2982 / 2816 class) ----------------------------------
# pe_pad_leaf_vectors._logged hides every access in 0x1F0000..0x1FF000 (the interpreter's stack), and
# pe_cd_leaf_vectors only drops a case when the access lies outside 0x1FE000..0x1FF000.  A caller-supplied pointer
# into the original's own frame window (a response buffer on the caller's stack: the real runtime layout) was
# therefore read but never logged, so the vector said "no load" and a native that reads the buffer was failed
# as `read-outside-original-loads` (cdgen42 line 2982).  Fix: a load from 0x1FE000..0x1FFFFF is logged
# (as 0x801FExxx) when none of its bytes was stored earlier by the original in the same case (the original's own
# spills/locals stay out); a load that mixes own-written and external bytes drops the case.  Stores stay unlogged.
STACKW = (0x1FE000, 0x1FF000)
_OWN = set()
# ---- own-written RAM loads (cdgen46b fix; resolves the cdgen46 B414 artefact, docs cdx-spu2/REPORT.md) -------------
# A load of main-RAM bytes (0x2000 <= a < STACKW[0]) that the original ITSELF stored earlier in the same case reads
# its own result, not entry state.  Logging it made the checker seed the word with the post-store value (the first
# logged load of a word wins), so "store == value read" held trivially: the cdgen46 rows' 0x8009B414 readback right
# after 80085EB4's own `sh` seeded B414 with the value the original had just stored, and a provider that stores back
# the ENTRY value passed 400/400.  Fix: such a load is not logged when every byte was own-written (counted per entry
# in OWNLOAD, non-pilot cases only, and reported by main as own_written_loads=N); a load that mixes own-written and
# external bytes DROPS THE CASE (the cdgen42b rule; counted in MIXEDLOAD, reported as mixed_own_loads=N;
# CDX_KEEP_MIXED_OWN_LOADS=1 logs only the external bytes instead).  Register-page accesses (a < 0x2000) are hardware
# reads and stay as before.
OWNLOAD = {}
MIXEDLOAD = {}      # per entry: loads that mixed own-written and external bytes (non-pilot cases; the case is dropped by default)
KEEP_MIXED_OWN_LOADS = bool(os.environ.get('CDX_KEEP_MIXED_OWN_LOADS'))   # =1: log only the external bytes instead of dropping the case
LEGACY_OWN_LOADS = bool(os.environ.get('CDX_LEGACY_OWN_LOADS'))   # =1: reproduce the pre-fix sets (own-written loads logged)
_chain_load = pl._ns['LOAD']
_chain_store = pl._ns['STORE']


def _own_store(fmt, buf, a, val):
    if 0x2000 <= a < STACKW[1]:
        _OWN.update(range(a, a + struct.calcsize(fmt)))
    return _chain_store(fmt, buf, a, val)


def _own_load(fmt, buf, a):
    n0 = len(pl.LOADS)
    val = _chain_load(fmt, buf, a)
    if STACKW[0] <= a < STACKW[1] and not getattr(c, 'NO_STACK_LOADS', False):
        n = struct.calcsize(fmt)
        own = [(a + i) in _OWN for i in range(n)]
        if not any(own):
            pl.LOADS.append((0x80000000 + a, n, val[0] & 0xFFFFFFFF))
        elif not all(own):
            c.UNFAITHFUL[0] = True
    elif 0x2000 <= a < STACKW[0] and len(pl.LOADS) > n0 and not LEGACY_OWN_LOADS:
        n = struct.calcsize(fmt)
        own = [(a + i) in _OWN for i in range(n)]
        if any(own):
            ent = pl.LOADS.pop()
            if not PILOT[0]:
                OWNLOAD[pl.CUR[0]] = OWNLOAD.get(pl.CUR[0], 0) + 1
            if not all(own):
                if not PILOT[0]:
                    MIXEDLOAD[pl.CUR[0]] = MIXEDLOAD.get(pl.CUR[0], 0) + 1
                if KEEP_MIXED_OWN_LOADS:
                    raw = ent[2]
                    for i in range(n):
                        if not own[i]:
                            pl.LOADS.append((0x80000000 + a + i, 1, (raw >> (8 * i)) & 0xFF))
                else:
                    c.UNFAITHFUL[0] = True          # default (the cdgen42b rule): drop the case
    return val


pl._ns['STORE'] = _own_store
pl._ns['LOAD'] = _own_load
BIOS_STOPS = (0xA0, 0xB0, 0xC0)
CLASS32 = (0, 1, 2, 3, 0x7F, 0x80, 0xFF, 0x100, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0x10000, 0x12345678)
EDGE = (0, 1, 2, 3, 0x7F, 0x80, 0xFF, 0x100, 0x101, 0x7FFF, 0x8000, 0xFFFF, 0x10000, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)

# ---- register-level hardware models for cdgen48 (default off: empty dicts leave every other batch byte-identical) --------
# REGSEQ[halfword offset] = [values, reads]: the i-th 16-bit read of that register returns values[i] (last repeats), written
# into the register page just before the load (like MEMSEQ for plain words); the read is logged as usual (L), so the checker
# replays it in order.  STUCK = set of halfword offsets whose STORE is accepted by the CPU (logged in S) but not latched
# by the device (the readback keeps the old value): a harness model of a hardware fault, never original behaviour.
REGSEQ = {}
STUCK = set()
_prev_load = pl._ns['LOAD']
_prev_store = v._store


def _reg_load(fmt, buf, a):
    m = REGSEQ.get(a & ~1)
    if m is not None and struct.calcsize(fmt) <= 2:
        vals = m[0]
        struct.pack_into('<H', buf, a & ~1, vals[min(m[1], len(vals) - 1)] & 0xFFFF)
        m[1] += 1
    return _prev_load(fmt, buf, a)


def _reg_store(fmt, buf, a, val):
    if STUCK and (a & ~1) in STUCK:
        return
    return _prev_store(fmt, buf, a, val)


pl._ns['LOAD'] = _reg_load
v._store = _reg_store


def _reset_models():
    c.MEMSEQ.clear()
    REGSEQ.clear()
    STUCK.clear()
    c.SPU_STICKY[0] = 0

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
    _OWN.clear()
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
        r = v.execute(ram, pc, (), stop_at=st, initial_regs=regs, instruction_budget=sp.get('budget', 100000), **xkw)
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
            _OWN.update(range(a, a + n))
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
        if hit in sp.get('silent', ()):       # cdgen47: diagnostic-text callees (printf / puts) run as a silent stop, not recorded
            r[2] = 0
            regs = dict(enumerate(r))
            pc = r[31]
            continue
        n = sp['callees'][hit]
        if hit == 0x80071A74 and sp.get('printf_fmt'):
            # cdgen47: retail printf records the format pointer and exactly the arguments the format consumes (the other argument
            # registers are leftovers); the format string words are appended to the loads so the checker can count the conversions
            fa = r[4] & 0xFFFFFFFF
            n = 1 + min(3, c._printf_arity(ram, fa))
            a0_ = fa & 0x1FFFFC
            while a0_ < len(ram) - 4:
                pl.LOADS.append((0x80000000 + a0_, 4, struct.unpack_from('<I', ram, a0_)[0]))
                if 0 in ram[a0_:a0_ + 4] or (a0_ + 4 - (fa & 0x1FFFFC)) > 256:
                    break
                a0_ += 4
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
    PILOT[0] = True
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
            _reset_models()
            c.CdIrq.active = False
        for key2 in {((a & ~3) if sz < 4 else a, sz) for a, sz, _val in r[1] if _is_state(a)}:
            seen.setdefault(key2[0], {}).setdefault(key2[1], 0)
            seen[key2[0]][key2[1]] += 1
    # keep words read in >= 20% of the pilots (state words, not pointer-chased noise)
    PILOT[0] = False
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
            if os.environ.get('CDX_DEBUG_DROP'):
                print('drop %08X seed %d args %s: wild pointer' % (entry, seed, ' '.join('%X' % q for q in args)))
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
            _reset_models()
            c.CdIrq.active = False
        if c.UNFAITHFUL[0]:
            dropped += 1
            if os.environ.get('CDX_DEBUG_DROP'):
                print('drop %08X seed %d args %s: unfaithful access' % (entry, seed, ' '.join('%X' % q for q in args)))
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
    if '-variant' in argv:
        i = argv.index('-variant')
        globals()['VARIANT'] = argv[i + 1]
        del argv[i:i + 2]
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
            e, SPEC[e]['name'], len(o), spun, dropped, BREAKS.get(e, 0), len(DISC[e])) +
            (' own_written_loads=%d' % OWNLOAD[e] if OWNLOAD.get(e) else '') +
            (' mixed_own_loads=%d' % MIXEDLOAD[e] if MIXEDLOAD.get(e) else ''))
    open(outp, 'w').write('\n'.join(lines) + '\n')
    print('\n'.join(report))
    if hasattr(sys.modules[__name__], 'COV'):
        print('seed-class counts', {k: v_ for k, v_ in sorted(sys.modules[__name__].COV.items())})
        print('runtime tuples', {k: len(v_) for k, v_ in sys.modules[__name__].RT.items()})


if __name__ == '__main__':
    main()
