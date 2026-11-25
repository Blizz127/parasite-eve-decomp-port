#!/usr/bin/env python3
"""List PE_* host helper definitions whose names point at SDK routines.

The Psy-Q guard counts func_<addr> symbols only; host code can also carry
SDK behaviour under PE_* names (gen_decomp_ports host replacements,
host-array variants). This lists every PE_* function definition in
pc_port/platform, pc_port/game/boot and pc_port/game/decomp_hand whose name
contains an address that configs/USA/psyq_classification.tsv classifies as
SDK, or an SDK function name of 6+ characters. Heuristic: see
docs/evidence/pe-helper-inventory-20261031/README.md. Reads names only.

Usage: pe_helper_sdk_inventory.py > inventory.tsv
"""
import glob
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DEF_RE = re.compile(r'^[A-Za-z_][\w *]*?\b(PE_\w+)\s*\([^;]*$')


def main():
    by_addr, by_name = {}, {}
    for line in open(ROOT / 'configs/USA/psyq_classification.tsv'):
        f = line.rstrip('\n').split('\t')
        if not f[0].startswith('0x') or f[2] == 'game':
            continue
        row = (f[0], f[2], f[-1])
        by_addr[f[0][2:].upper()[-5:]] = row
        if len(f[-1]) >= 6 and not f[-1].startswith('text_'):
            by_name[f[-1].lower()] = row
    print('pe_name\tfile\tsdk_addr\tlib\tsdk_name')
    seen = set()
    for d in ('pc_port/platform', 'pc_port/game/boot', 'pc_port/game/decomp_hand'):
        for p in sorted(glob.glob(str(ROOT / d / '*.c'))):
            rel = str(Path(p).relative_to(ROOT))
            for line in open(p, errors='replace'):
                m = DEF_RE.match(line)
                if not m or (rel, m.group(1)) in seen:
                    continue
                n = m.group(1)
                seen.add((rel, n))
                hit = next((by_addr[h.upper()[-5:]] for h in re.findall(r'[0-9A-Fa-f]{5,8}', n)
                            if h.upper()[-5:] in by_addr), None)
                if hit is None:
                    hit = next((v for k, v in by_name.items() if k in n.lower()), None)
                if hit:
                    print('\t'.join((n, rel) + hit))


if __name__ == '__main__':
    main()
