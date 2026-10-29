#!/usr/bin/env python3
"""Directed test-only differential for the native ovl_03C9 effect child 801F0068.

func_801F0690 (effect descriptors 0x6E-0x71) passes this child to its record
pool.  This runs the existing e21 raw-retail oracle, unchanged, against
pc_port/game/boot/func_801F0068_effect_native.c.  The extra leaf metadata exists
only in memory: no matched-src or config claim is made.

Calls are mocked on both sides.  Success therefore proves the caller's control
flow, memory and ordered ABI, not the actual rendering or callee behaviour.

Every callee the native file declares `void` gets a distinctive nonzero mock
return.  A retail read of v0 after such a call would then show up as a
mismatch, and a pass confirms the void declaration.
"""
from pathlib import Path
import argparse
import json
import os
import sys
ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools/analysis'))
if os.environ.get('LIBERATE_PYLIB'):
    sys.path.insert(0, os.environ['LIBERATE_PYLIB'])
import e21_diff as e

NAME, ADDR, OFF, SIZE = 'func_801F0068', 0x801F0068, 0x2870, 0x628
VOID_CALLEES = (0x800C6EC0, 0x800C6ED8, 0x800C6EF8, 0x800C6F4C, 0x800C6FA0, 0x800C71E4,
                0x800CEDA8, 0x800CF3AC, 0x800CFB7C, 0x800D0728, 0x800D2370)
SPECS = [
    # mode 1 (update): wait 0 lives 32 steps, wait 1 moves and lives 40, wait 2 turns and lives 32
    'a0=1; a1=0x80140000; h(a1+0x10)=0; h(a1+0x12)=0,1,30,31,32,33,32767,65535',
    'a0=1; a1=0x80140000; h(a1+0x10)=1; h(a1+0x12)=0,1,38,39,40,41,65535',
    'a0=1; a1=0x80140000; h(a1+0x10)=2; h(a1+0x12)=0,1,30,31,32,65535',
    'a0=1; a1=0x80140000; h(a1+0x10)=3,4,65535',
    # mode 2 (draw) for each wait value
    'a0=2; a1=0x80140000; h(a1+0x10)=0; h(a1+0x12)=0,1,15,16,31,32',
    'a0=2; a1=0x80140000; h(a1+0x10)=1; h(a1+0x12)=0,1,20,39,40',
    'a0=2; a1=0x80140000; h(a1+0x10)=2; h(a1+0x12)=0,1,16,31,32',
    'a0=2; a1=0x80140000; h(a1+0x10)=3,65535',
    # draw with lighting mode 4: the texture-coordinate bump keyed on D_800F3428
    'a0=2; a1=0x80140000; h(a1+0x10)=0; h(a1+0x12)=0,1,16; h(0x800F336C)=4; w(0x800F3428)=0,1',
    'a0=2; a1=0x80140000; h(a1+0x10)=1; h(a1+0x12)=0,1,20; h(0x800F336C)=4; w(0x800F3428)=0,1',
    'a0=2; a1=0x80140000; h(a1+0x10)=2; h(a1+0x12)=0,1,16; h(0x800F336C)=4; w(0x800F3428)=0,1',
    # out-of-range modes
    'a0=0,3,4294967295; a1=0x80140000',
]
COMMON = ('; h(0x800E11E6)=0; w(0x800F33E0)=0x80162000; w(0x80162008)=0x80163000; '
          'w(0x800F32D0)=0x80160000; w(0x80160008)=0x80161000; w(0x800BCFA4)=0x80164000; '
          'w(0x8009D254)=0x80165000; '
          + '; '.join(f'r(0x{a:08X})=0x{0x5A5A0001 + i:08X}' for i, a in enumerate(VOID_CALLEES)))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--seeds', type=int, default=64)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--native-frame', action='store_true')
    ns = ap.parse_args()
    # Same raw frame address for both sides unless --native-frame (the
    # production arena), as the Liberate checker does.
    if not ns.native_frame:
        e.HARNESS = e.HARNESS.replace('static pe_addr_t fsp = 0x801FEC00u;', f'static pe_addr_t fsp = 0x{e.SP0:08X}u;').replace('a[i] < 0x801FF000u', 'a[i] < 0x80200000u')
    os.environ.pop('E21_STACK_COINCIDE', None)
    old_run = e.run_mips
    metrics = {'attempts': 0, 'faults': []}

    def judged(*args, **kwargs):
        metrics['attempts'] += 1
        prior = set(e.COVER) if e.COVER is not None else None
        try:
            return old_run(*args, **kwargs)
        except e.Fault as fault:
            metrics['faults'].append({'seed': args[3], 'reason': str(fault)})
            if prior is not None:
                e.COVER.clear(); e.COVER.update(prior)
            raise
    e.run_mips = judged
    old_parse = e.o.parse_overlay

    def parse(ovl):
        ov = old_parse(ovl)
        if ovl == 'ovl_03C9':
            ov['leaves'][NAME] = {'name': NAME, 'file_offset': OFF, 'file_size': SIZE, 'vram': ADDR,
                                  'words': SIZE // 4, 'nonmatching': True}
        return ov
    e.o.parse_overlay = parse
    results = []
    covered = set()
    for i, spec in enumerate(SPECS):
        spec += COMMON
        p = ROOT / 'pc_port/game/boot' / f'{NAME}_effect_native.c'
        metrics['attempts'] = 0; metrics['faults'] = []
        code, detail = e.one(p, NAME, 'ovl_03C9', ns.seeds, 'cc', True,
                             directed={('ovl_03C9', NAME): [e.parse_directed(spec)]}, check_gte=True)
        covered |= set(e.COVER or [])
        results.append({'function': NAME, 'spec_index': i, 'spec': spec, 'status': code, 'detail': detail,
                        'covered_pcs': sorted(e.COVER or []),
                        'coverage_scope': 'completed oracle cases only; equivalence contingent on status ok',
                        'frame_layout': 'production arena' if ns.native_frame else 'same raw frame address',
                        'attempts': metrics['attempts'], 'oracle_faults': list(metrics['faults'])})
        print(NAME, i, code, detail, flush=True)
        ns.output.write_text(json.dumps(results, indent=2) + '\n')
        if code != 'ok':
            return 1
    words = {ADDR + 4 * k for k in range(SIZE // 4)}
    print(f'{NAME} covered words {len(words & covered)}/{len(words)}', flush=True)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
