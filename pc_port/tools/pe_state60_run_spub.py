#!/usr/bin/env python3
"""Run the state60a rows 14-26 checker (pe_state60_spub_check.c) and the m1/m2/m3 mutant controls.

usage (from the repository root): pe_state60_run_spub.py VECTOR_ROOT OUT.json
  VECTOR_ROOT/state60a/func_*.txt are the vector files; only the 13 functions of this batch are replayed
  (a temporary directory of symlinks to those files is built, so pe_state60_run.py itself is unmodified).
The family 'spub' -> batch dir state60a and the header pc_port/include/pe_plat/spu_b.h (for the m3 no-op object)
are registered on the imported pe_state60_run module."""
import importlib.util
import os
import sys
import tempfile

MINE = ['80085B34', '80085BB4', '80085C44', '80085D84', '80085DC4', '80085EB4', '80085F14',
        '80085F44', '80085F74', '800862F4', '80089EB8', '80089F08', '80089F28']


def main():
    root, outp = sys.argv[1], sys.argv[2]
    here = os.path.dirname(os.path.abspath(__file__))
    spec = importlib.util.spec_from_file_location('pe_state60_run', os.path.join(here, 'pe_state60_run.py'))
    m = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    m.FAMS['spub'] = 'state60a'
    m.SYMS['spub'] = 'pc_port/include/pe_plat/spu_b.h'
    for k in [k for k, h in m.SYMS.items() if not os.path.isfile(h)]:   # other authors' headers are absent on this branch
        del m.SYMS[k]
    with tempfile.TemporaryDirectory(dir='build') as tmp:
        d = os.path.join(tmp, 'state60a')
        os.makedirs(d)
        for n in MINE:
            os.symlink(os.path.join(os.path.abspath(root), 'state60a', 'func_%s.txt' % n), os.path.join(d, 'func_%s.txt' % n))
        sys.argv = ['pe_state60_run.py', tmp, outp, 'spub']
        m.main()


if __name__ == '__main__':
    main()
