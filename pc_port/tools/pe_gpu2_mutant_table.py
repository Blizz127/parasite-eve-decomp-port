#!/usr/bin/env python3
"""Per-entry control table: of the cases the CURRENT binding passes (mutant 0), how many does each mutant fail?
Usage: pe_gpu2_mutant_table.py VECTORS OUT_PREFIX   (reads OUT_PREFIX_m{0,1,2,3}.out/.err written by pe_gpu2_run_checks.sh)"""
import re
import sys
from collections import defaultdict

vec, pre = sys.argv[1], sys.argv[2]
entries = [ln.split()[0] for ln in open(vec)]


def fails(m):
    out = set()
    for ln in open('%s_m%d.err' % (pre, m)):
        r = re.match(r'MISMATCH line (\d+):', ln)
        if r:
            out.add(int(r.group(1)))
    return out


sel = set()
for ln in open('%s_m0.out' % pre):
    r = re.match(r'\s+([0-9A-F]{8}): (\d+) cases', ln)
    if r:
        sel.add(r.group(1))
f = [fails(m) for m in range(4)]
tot = defaultdict(int); ok0 = defaultdict(int); k = [defaultdict(int) for _ in range(4)]
for n, e in enumerate(entries, 1):
    if e not in sel:
        continue
    tot[e] += 1
    if n not in f[0]:
        ok0[e] += 1
        for m in (1, 2, 3):
            if n in f[m]:
                k[m][e] += 1
print('| entry | cases | pass (current binding) | of those failed by no-op stub | by return flip | by store flip |')
print('|---|---|---|---|---|---|')
for e in sorted(tot):
    print('| %s | %d | %d | %d | %d | %d |' % (e, tot[e], ok0[e], k[1][e], k[2][e], k[3][e]))
