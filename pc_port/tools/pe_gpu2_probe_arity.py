#!/usr/bin/env python3
"""Black-box arity probe for guest callees that have no public prototype: run the callee itself with garbage in the
registers beyond `nargs`; the smallest nargs for which no case is dropped (events and RAM identical for both register
fills) is the number of argument registers the callee actually consumes.  Usage: pe_gpu2_probe_arity.py func_X ..."""
import sys

import pe_gpu2_vectors as V
import pe_gpu2_common as C

base = C.fixture()
for name in sys.argv[1:]:
    off, size, entry = C.func_range(name)
    res = []
    for nargs in range(5):
        ok = 0
        total = 40
        for k in range(total):
            rng = C.Rng(entry + k)
            args = [V.word(rng) if rng.next() & 1 else V.zptr(rng, 4, 0x400) for _ in range(4)]

            def setup(ram, rng=rng):
                V.mk_env(rng, ram)
            r = C.case_run(base, setup, entry, size, args, nargs, V.rets_all(rng), None, budget=60000, gseed=k + 1)
            ok += r is not None
        res.append(ok)
    first = next((n for n, ok in enumerate(res) if ok == max(res)), 4)
    print('%s: consistent cases of 40 for nargs 0..4 = %s -> arity %d' % (name, res, first))
