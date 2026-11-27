#!/usr/bin/env python3
"""Run a cdgen47 checker once per vector line (fresh process, so provider-internal static state, e.g. the stream handler's
`stream_ctx`, cannot carry from one case to the next), and optionally also in reversed order inside one process.
Usage: pe_cdx_cdgen47_percase.py CHECKER VECTORS [FAILS_OUT]
Prints: per-case results (fresh process) and the shared-process forward / reversed-order results (differing line sets are listed)."""
import os
import subprocess
import sys
import tempfile

checker, vec = sys.argv[1], sys.argv[2]
lines = open(vec).read().splitlines()
tmp = tempfile.mkdtemp(prefix='cdx47pc')
fresh_bad = []
for i, l in enumerate(lines, 1):
    p = os.path.join(tmp, 'one.txt')
    open(p, 'w').write(l + '\n')
    r = subprocess.run([checker, p], capture_output=True, text=True)
    if r.returncode != 0:
        fresh_bad.append(i)


def shared(order):
    p = os.path.join(tmp, 'all.txt')
    open(p, 'w').write('\n'.join(lines[j] for j in order) + '\n')
    r = subprocess.run([checker, p], capture_output=True, text=True, env=dict(os.environ, CDX_LINES='1'))
    pos = {int(t.split()[1]) for t in r.stderr.splitlines() if t.startswith('FAILLINE')}
    return {order[k - 1] + 1 for k in pos}


fwd = shared(list(range(len(lines))))
rev = shared(list(range(len(lines) - 1, -1, -1)))
print('cases %d: fresh-process mismatches %d, shared-process (file order) %d, shared-process (reversed order) %d' % (len(lines), len(fresh_bad), len(fwd), len(rev)))
print('order-dependent cases (fresh vs shared forward): %d; forward vs reversed: %d' % (len(set(fresh_bad) ^ fwd), len(fwd ^ rev)))
if len(sys.argv) > 3:
    open(sys.argv[3], 'w').write(''.join('FAILLINE %d\n' % n for n in fresh_bad))
