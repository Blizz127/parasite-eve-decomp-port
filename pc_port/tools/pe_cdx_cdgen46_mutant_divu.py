#!/usr/bin/env python3
"""Interpreter-mutant control for batch cdgen46: regenerate the vectors with a deliberately wrong DIVU (LO and HI
swapped).  The vectors must differ from the real ones in every case (the DIVU result reaches observable stores).
Run from the repo root: python3 pc_port/tools/pe_cdx_cdgen46_mutant_divu.py OUT.txt"""
import sys
sys.path.insert(0, 'pc_port/tools')
import pe_battle_hud_oracle as h   # noqa: E402
_orig = h.mips_divu
h.mips_divu = lambda n, d: _orig(n, d)[::-1]
sys.argv = ['pe_cdx_vectors.py', sys.argv[1], 'cdgen46', '-n', '200']
import pe_cdx_vectors   # noqa: E402  (imports pe_irq_callback_vectors after the patch, so the exec'd interpreter copy sees it)
pe_cdx_vectors.main()
