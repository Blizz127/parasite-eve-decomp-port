#!/usr/bin/env python3
"""Assemble the per-function results table of REPORT.md from a pe_gpu2_run_checks.sh output directory.
Usage: pe_gpu2_report_tables.py RUNDIR VECTOR_DIR EXPAND_VECTORS"""
import re
import sys
from collections import Counter

import pe_gpu2_vectors as V
import pe_gpu2_common as C
import importlib.util

run, vec, expv = sys.argv[1:4]
spec = importlib.util.spec_from_file_location('rd', 'pe_gpu2_report_data.py')

PROVIDER = {}
src = open('pe_gpu2_report_data.py').read()
ns = {}
exec(src[src.index('PROVIDER = {'):src.index('BATCH = ')], ns)
PROVIDER = ns['PROVIDER']
BATCH = {f: b for b, fs in V.BATCHES.items() for f in fs}
BATCH['func_8010BD4C'] = 'gpu2d'

cnt = Counter()
for b in V.BATCHES:
    for ln in open('%s/%s/%s.txt' % (vec, b, b)):
        cnt['func_' + ln.split()[0]] += 1
for ln in open(expv):
    if not ln.startswith('#'):
        cnt['func_' + ln.split()[0]] += 1

res = {}
for b, lines in (('gpu2a', ['l1']), ('gpu2b', ['l1']), ('gpu2c', ['l1']), ('gpu2d', ['l1'])):
    for l in lines:
        for ln in open('%s/mutant_table_%s_%s.md' % (run, b, l)):
            r = re.match(r'\| ([0-9A-F]{8}) \| (\d+) \| (\d+) \| (\d+) \| (\d+) \| (\d+) \|', ln)
            if r:
                res['func_' + r.group(1)] = [int(x) for x in r.groups()[1:]]
# expander (separate vector file)
exp = []
for m in range(4):
    for ln in open('%s/run_gpu2d_expand_m%d.out' % (run, m)):
        r = re.match(r'\s+8010BD4C: (\d+) cases, (\d+) mismatches', ln)
        if r:
            exp.append((int(r.group(1)), int(r.group(2))))
print('| function | yaml offset/size | provider file(s) | batch | cases | smoke: current binding passes | no-op stub fails (of those) | return flip fails | store flip fails |')
print('|---|---|---|---|---|---|---|---|---|')
for f in sorted(PROVIDER, key=lambda x: (BATCH[x], x)):
    if f in V.OVL:
        off, size, ent = C.func_range(f, V.OVL_YAML, V.OVL_VRAM)
        loc = 'ovl_039F %#x/%#x' % (off, size)
    else:
        off, size, ent = C.func_range(f)
        loc = 'disc1 %#x/%#x' % (off, size)
    if f == 'func_8010BD4C':
        c, bad0 = exp[0]
        ok0 = c - bad0
        row = '%d | %d | all %d | (see note) | (see note)' % (ok0, exp[1][1], exp[1][1]) if False else \
            '%d of %d | %d of %d | %d of %d (return unchanged: 1) | %d of %d' % (ok0, c, exp[1][1], c, exp[2][1], c, exp[3][1], c)
        print('| %s | %s | %s | %s | %d | %s |' % (f, loc, PROVIDER[f], BATCH[f], cnt[f], row))
        continue
    r = res.get(f)
    if r is None:
        print('| %s | %s | %s | %s | %d | not smoked (no linked binding in this snapshot) | - | - | - |' % (f, loc, PROVIDER[f], BATCH[f], cnt[f]))
    else:
        print('| %s | %s | %s | %s | %d | %d of %d | %d | %d | %d |' % (f, loc, PROVIDER[f], BATCH[f], cnt[f], r[1], r[0], r[2], r[3], r[4]))
