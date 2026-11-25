#!/usr/bin/env python3
"""Canonical guest-name prototypes for pe_plat bindings.

Every `#define func_X pe_plat_Y` in pc_port/include/pe_plat/*.h needs a
`func_X(...)` prototype in the same header, copied from pe_plat_Y's own
prototype.  gen_decomp_ports.py and gen_overlay_ports.py read callee
signatures from these.  Without them, a fresh generation:
- cannot lower callers (func_8008D7D0, func_800D27FC);
- writes wrong-arity externs;
- drops overlay dispatch entries.

The `pe_plat_guest_addr` typedef is written as uint32_t, because the overlay
thunk builder knows only plain integer type names.

Usage:
  pe_plat_canonical_protos.py --check   exit 1 if any binding lacks one
  pe_plat_canonical_protos.py --write   append the missing prototypes
"""
import argparse
import glob
import re
import sys

DEF = re.compile(r'^#define\s+(func_[0-9A-Fa-f]{8})\s+(pe_plat_\w+)\s*$', re.M)


def scan(path):
    s = open(path).read()
    flat = re.sub(r'/\*.*?\*/', ' ', s, flags=re.S)
    flat = re.sub(r'^\s*#.*$', ' ', flat, flags=re.M)
    flat = re.sub(r'\s*\n\s*', ' ', flat)
    protos = {}
    for m in re.finditer(r'([A-Za-z_][\w \*]*?)\s*\b(pe_plat_\w+)\s*\(([^;{}()]*)\)\s*;', flat):
        protos.setdefault(m.group(2), (m.group(1).strip(), m.group(3).strip()))
    have = set(re.findall(r'\b(func_[0-9A-Fa-f]{8})\s*\([^;{]*\)\s*;', flat))
    new, missing = [], []
    for m in DEF.finditer(s):
        fx, py = m.group(1), m.group(2)
        if fx in have:
            continue
        if py not in protos:
            missing.append((fx, py))
            continue
        ret, prm = protos[py]
        ret = re.sub(r'^(extern|static|inline)\s+', '', ret)
        ret = re.sub(r'\bpe_plat_guest_addr\b', 'uint32_t', ret)
        prm = re.sub(r'\bpe_plat_guest_addr\b', 'uint32_t', prm)
        new.append('%s %s(%s);' % (ret, fx, prm or 'void'))
    return s, new, missing


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    g = ap.add_mutually_exclusive_group(required=True)
    g.add_argument('--check', action='store_true')
    g.add_argument('--write', action='store_true')
    args = ap.parse_args()
    bad = 0
    for path in sorted(glob.glob('pc_port/include/pe_plat/*.h')):
        s, new, missing = scan(path)
        for fx, py in missing:
            print('MISSING provider prototype: %s %s -> %s' % (path, fx, py))
            bad = 1
        if not new:
            continue
        if args.check:
            print('%s: %d binding(s) without a canonical prototype' % (path, len(new)))
            bad = 1
            continue
        guard = s.rstrip().rfind('#endif')
        block = ('/* Canonical guest-name prototypes for the bindings above (generated\n'
                 ' * from the pe_plat prototypes; tools read callee signatures from these). */\n'
                 + '\n'.join(new) + '\n')
        s = s[:guard] + block + s[guard:] if guard != -1 else s + block
        open(path, 'w').write(s)
        print('%s: added %d' % (path, len(new)))
    if args.check and not bad:
        print('pe_plat_canonical_protos: every binding has a canonical prototype')
    return bad


if __name__ == '__main__':
    sys.exit(main())
