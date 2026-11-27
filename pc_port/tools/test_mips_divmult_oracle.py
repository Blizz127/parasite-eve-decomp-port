"""ISA-level checks for the test interpreter's multiply / divide unit (MULT, MULTU, DIV, DIVU, MFHI, MFLO,
MTHI, MTLO, BREAK).  Synthetic instructions are executed through pe_battle_hud_oracle.execute (and through the
rewritten copy in pe_irq_callback_vectors that the black-box vector generators use).  Expected values follow the
R3000 definition: DIV truncates toward zero and the remainder takes the dividend's sign; division by zero does not
trap (DIVU: LO = 0xFFFFFFFF, HI = dividend; DIV: LO = 1 for a negative dividend else 0xFFFFFFFF, HI = dividend);
0x80000000 / -1 gives LO = 0x80000000, HI = 0.  The encodings are the ones GNU as emits for
`div $0,$t0,$t1` etc. (cross-checked by test_encodings_match_gnu_as when the vendored toolchain is present).
Run: python3 pc_port/tools/test_mips_divmult_oracle.py
"""
import os
import struct
import subprocess
import tempfile
import unittest

import pe_battle_hud_oracle as hud
import pe_irq_callback_vectors as gen
from pe_battle_hud_oracle import execute

DIV, DIVU, MULT, MULTU = 0x0109001A, 0x0109001B, 0x01090018, 0x01090019   # $0,$t0,$t1
MFHI_V1, MFLO_V0 = 0x00001810, 0x00001012                                  # mfhi $v1 / mflo $v0
MTHI_T0, MTLO_T1 = 0x01000011, 0x01200013
MFHI_V0, MFLO_V1 = 0x00001010, 0x00001812
JR_RA, NOP, BREAK7 = 0x03E00008, 0, 0x0007000D
M = 0xFFFFFFFF


def run(code, t0=0, t1=0, fn=execute, **kw):
    ram = bytearray(0x200000)
    struct.pack_into('<%dI' % len(code), ram, 0x10000, *code)
    return fn(ram, 0x80010000, initial_regs={8: t0 & M, 9: t1 & M, 31: 0}, **kw)


def divmod_prog(op, t0, t1, **kw):
    r = run([op, MFLO_V0, MFHI_V1, JR_RA, NOP], t0, t1, **kw)
    return r[2], r[3]


class Tests(unittest.TestCase):
    engines = (execute, gen.execute)

    def check(self, op, t0, t1, lo, hi):
        for fn in self.engines:
            for traps in (True, False):
                if traps and t1 == 0:
                    continue
                self.assertEqual(divmod_prog(op, t0, t1, fn=fn, div_zero_traps=traps), (lo & M, hi & M),
                                 (op, t0, t1, fn.__module__, traps))

    def test_div_sign_combinations(self):
        self.check(DIV, 7, 2, 3, 1)
        self.check(DIV, -7, 2, -3, -1)       # remainder has the dividend's sign
        self.check(DIV, 7, -2, -3, 1)
        self.check(DIV, -7, -2, 3, -1)
        self.check(DIV, 0, 5, 0, 0)
        self.check(DIV, 5, 7, 0, 5)
        self.check(DIV, -5, 7, 0, -5)
        self.check(DIV, 0x7FFFFFFF, 1, 0x7FFFFFFF, 0)
        self.check(DIV, 0x80000000, 1, 0x80000000, 0)
        self.check(DIV, 0x80000000, 2, 0xC0000000, 0)

    def test_div_min_by_minus_one(self):
        self.check(DIV, 0x80000000, -1, 0x80000000, 0)
        self.check(DIV, 0x80000001, -1, 0x7FFFFFFF, 0)

    def test_div_by_zero_does_not_trap(self):
        self.check(DIV, 5, 0, M, 5)          # non-negative dividend: LO = -1, HI = dividend
        self.check(DIV, 0, 0, M, 0)
        self.check(DIV, -5, 0, 1, -5)        # negative dividend: LO = 1
        self.check(DIV, 0x80000000, 0, 1, 0x80000000)
        self.check(DIV, 0x7FFFFFFF, 0, M, 0x7FFFFFFF)

    def test_divu(self):
        self.check(DIVU, 7, 2, 3, 1)
        self.check(DIVU, M, 2, 0x7FFFFFFF, 1)
        self.check(DIVU, 0x80000000, M, 0, 0x80000000)
        self.check(DIVU, M, M, 1, 0)
        self.check(DIVU, 5, 7, 0, 5)
        self.check(DIVU, 0x80000000, 0x10000, 0x8000, 0)
        self.check(DIVU, 0x12345678, 0x1000, 0x12345, 0x678)

    def test_divu_by_zero_does_not_trap(self):
        self.check(DIVU, 5, 0, M, 5)
        self.check(DIVU, 0, 0, M, 0)
        self.check(DIVU, M, 0, M, M)         # LO = 0xFFFFFFFF, HI = dividend (not sign dependent)
        self.check(DIVU, 0x80000000, 0, M, 0x80000000)

    def test_default_still_traps_on_zero_divisor(self):
        """the fixture oracles (pe_transition_*, pe_object_render) rely on this assertion"""
        for op in (DIV, DIVU):
            with self.assertRaises(AssertionError) as cm:
                divmod_prog(op, 9, 0)
            self.assertEqual(cm.exception.args[0][0], 'division by zero in original fixture')

    def test_mult_multu(self):
        def mm(op, a, b):
            r = run([op, MFLO_V0, MFHI_V1, JR_RA, NOP], a, b)
            return r[2], r[3]
        self.assertEqual(mm(MULT, 0x80000000, 0x80000000), (0, 0x40000000))
        self.assertEqual(mm(MULTU, 0x80000000, 0x80000000), (0, 0x40000000))
        self.assertEqual(mm(MULT, M, M), (1, 0))                      # (-1) * (-1)
        self.assertEqual(mm(MULTU, M, M), (1, 0xFFFFFFFE))
        self.assertEqual(mm(MULT, M, 5), (0xFFFFFFFB, M))             # -1 * 5
        self.assertEqual(mm(MULTU, M, 5), (0xFFFFFFFB, 4))
        self.assertEqual(mm(MULT, 0x7FFFFFFF, 0x7FFFFFFF), (1, 0x3FFFFFFF))
        self.assertEqual(mm(MULT, 0x80000000, 2), (0, M))             # -2^31 * 2 = -2^32

    def test_mthi_mtlo_roundtrip(self):
        r = run([MTHI_T0, MTLO_T1, MFHI_V0, MFLO_V1, JR_RA, NOP], 0xDEADBEEF, 0x12345678)
        self.assertEqual((r[2], r[3]), (0xDEADBEEF, 0x12345678))
        # MTLO / MTHI write one half only; the other survives a following mfhi/mflo
        r = run([DIV, MTLO_T1, MFHI_V0, MFLO_V1, JR_RA, NOP], 7, 0xABCD0001)
        self.assertEqual((r[2], r[3]), (7, 0xABCD0001))     # DIV leaves HI = 7 (|dividend| < |divisor|); MTLO then replaces LO only

    def test_div_in_delay_slot(self):
        # `j L` with the DIVU in its delay slot: HI/LO are produced before control reaches L
        j = 0x08000000 | ((0x80010010 & 0x0FFFFFFF) >> 2)
        r = run([j, DIVU, NOP, NOP, MFLO_V0, MFHI_V1, JR_RA, NOP], 100, 7)
        self.assertEqual((r[2], r[3]), (14, 2))

    def test_break_raises_a_distinct_assertion(self):
        with self.assertRaises(hud.MipsBreak):
            run([BREAK7, NOP])
        self.assertTrue(issubclass(hud.MipsBreak, AssertionError))

    def test_guarded_divu_like_the_retail_libspu_sequence(self):
        """divu zero,a1,a0 ; bnez a0,+2 ; nop ; break 7 ; mfhi v0  (retail 0x8007DB44)"""
        code = [0x00A4001B, 0x14800002, NOP, BREAK7, 0x00001010, JR_RA, NOP]
        ram = bytearray(0x200000)
        struct.pack_into('<7I', ram, 0x10000, *code)
        r = execute(ram, 0x80010000, initial_regs={4: 0x30, 5: 0x1234, 31: 0})
        self.assertEqual(r[2], 0x1234 % 0x30)
        with self.assertRaises(hud.MipsBreak):
            execute(ram, 0x80010000, initial_regs={4: 0, 5: 0x1234, 31: 0}, div_zero_traps=False)

    def test_encodings_match_gnu_as(self):
        root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools', 'mipsel-host', 'bin')
        gas = os.path.join(root, 'mipsel-linux-gnu-as')
        if not os.path.exists(gas):
            self.skipTest('vendored mipsel toolchain not present')
        src = ('.set noreorder\n.set noat\ndiv $0,$8,$9\ndivu $0,$8,$9\nmult $8,$9\nmultu $8,$9\n'
               'mthi $8\nmtlo $9\nmfhi $3\nmflo $2\nmfhi $2\nmflo $3\nbreak 7\n')
        with tempfile.TemporaryDirectory() as d:
            with open(d + '/a.s', 'w') as f:
                f.write(src)
            subprocess.run([gas, '-march=r3000', '-o', d + '/a.o', d + '/a.s'], check=True)
            subprocess.run([os.path.join(root, 'mipsel-linux-gnu-objcopy'), '-O', 'binary', '-j', '.text',
                            d + '/a.o', d + '/a.bin'], check=True)
            with open(d + '/a.bin', 'rb') as f:
                words = struct.unpack('<11I', f.read()[:44])      # gas pads .text to a 16-byte multiple
        self.assertEqual(words, (DIV, DIVU, MULT, MULTU, MTHI_T0, MTLO_T1, MFHI_V1, MFLO_V0, MFHI_V0, MFLO_V1, BREAK7))


if __name__ == '__main__':
    unittest.main()
