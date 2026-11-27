#!/usr/bin/env python3
"""Group the checker's MISMATCH lines (run with G2_VERBOSE=1) by entry and first-failure kind.
Usage: pe_gpu2_categorize.py STDERR_FILE"""
import re
import sys
from collections import Counter, defaultdict

cat = defaultdict(Counter)
for ln in open(sys.argv[1]):
    if not ln.startswith('MISMATCH'):
        continue
    m = re.search(r'MISMATCH line \d+: (.*?) ret [0-9A-F]+/[0-9A-F]+ consumed (\d+)/(\d+) written (\d+)/(\d+): ([0-9A-F]{8}) ', ln)
    if not m:
        continue
    msg, ent = m.group(1), m.group(6)
    if 'RAM load outside' in msg:
        k = 'reads RAM the original did not read'
    elif 'hw load' in msg:
        k = 'different hardware load'
    elif 'hw store' in msg:
        k = 'different hardware store'
    elif 'HANG' in msg:
        k = 'provider hangs (60 ms)'
    elif 'stop' in msg:
        k = 'provider requests a stop'
    elif re.search(r'unexpected func_[0-9A-F]{8}', msg):
        k = 'different guest call / argument (' + re.search(r'unexpected (func_[0-9A-F]{8})', msg).group(1) + ')'
    elif msg.strip() == '':
        k = 'final state differs (events left / written set / return value)'
    else:
        k = msg.split(' unexpected ')[-1][:40]
    cat[ent][k] += 1
for ent in sorted(cat):
    print(ent)
    for k, n in cat[ent].most_common():
        print('   %5d  %s' % (n, k))
