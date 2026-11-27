#!/usr/bin/env python3
"""Audit of black-box vector files / generators for the "own-written read" seeding artefact.

The artefact: a generator runs an original function in the test-only interpreter, logs its loads (L list), and the
checker seeds the input RAM from the L list ("first load of a byte wins").  When the first load of a byte happens
AFTER the function itself stored to that byte in the same run (an `lhu` right after its own `sh`), the seeded
"input" is the function's own output; a provider that stores back the value it read (or, where the checker only
compares the final RAM image, one that omits the store) passes every row.

Two modes (read-only on generators and vectors; nothing here reads a provider, src/, asm/, vendor/ ...):

  static  FILE... [--model first|skipcaret|events] [--min-rows N] [--top N] [--entry HEX]
      Line metric from the shipped vector files alone.  For every row it builds the seed image the way the family's
      checker does and replays S in order to get the final stored values.  For every stored (entry, address, size)
      it counts the rows in which EVERY stored byte was seeded from a first load AND the seeded value equals the
      final stored value ("store == read" word).  Order between loads and stores is not in the line, so a flagged
      word can also be an honest input that equals the stored value (clear loops, idempotent stores): the dynamic
      mode classifies those.  Models:
        first      L/S line, first load of a byte wins (cdx, cd_leaf, ds, gpu_leaf, gte_leaf, gte_trig, pad_leaf,
                   state60, spu_caller, cdready, tim checkers)
        skipcaret  like `first`, but an L token ending in '^' (a load made after the original stored to its bytes) is
                   not seeded (gte_cmd, gte60 checkers)
        events     ordered `EV` events (gpu2 / gpu_rows): L seeds a byte only when no earlier S wrote it (gpu2
                   g2_prepare); register window 0x1F801000.. is replayed in order and ignored here

  dyn  MODULE [generator args ...]   (the generator's own command line, output path included)
      Re-runs the generator's main() under a tracker installed on its interpreter hooks (pe_pad_leaf_vectors `pl`
      chain and pe_gpu_rows_vectors `R` chain).  Per case it knows exactly which stores the original code made
      (STORE hook, includes stack stores and stores the generator does not log), which bytes the harness wrote
      (appends to the store log that did not come through the interpreter), and classifies every logged load:
        own      first load of a byte after the original's own store  (the artefact)
        harness  first load after a harness / callee-model write       (separate class)
      It also counts loads the generator HIDES (not appended to the L list) from the stack window
      0x1F0000..0x1FF000 whose bytes the original did not write itself (the cdgen42 line 2982 class), split into
      the o32 incoming-argument words 0x1FF010..0x1FF02F (legitimate harness arguments) and others.
      Result is written as JSON lines to $SELFREAD_OUT (default: stderr summary only).

  dynR MODULE [args ...]   same for the pe_gpu_rows_vectors `R` chain (pe_gpu_rows_vectors, pe_gpu2_vectors, pe_gpu_swap_vectors)
  dynscript SCRIPT.py [args ...]   same for a stand-alone evidence generator script (docs/evidence/*/generate*.py)
  summ JSON...             one line per (run, entry): own-RAM / register / frame / harness readbacks and hidden loads

Usage:  pe_vector_selfread_audit.py static FILE... [options]
        SELFREAD_OUT=out.json pe_vector_selfread_audit.py dyn pe_cdx_vectors OUT.txt cdgen46 -n 200
        pe_vector_selfread_audit.py summ out.json ...
"""
import collections
import json
import os
import re
import struct
import sys

TOK = re.compile(r'^([0-9A-Fa-f]{8}):(\d+)=([0-9A-Fa-f]+)(@\d+)?(\^)?(#\w+)?$')
STACKW = (0x801FE000, 0x801FF000)


def ekey(tok):
    try:
        return int(tok, 16) if re.fullmatch(r'[0-9A-Fa-f]{8}', tok) else tok
    except ValueError:
        return tok


def efmt(e):
    return '%08X' % e if isinstance(e, int) else str(e)


def is_reg(addr):
    return (addr >> 16) == 0x1F80


# ------------------------------------------------------------------------------------------------ static mode
def parse_ls(line):
    """`ENTRY seed pat A args RET r L ... S ... [RETS ...] [CALL ...]` -> (entry, loads, stores) or None."""
    t = line.split()
    if len(t) < 8 or 'L' not in t or 'A' not in t[:6]:
        return None
    li = t.index('L')
    if 'S' not in t[li:]:
        return None
    si = t.index('S', li)
    end = len(t)
    for stop in ('RETS', 'CALL'):
        if stop in t[si:]:
            end = min(end, t.index(stop, si))
    out = []
    for toks in (t[li + 1:si], t[si + 1:end]):
        lst = []
        for k in toks:
            m = TOK.match(k)
            if m:
                lst.append((int(m.group(1), 16), int(m.group(2)), int(m.group(3), 16), bool(m.group(5))))
        out.append(lst)
    return ekey(t[0]), out[0], out[1]


def parse_ev(line):
    """gpu2 / gpu_rows: `ENTRY seed A a0 a1 a2 a3 R ... RET r EV ev...` -> (entry, events)."""
    t = line.split()
    if 'EV' not in t:
        return None
    evs = []
    for k in t[t.index('EV') + 1:]:
        if len(k) > 2 and k[1] == ':' and k[0] in 'LS':
            m = re.match(r'^[LS]:([0-9A-Fa-f]{8}):(\d+)=([0-9A-Fa-f]+)$', k)
            if m:
                evs.append((k[0], int(m.group(1), 16), int(m.group(2)), int(m.group(3), 16)))
    return ekey(t[0]), evs


def static_file(path, model, stats, entry_filter=None):
    rows = 0
    with open(path, errors='replace') as f:
        for line in f:
            if model == 'events':
                p = parse_ev(line)
                if not p:
                    continue
                entry, evs = p
                loads = [(a, n, v, False) for t, a, n, v in evs if t == 'L']
                seeded = {}
                final = {}
                for t, a, n, v in evs:
                    if is_reg(a):
                        continue
                    for i in range(n):
                        b = a + i
                        byte = (v >> (8 * i)) & 0xFF
                        if t == 'L':
                            if b not in seeded and b not in final:
                                seeded[b] = byte
                        else:
                            final[b] = byte
                stores = [(a, n, v, False) for t, a, n, v in evs if t == 'S' and not is_reg(a)]
            else:
                p = parse_ls(line)
                if not p:
                    continue
                entry, loads, stores = p
                seeded = {}
                for a, n, v, caret in loads:
                    if is_reg(a) or (model == 'skipcaret' and caret):
                        continue
                    for i in range(n):
                        seeded.setdefault(a + i, (v >> (8 * i)) & 0xFF)
                final = {}
                for a, n, v, _c in stores:
                    if is_reg(a):
                        continue
                    for i in range(n):
                        final[a + i] = (v >> (8 * i)) & 0xFF
            if entry_filter and entry != entry_filter:
                continue
            rows += 1
            seen = set()
            for a, n, v, _c in stores:
                if is_reg(a) or STACKW[0] <= a < STACKW[1] or (a, n) in seen:
                    continue
                seen.add((a, n))
                key = (entry, a, n)
                st = stats[key]
                st['rows'] += 1
                fin = [final[a + i] for i in range(n)]
                st['vals'].add(int.from_bytes(bytes(fin), 'little'))
                if all((a + i) in seeded for i in range(n)):
                    sv = [seeded[a + i] for i in range(n)]
                    st['seeded'] += 1
                    if sv == fin:
                        st['eq'] += 1
                        st['eqvals'].add(int.from_bytes(bytes(fin), 'little'))
                else:
                    st['unseeded'] += 1
                    if all(x == 0 for x in fin):
                        st['zero_unseeded'] += 1
    return rows


def run_static(argv):
    model, min_rows, top, efilt = 'first', 1, 40, None
    files = []
    i = 0
    while i < len(argv):
        if argv[i] == '--model':
            model = argv[i + 1]
            i += 2
        elif argv[i] == '--min-rows':
            min_rows = int(argv[i + 1])
            i += 2
        elif argv[i] == '--top':
            top = int(argv[i + 1])
            i += 2
        elif argv[i] == '--entry':
            efilt = int(argv[i + 1], 16)
            i += 2
        else:
            files.append(argv[i])
            i += 1
    stats = collections.defaultdict(lambda: dict(rows=0, seeded=0, eq=0, unseeded=0, zero_unseeded=0, vals=set(), eqvals=set()))
    total = 0
    for p in files:
        total += static_file(p, model, stats, efilt)
    print('files=%d rows=%d model=%s' % (len(files), total, model))
    per_entry = collections.defaultdict(lambda: dict(words=0, flagged=0, full=0, rows=0))
    for (entry, a, n), st in stats.items():
        pe = per_entry[entry]
        pe['words'] += 1
        pe['rows'] = max(pe['rows'], st['rows'])
        if st['eq']:
            pe['flagged'] += 1
        if st['rows'] >= min_rows and st['eq'] == st['rows'] and len(st['eqvals']) > 1:
            pe['full'] += 1
    print('entry          rows  store-words  words-with-eq  words-eq-in-ALL-rows(>1 distinct value)')
    for entry in sorted(per_entry, key=efmt):
        pe = per_entry[entry]
        print('%-14s %6d %6d %6d %6d' % (efmt(entry), pe['rows'], pe['words'], pe['flagged'], pe['full']))
    flagged = [(k, s) for k, s in stats.items() if s['eq'] and s['rows'] >= min_rows]
    flagged.sort(key=lambda kv: (-kv[1]['eq'] / kv[1]['rows'], -kv[1]['rows']))
    print('\nflagged store words (entry addr size : rows eq seeded distinct_eq_values distinct_values):')
    for (entry, a, n), s in flagged[:top]:
        print('  %s %08X:%d : rows=%d eq=%d (%.0f%%) seeded=%d eqvals=%d vals=%d' % (
            efmt(entry), a, n, s['rows'], s['eq'], 100.0 * s['eq'] / s['rows'], s['seeded'], len(s['eqvals']), len(s['vals'])))
    zu = [(k, s) for k, s in stats.items() if s['zero_unseeded'] and s['zero_unseeded'] == s['rows']]
    print('\nstore words whose value is 0 in every row and never seeded (cdx: unseeded RAM is 0, an omitted store passes): %d' % len(zu))
    return stats


# ------------------------------------------------------------------------------------------------ dynamic mode
class CaseTracker:
    """Per-case own-write / harness-write / hidden-load accounting for the pl (and R) interpreter chains."""

    def __init__(self, out):
        self.out = out
        self.reset()
        self.entry = 0
        self.cases = collections.Counter()
        self.agg = collections.defaultdict(lambda: collections.Counter())
        self.vals = collections.defaultdict(set)
        self.hsamples = collections.defaultdict(list)
        self.hidden = collections.defaultdict(lambda: collections.Counter())
        self.ncase = collections.Counter()
        self.in_store = False

    def reset(self):
        self.own = {}        # byte -> value (interpreter stores, last value)
        self.harn = {}       # byte -> value (harness writes)
        self.seed = {}       # byte -> (value, cls, load_addr, load_size)   first logged load wins
        self.final = {}      # byte -> last value stored by the original
        self.stk = []        # hidden loads

    def flush(self):
        entry = self.entry
        if entry is None:
            return
        self.ncase[entry] += 1
        words = {}
        for b, (val, cls, la, ln) in self.seed.items():
            if cls == 'ext':
                continue
            w = words.setdefault((la, ln, cls), [True, True])
            # w[0]: all bytes equal final stored value, w[1]: (unused)
            if self.final.get(b) != val:
                w[0] = False
        for (la, ln, cls), (eqfinal, _x) in words.items():
            self.agg[entry][(la, ln, cls, 'cases')] += 1
            if cls != 'ext':
                vs = self.vals[(entry, la, ln, cls)]
                if len(vs) < 1000:
                    vs.add(int.from_bytes(bytes(self.seed[(la & 0x1FFFFF) + i][0] for i in range(ln)), 'little'))
            if eqfinal:
                self.agg[entry][(la, ln, cls, 'eq_final')] += 1
        hid = set()
        for a, n in self.stk:
            hid.add((a, n))
        for a, n in hid:
            kind = ('argwin' if 0x1FF010 <= a < 0x1FF030 else 'below1FE000' if a < 0x1FE000
                    else 'frame_ext_or_mixed')
            self.hidden[entry][kind] += 1
            smp = self.hsamples[(entry, kind)]
            if len(smp) < 6:
                smp.append('%08X:%d' % (0x80000000 + a, n))
        if hid:
            self.hidden[entry]['cases_with_hidden'] += 1
        self.reset()


TRACKER = None


def install(out):
    """Wrap the pl and R interpreter chains (after the generator module has installed its own wrappers)."""
    global TRACKER
    import pe_pad_leaf_vectors as pl
    tr = TRACKER = CaseTracker(out)

    class LoadList(list):
        def clear(self):
            tr.flush()
            super().clear()

        def append(self, item):
            a, n, val = item
            off = a & 0x1FFFFF
            own = [(off + i) in tr.own for i in range(n)]
            har = [(off + i) in tr.harn for i in range(n)]
            self._last = []
            for i in range(n):
                b = off + i
                if b in tr.seed:
                    continue
                self._last.append(b)
                if own[i]:
                    tr.seed[b] = ((val >> (8 * i)) & 0xFF, 'own', a, n)
                elif har[i]:
                    tr.seed[b] = ((val >> (8 * i)) & 0xFF, 'harness', a, n)
                else:
                    tr.seed[b] = ((val >> (8 * i)) & 0xFF, 'ext', a, n)
            super().append(item)

    def _pop(self, *a):
        item = list.pop(self, *a)
        for b in getattr(self, '_last', ()):          # scratch patch (cdgen47): the generator's own-written filter removes this load again
            tr.seed.pop(b, None)
        self._last = []
        return item
    LoadList.pop = _pop

    class StoreList(list):
        def clear(self):
            super().clear()

        def append(self, item):
            a, n, val = item
            if not tr.in_store:             # not through the interpreter hook: a harness / callee-model write
                off = a & 0x1FFFFF
                for i in range(n):
                    tr.harn[off + i] = (val >> (8 * i)) & 0xFF
            super().append(item)

    pl.LOADS = LoadList()
    pl.STORES = StoreList()
    chain_load, chain_store = pl._ns['LOAD'], pl._ns['STORE']

    def store(fmt, buf, a, val):
        n = struct.calcsize(fmt)
        tr.in_store = True
        try:
            r = chain_store(fmt, buf, a, val)
        finally:
            tr.in_store = False
        for i in range(n):
            tr.own[a + i] = (val >> (8 * i)) & 0xFF
            tr.final[a + i] = (val >> (8 * i)) & 0xFF
            tr.harn.pop(a + i, None)
        return r

    def load(fmt, buf, a):
        n0 = len(pl.LOADS)
        val = chain_load(fmt, buf, a)
        n = struct.calcsize(fmt)
        if len(pl.LOADS) == n0 and 0x1F0000 <= a < 0x1FF000 and not getattr(pl.v, '_dummy', False):
            if not all((a + i) in tr.own for i in range(n)):
                tr.stk.append((a, n))
        if len(pl.LOADS) == n0 and 0x1F0000 <= a < 0x1FF000:
            pass
        return val

    pl._ns['STORE'] = store
    pl._ns['LOAD'] = load
    # entry identity: the generators set pl.CUR[0] after clearing
    class Cur(list):
        def __setitem__(self, i, v):
            tr.entry = v
            super().__setitem__(i, v)
    cur = Cur(pl.CUR)
    pl.CUR = cur
    tr.entry = pl.CUR[0]
    return tr


def install_R(out):
    """Tracker for the R chain (pe_gpu_rows_vectors / pe_gpu2_vectors / pe_gpu_swap_vectors): one ordered event stream per case.
    Counts the loads the engine hides (R.logged() false: the stack window 0x1F0000..0x1FF000) whose bytes the original
    did not store itself.  A case starts at the first R.execute() call with $ra == RET (resumes after a callee stop
    carry another $ra)."""
    global TRACKER
    import pe_gpu_rows_vectors as R
    tr = TRACKER = CaseTracker(out)
    chain_load, chain_store, chain_exec = R._ns['LOAD'], R._ns['STORE'], R.execute

    def store(fmt, buf, a, val):
        r = chain_store(fmt, buf, a, val)
        for i in range(struct.calcsize(fmt)):
            tr.own[a + i] = 1
        return r

    def load(fmt, buf, a):
        n = struct.calcsize(fmt)
        val = chain_load(fmt, buf, a)
        if 0x1F0000 <= a < 0x1FF000 and not R.logged(a) and not R.v.JMPBUF[0] <= a < R.v.JMPBUF[1]:
            if not all((a + i) in tr.own for i in range(n)):
                tr.stk.append((a, n))
        return val

    def execute(ram, pc, args=(), **kw):
        ir = kw.get('initial_regs') or {}
        if ir.get(31) == R.RET:
            tr.flush()
            tr.entry = pc
        return chain_exec(ram, pc, args, **kw)

    R._ns['STORE'], R._ns['LOAD'] = store, load
    R.execute = execute
    return tr


def run_dyn_R(argv):
    mod = argv[0]
    here = os.path.dirname(os.path.abspath(__file__))
    sys.path.insert(0, here)
    out = os.environ.get('SELFREAD_OUT')
    m = __import__(mod)
    sys.argv = [m.__file__] + argv[1:]
    tr = install_R(out)
    try:
        m.main()
    finally:
        tr.flush()
        report(tr, out)


def run_summ(argv):
    """summ JSON... : one line per (file, entry) with own-write / harness-write readbacks and hidden loads."""
    maxw = 6
    for path in argv:
        try:
            data = json.load(open(path))
        except (OSError, ValueError) as e:
            print('%s: unreadable (%s)' % (os.path.basename(path), e))
            continue
        name = os.path.basename(path)[:-5]
        for entry, r in sorted(data.items()):
            if entry == '00000000':
                continue
            allown = {w: d for w, d in r['words'].items() if w.endswith(':own')}

            def kind(w):
                a = int(w.split(':')[0], 16)
                return 'reg' if is_reg(a) else 'frame' if 0x801F0000 <= a < 0x80200000 else 'ram'
            own = {w: d for w, d in allown.items() if kind(w) == 'ram'}
            ownreg = {w: d for w, d in allown.items() if kind(w) == 'reg'}
            ownfr = {w: d for w, d in allown.items() if kind(w) == 'frame'}
            har = {w: d for w, d in r['words'].items() if w.endswith(':harness')}
            hid = {k: v for k, v in r['hidden'].items() if k != 'cases_with_hidden'}
            tag = []
            if ownreg:
                tag.append('reg-readback words=%d (replayed in order, not a RAM seed)' % len(ownreg))
            if ownfr:
                tag.append('frame-spill words=%d (stack, not compared)' % len(ownfr))
            if own:
                top = sorted(own.items(), key=lambda kv: -kv[1].get('cases', 0))[:maxw]
                tag.append('OWN-RAM words=%d eq_final=%d top=[%s]' % (
                    len(own), sum(1 for d in own.values() if d.get('eq_final')),
                    ', '.join('%s c=%d eq=%d v=%d' % (w[:-4], d.get('cases', 0), d.get('eq_final', 0), d.get('distinct', 0)) for w, d in top)))
            if har:
                tag.append('HARNESS words=%d' % len(har))
            if hid:
                tag.append('HIDDEN %s (cases %d) e.g. %s' % (hid, r['hidden'].get('cases_with_hidden', 0), r.get('hidden_samples', {})))
            print('%-16s %s cases=%d %s' % (name, entry, r['cases'], '; '.join(tag) or 'clean'))


def run_dynscript(argv):
    """dynscript SCRIPT.py args... : run a stand-alone evidence generator script (docs/evidence/*/generate*.py, which
    import pe_cd_leaf_vectors and call cdgen_cases / run_with_callees) under the tracker."""
    import runpy
    here = os.path.dirname(os.path.abspath(__file__))
    sys.path.insert(0, here)
    out = os.environ.get('SELFREAD_OUT')
    __import__('pe_cd_leaf_vectors')          # install the generator's own chain first, the tracker goes on top
    tr = install(out)
    script = argv[0]
    sys.argv = [script] + argv[1:]
    try:
        runpy.run_path(script, run_name='__main__')
    finally:
        tr.flush()
        report(tr, out)


def run_dyn(argv):
    mod = argv[0]
    here = os.path.dirname(os.path.abspath(__file__))
    sys.path.insert(0, here)
    out = os.environ.get('SELFREAD_OUT')
    m = __import__(mod)
    sys.argv = [m.__file__] + argv[1:]
    tr = install(out)
    try:
        m.main()
    finally:
        tr.flush()
        report(tr, out)


def report(tr, out):
    res = {}
    for entry in sorted(set(tr.agg) | set(tr.hidden) | set(tr.ncase)):
        words = collections.defaultdict(dict)
        for (la, ln, cls, kind), c in tr.agg[entry].items():
            words['%08X:%d:%s' % (la, ln, cls)][kind] = c
            words['%08X:%d:%s' % (la, ln, cls)]['distinct'] = len(tr.vals.get((entry, la, ln, cls), ()))
        res['%08X' % entry] = dict(cases=tr.ncase[entry], words=dict(words), hidden=dict(tr.hidden[entry]),
                                   hidden_samples={k[1]: v for k, v in tr.hsamples.items() if k[0] == entry})
    if out:
        json.dump(res, open(out, 'w'), indent=1, sort_keys=True)
    for entry, r in res.items():
        own = {w: d for w, d in r['words'].items() if w.endswith(':own')}
        har = {w: d for w, d in r['words'].items() if w.endswith(':harness')}
        print('DYN %s cases=%d own_words=%d own_eq_final_words=%d harness_words=%d hidden=%s' % (
            entry, r['cases'], len(own), sum(1 for d in own.values() if d.get('eq_final')), len(har), r['hidden']), file=sys.stderr)


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(2)
    if sys.argv[1] == 'static':
        run_static(sys.argv[2:])
    elif sys.argv[1] == 'dyn':
        run_dyn(sys.argv[2:])
    elif sys.argv[1] == 'dynscript':
        run_dynscript(sys.argv[2:])
    elif sys.argv[1] == 'summ':
        run_summ(sys.argv[2:])
    elif sys.argv[1] == 'dynR':
        run_dyn_R(sys.argv[2:])
    else:
        print(__doc__)
        sys.exit(2)
