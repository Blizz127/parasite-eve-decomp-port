#!/usr/bin/env python3
"""Join a checker run's failing line numbers (CDX_LINES=1 stderr) with a vector set's .classes sidecar:
per class (verbatim / perturbed / discovery, with -explore / -wait suffixes) cases and mismatches.
Usage: pe_cdx_cdgen47_classes.py VECTORS.txt FAILLINES.txt   (FAILLINES: stderr of `CDX_LINES=1 ./check47_x VECTORS.txt`)"""
import collections
import sys

cls = {}
for line in open(sys.argv[1] + '.classes'):
    n, k, e = line.split()
    cls[int(n)] = k
fails = {int(l.split()[1]) for l in open(sys.argv[2]) if l.startswith('FAILLINE')}
tot, bad = collections.Counter(), collections.Counter()
for n, k in cls.items():
    base = k.split('-')[0]
    for key in {base, k, base + ('-wait' if k.endswith('-wait') else '-nowait')}:
        tot[key] += 1
        bad[key] += n in fails
for key in sorted(tot):
    print('  %-24s cases=%-4d mismatches=%d' % (key, tot[key], bad[key]))
