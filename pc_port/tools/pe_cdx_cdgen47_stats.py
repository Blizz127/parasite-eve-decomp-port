#!/usr/bin/env python3
"""Coverage statistics of cdgen47 vector sets: cases, distinct access paths (ordered loads' addresses + callee names), callee-token counts,
distinct values per non-CD-data register read (a register read that is single-valued across the set is not exercised).
Usage: pe_cdx_cdgen47_stats.py VECTORS.txt.."""
import collections
import re
import sys

NAMES = {'8007B9EC': 'flush', '80071A74': 'printf', '80073C5C': 'puts', '80073A44': 'VSync', '8007AAB4': 'ack', '8007B010': 'poll', '8007A488': 'cd_ready',
         '8007C214': 'data_ready', '8007CEAC': 'dma_issue', '80073DE8': 'ctl_state', '80131140': 'cbA', '80131180': 'cbB', '801311C0': 'cbStream'}
for path in sys.argv[1:]:
    paths, callee, regs = set(), collections.Counter(), collections.defaultdict(set)
    n = 0
    for line in open(path):
        if ' L ' not in line:
            continue
        n += 1
        L = line.split(' L ')[1].split(' S ')[0].split()
        call = line.split(' CALL ')[1].split()
        names = tuple(t.split(',')[0].split('@')[0] for t in call if t != '-')
        paths.add((tuple(t.split('=')[0] for t in L), names))
        for nm in set(names):
            callee[NAMES.get(nm, nm)] += 1
        for t in L:
            m = re.match(r'(1F80[0-9A-F]{4}):(\d)=([0-9A-F]+)$', t)
            if m:
                regs[m.group(1) + '/' + m.group(2)].add(m.group(3))
    print('%s: cases=%d distinct_access_paths=%d' % (path.split('/')[-1], n, len(paths)))
    print('   callee cases: ' + ', '.join('%s=%d' % kv for kv in sorted(callee.items())))
    print('   distinct values per register read: ' + ', '.join('%s=%d' % (k, len(v)) for k, v in sorted(regs.items())))
