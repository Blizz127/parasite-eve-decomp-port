#!/usr/bin/env python3
"""Own-written-load filter for the pe_pad_leaf_vectors (`pl`) interpreter chain.

The artefact it removes (docs/evidence/public-openness-20261126/VECTOR-GENERATOR-SELFREAD-AUDIT.md): a generator logs every
non-stack load of the original in the L list, and the checkers seed "the first load of a byte wins".  When the first load
of a byte comes AFTER the original itself stored to that byte in the same run (an `lhu` right after its own `sh`), the
seeded "input" is the function's own output, so "store == value read" holds trivially and a provider that stores back
what it read passes.

Rule (same as pe_cdx_vectors.py's cdgen46b fix, sw-cdx-spu2):
  * a load whose bytes were ALL stored earlier, in the same case, by the original itself is never logged (it is not an
    input); it is counted (`own_loads`) and the case is counted (`own_cases`);
  * a load that MIXES own-written and external bytes drops the whole case (cd.UNFAITHFUL, the generator's existing
    drop path); counted (`mixed_loads`).  (sw-cdx-spu2's pe_cdx_vectors.py instead keeps the case and logs the external
    bytes of a mixed load; the choice is the lead's, see REVECTOR-REPORT.md.)
  * harness / callee-model writes (they reach pl.STORES without passing the interpreter's store hook) are NOT own: they
    clear the own mark of their bytes, so a later load of such a byte is an external (model) value and stays logged;
  * hardware-register offsets (< 0x2000: register page, replayed in order by the checkers), the stack/frame window
    (>= 0x1F0000, never logged) and caller-exempted words (plain-memory wait models: MEMSEQ) are left alone.

Install:  OWN = pe_ownwrite.install(pl, unfaithful_list, exempt=lambda a: ...)   (innermost, at import time;
          inactive until enable())
Enable:   OWN.enable()  from the generator's main() unless the environment variable CD_LEGACY_OWN_LOADS is set
          (legacy: nothing is filtered, output byte-identical to the pre-fix generator).
Case start: wrap the generator's per-case runners with OWN.case_runner(fn) (clears the own set).
Nothing here depends on pl.LOADS / pl.CUR / the audit tool's replacement objects.
"""
import collections
import os
import struct
import sys

LO, HI = 0x2000, 0x1F0000


class OwnWrite:
    def __init__(self, pl, unfaithful, exempt):
        self.pl = pl
        self.unfaithful = unfaithful
        self.exempt = exempt or (lambda a: False)
        self.on = False
        self.own = set()
        self.over = set()      # bytes the original stored and a harness / callee-model write then overwrote
        self.buf = None
        self.in_store = False
        self.hit = False
        self.counts = collections.defaultdict(collections.Counter)

    # ---- per case ----
    def reset(self):
        if self.hit:
            self.counts[self.pl.CUR[0]]['own_cases'] += 1
        self.hit = False
        self.own.clear()
        self.over.clear()

    def case_runner(self, fn):
        def run(*a, **k):
            self.reset()
            return fn(*a, **k)
        run.__name__ = getattr(fn, '__name__', 'run')
        run.__wrapped__ = fn
        return run

    # ---- hooks ----
    def hook_store(self, inner):
        def store(fmt, buf, a, val):
            if LO <= a < HI:
                r = range(a, a + struct.calcsize(fmt))
                self.own.update(r)
                self.over.difference_update(r)
            self.in_store = True
            try:
                return inner(fmt, buf, a, val)
            finally:
                self.in_store = False
        return store

    def hook_load(self, inner):
        def load(fmt, buf, a):
            if self.on and LO <= a < HI and self.over and not self.exempt(a):
                n = struct.calcsize(fmt)
                if all((a + i) in self.over for i in range(n)):     # logged: its value is the harness model's, not the original's
                    self.counts[self.pl.CUR[0]]['harness_over_loads'] += 1
            if self.on and LO <= a < HI and self.own and not self.exempt(a):
                n = struct.calcsize(fmt)
                k = sum((a + i) in self.own for i in range(n))
                if k == n:
                    c = self.counts[self.pl.CUR[0]]
                    c['own_loads'] += 1
                    c['own_load_a%X_n%d' % (0x80000000 + a, n)] += 1
                    self.hit = True
                    return struct.unpack_from(fmt, buf, a)      # the value is read, nothing is logged
                if k:
                    self.counts[self.pl.CUR[0]]['mixed_loads'] += 1
                    self.unfaithful[0] = True
            return inner(fmt, buf, a)
        return load

    def enable(self):
        """Activate the filter and make harness writes clear own marks.  Called from main()."""
        if os.environ.get('CD_LEGACY_OWN_LOADS'):
            return False
        self.on = True
        base = type(self.pl.STORES)
        me = self

        class Stores(base):
            def append(self, item):
                if not me.in_store:
                    a, n, _v = item
                    off = a & 0x1FFFFF
                    for i in range(n):
                        if (off + i) in me.own:
                            me.over.add(off + i)
                        me.own.discard(off + i)
                base.append(self, item)

        self.pl.STORES = Stores(self.pl.STORES)
        return True

    def report(self, out=sys.stderr):
        self.reset()
        if not self.on:
            print('ownwrite: LEGACY (filter off)', file=out)
            return
        for e in sorted(self.counts, key=lambda x: str(x)):
            c = self.counts[e]
            words = sorted(k for k in c if k.startswith('own_load_a'))
            print('ownwrite: entry %s own_loads=%d own_cases=%d mixed_loads=%d harness_over_loads=%d' % (
                '%08X' % e if isinstance(e, int) else e, c['own_loads'], c['own_cases'], c['mixed_loads'],
                c['harness_over_loads']), file=out)
            for w in words:
                print('ownwrite:   %s x%d' % (w[9:], c[w]), file=out)


def install(pl, unfaithful, exempt=None):
    o = OwnWrite(pl, unfaithful, exempt)
    pl._ns['STORE'] = o.hook_store(pl._ns['STORE'])
    pl._ns['LOAD'] = o.hook_load(pl._ns['LOAD'])
    return o
