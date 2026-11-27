#!/usr/bin/env python3
"""Assemble the per-function table of docs/evidence/native-state60-20261126/REPORT.md from the vector files,
stats.json and the pe_state60_run.py JSON.  Summaries are computed from the L/S/RET/CALL logs only.

usage: pe_state60_report.py VECTOR_ROOT RUN.json > table.md"""
import collections
import json
import os
import re
import sys

root, runp = sys.argv[1], sys.argv[2]
run = json.load(open(runp))['functions']
rows = []
for batch in sorted(d for d in os.listdir(root) if d.startswith('state60')):
    st = json.load(open(os.path.join(root, batch, 'stats.json')))
    for fn in sorted(st):
        s = st[fn]
        lines = [l.split() for l in open(os.path.join(root, batch, fn + '.txt')) if l.strip()]
        rets, stores_ram, stores_reg, loads_reg, loads_ram, calls = set(), collections.Counter(), collections.Counter(), collections.Counter(), collections.Counter(), collections.Counter()
        for t in lines:
            ri = t.index('RET'); li = t.index('L'); si = t.index('S'); ci = t.index('CALL'); r2 = t.index('RETS')
            rets.add(t[ri + 1])
            for x in t[li + 1:si]:
                if x != '-':
                    a = int(x.split(':')[0], 16)
                    (loads_reg if a >> 16 == 0x1F80 else loads_ram)[a if a >> 16 == 0x1F80 or not 0x80100000 <= a < 0x80102000 else 'zone'] += 1
            for x in t[si + 1:r2]:
                if x != '-':
                    a = int(x.split(':')[0], 16)
                    sz = x.split(':')[1].split('=')[0]
                    (stores_reg if a >> 16 == 0x1F80 else stores_ram)[('%08X' % a if a >> 16 == 0x1F80 or not 0x80100000 <= a < 0x80102000 else 'zone') + '/' + sz] += 1
            for x in t[ci + 1:]:
                if x != '-':
                    calls[x.split(',')[0]] += 1
        def lst(c, n=6):
            ks = sorted(c, key=lambda k: -c[k])
            return ', '.join(str(k) if isinstance(k, str) else '%08X' % k for k in ks[:n]) + (' +%d more' % (len(ks) - n) if len(ks) > n else '') if c else '-'
        parts = ['RET: %s' % ('none' if s['kinds'] is None else '%d distinct' % len(rets) if len(rets) > 1 else rets.copy().pop())]
        parts.append('RAM stores: ' + lst(stores_ram))
        parts.append('reg stores: ' + lst(stores_reg))
        parts.append('RAM loads: ' + lst(loads_ram))
        parts.append('reg loads: ' + lst(loads_reg))
        parts.append('callees: ' + lst(calls))
        r = run.get(fn, {})
        def res(k):
            x = r.get(k)
            return '?' if not x else '%s %s/%s' % (x['status'], x['passed'], x['checked'])
        def mf(k):
            x = r.get(k)
            return '?' if not x else str(x['failed'])
        rows.append('| %s | %s/%s | %s | %s | %d | %s | %s | %s/%s/%s | %s |' % (
            fn, s['yaml_offset'], s['yaml_size'], r.get('family', ''), s['batch'], s['cases'],
            '; '.join(parts), res('base'), mf('m1'), mf('m2'), mf('m3'),
            'controlled' if r.get('controlled') else 'NOT controlled'))
print('| function | yaml offset/size | checker | batch | cases | observable outputs (from logs) | checker smoke vs current binding | mutant fails m1/m2/m3 | control |')
print('|---|---|---|---|---|---|---|---|---|')
print('\n'.join(rows))
