#!/usr/bin/env python3
"""cdgen49: render the generator's OWN translation of retail 0x8006E6D4 into a scratch directory, without touching the tree.

tools/analysis/gen_decomp_ports.py skips func_8006E6D4 because platform/pe_libcd.c defines it (collision).  This wrapper masks
that one definition (monkeypatches pc_port_definitions in memory; no file is edited) and runs the generator with --e21 (the
guest-memory lowering mode: the verbatim path refuses the leaf because `&loc` is a host pointer passed to func_80080B44) into
OUT_DIR.  Needs pycparser (tools/requirements-port.txt pins 3.11; the run recorded in the REPORT used what PYTHONPATH had).
Usage: PYTHONPATH=<dir with pycparser/> python3 pe_cdx_cdgen49_gen6e6d4.py OUT_DIR   (run from the repo root)"""
import pathlib
import sys

sys.path.insert(0, 'tools/analysis')
import gen_decomp_ports as g   # noqa: E402

orig = g.pc_port_definitions


def patched():
    d = orig()
    s = d.get('func_8006E6D4')
    if s:
        s.discard('pc_port/platform/pe_libcd.c')
        if not s:
            del d['func_8006E6D4']
    return d


g.pc_port_definitions = patched
g.OUT_DIR = pathlib.Path(sys.argv[1])
sys.argv = ['gen_decomp_ports.py', '--e21', '--only', 'func_8006E6D4', '--no-compile-check']
raise SystemExit(g.main())
