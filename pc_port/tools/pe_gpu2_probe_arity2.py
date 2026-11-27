#!/usr/bin/env python3
"""Controlled arity probes for callees whose random-argument probe (pe_gpu2_probe_arity.py) was inconclusive because
random arguments spin or jump: arguments are drawn from each callee's plausible domain (see the lambda)."""
import pe_gpu2_vectors as V
import pe_gpu2_common as C

base = C.fixture()
PROBES = {
    'func_80071A34': lambda r: [V.ZONE + 4 * (r.next() % 0x100), V.ZONE + 0x1000 + 4 * (r.next() % 0x100), 4 * (r.next() % 0x30), 0],
    'func_80073CF4': lambda r: [r.next() % 7, r.pick((0, 0x80130000, 0x80130040)), 0, 0],
    'func_80073A44': lambda r: [r.pick((0xFFFFFFFF, 0, 1, 2, 3, 5)), 0, 0, 0],
    'func_80077294': lambda r: [r.pick((0, 1, 2, 0xFFFFFFFF)), 0, 0, 0],
}
for name, mk in PROBES.items():
    off, size, entry = C.func_range(name)
    res = []
    for nargs in range(5):
        ok = 0
        for k in range(60):
            rng = C.Rng(entry + k)
            args = mk(rng)

            def setup(ram, rng=rng):
                V.mk_env(rng, ram)
                C.put32(ram, 0x80095754, 0); C.put32(ram, 0x80095758, 0)
            r = C.case_run(base, setup, entry, size, args, nargs, V.rets_all(rng), {'stat': [V.READY_POOL[0]], 'chcr': [0]},
                           budget=60000, gseed=k + 1)
            ok += r is not None
        res.append(ok)
    first = next((n for n, ok in enumerate(res) if ok == max(res)), 4)
    print('%s: consistent cases of 60 for nargs 0..4 = %s -> arity %d' % (name, res, first))
