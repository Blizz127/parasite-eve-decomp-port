import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


class TestLaAbsoluteSmallData(unittest.TestCase):
    """Patch 17: MASPSX_LA_ABSOLUTE_SMALL_DATA (default ON; env "0" opts out).

    Retail func_80044274 (-G8): `sw $v0,0x220($gp)` stores to D_8009CF90
    gp-relative, but the address argument `&D_8009CF90` is
    `lui $a0,0x800A` / `addiu $a0,$a0,-0x3070` (0x80044360). ASPSX < 2.80
    never forms a gp-relative `la`; maspsx left the `la` to GNU as, which
    makes it `addiu $a0,$gp,...` for any small-data symbol.
    """

    EXTERNS = [".extern\tD_8009CF90, 4", ".extern\tD_8009CF94, 4"]

    @staticmethod
    def process(lines, on=True, sdata_limit=8, gp_allow_la=False):
        with patch.dict(os.environ, {"MASPSX_LA_ABSOLUTE_SMALL_DATA": "0"}):
            out = MaspsxProcessor(
                lines,
                sdata_limit=sdata_limit,
                gp_allow_la=gp_allow_la,
                la_absolute_small_data=on,
            ).process_lines()
        return [x for x in strip_comments(out) if x and not x.startswith(".")]

    def test_positive_extern_small_data_la(self):
        res = self.process(self.EXTERNS + ["la\t$4,D_8009CF90"])
        self.assertEqual(
            ["lui\t$4,%hi(D_8009CF90)", "addiu\t$4,$4,%lo(D_8009CF90)"], res
        )

    def test_positive_store_stays_gp_relative(self):
        res = self.process(
            self.EXTERNS + ["sw\t$2,D_8009CF90", "la\t$4,D_8009CF90"]
        )
        # the store is left for GNU as, which honours the sized .extern and
        # places it gp-relative; only the `la` is rewritten.
        self.assertEqual("sw\t$2,D_8009CF90", res[0])
        self.assertEqual("lui\t$4,%hi(D_8009CF90)", res[1])

    def test_positive_addend(self):
        res = self.process(self.EXTERNS + ["la\t$5,D_8009CF90+4"])
        self.assertEqual(
            ["lui\t$5,%hi(D_8009CF90+4)", "addiu\t$5,$5,%lo(D_8009CF90+4)"], res
        )

    def test_positive_local_sdata(self):
        lines = [".sdata", "D_x:", ".word\t0", ".text", "la\t$4,D_x"]
        res = self.process(lines)
        self.assertIn("lui\t$4,%hi(D_x)", res)

    def test_positive_environment_flag(self):
        with patch.dict(os.environ, {"MASPSX_LA_ABSOLUTE_SMALL_DATA": "1"}):
            out = MaspsxProcessor(
                self.EXTERNS + ["la\t$4,D_8009CF90"], sdata_limit=8
            ).process_lines()
        res = [x for x in strip_comments(out) if x and not x.startswith(".")]
        self.assertEqual("lui\t$4,%hi(D_8009CF90)", res[0])

    def test_positive_default_on(self):
        # Default ON: no env var, no ctor argument.
        env = {k: v for k, v in os.environ.items()
               if k != "MASPSX_LA_ABSOLUTE_SMALL_DATA"}
        with patch.dict(os.environ, env, clear=True):
            out = MaspsxProcessor(
                self.EXTERNS + ["la\t$4,D_8009CF90"], sdata_limit=8
            ).process_lines()
        res = [x for x in strip_comments(out) if x and not x.startswith(".")]
        self.assertEqual("lui\t$4,%hi(D_8009CF90)", res[0])

    def test_negative_env_zero_opts_out(self):
        with patch.dict(os.environ, {"MASPSX_LA_ABSOLUTE_SMALL_DATA": "0"}):
            out = MaspsxProcessor(
                self.EXTERNS + ["la\t$4,D_8009CF90"], sdata_limit=8
            ).process_lines()
        res = [x for x in strip_comments(out) if x and not x.startswith(".")]
        self.assertEqual(["la\t$4,D_8009CF90"], res)

    def test_negative_flag_off_passthrough(self):
        # Flag off: the line is left for GNU as exactly as before.
        res = self.process(self.EXTERNS + ["la\t$4,D_8009CF90"], on=False)
        self.assertEqual(["la\t$4,D_8009CF90"], res)

    def test_negative_large_extern(self):
        # HA4: a symbol GNU as places absolute anyway is untouched.
        res = self.process([".extern\tD_big, 16", "la\t$4,D_big"])
        self.assertEqual(["la\t$4,D_big"], res)

    def test_negative_g0(self):
        # -G0: nothing is small data; the store/la arm is not entered.
        res = self.process(["la\t$4,D_8009CF90"], sdata_limit=0)
        self.assertEqual(["la\t$4,D_8009CF90"], res)

    def test_negative_gp_allow_la(self):
        # HA1: ASPSX >= 2.80 (gp_allow_la) does form a gp-relative la.
        res = self.process(
            self.EXTERNS + ["la\t$4,D_8009CF90"], gp_allow_la=True
        )
        self.assertNotIn("lui\t$4,%hi(D_8009CF90)", res)

    def test_negative_indexed_la(self):
        # Indexed forms go through the indexed path, not this arm.
        on = self.process(self.EXTERNS + ["la\t$4,D_8009CF90($2)"])
        off = self.process(self.EXTERNS + ["la\t$4,D_8009CF90($2)"], on=False)
        self.assertEqual(off, on)

    def test_negative_numeric_la(self):
        # HA3: a numeric operand is not a symbol macro.
        on = self.process(["la\t$4,1234"])
        off = self.process(["la\t$4,1234"], on=False)
        self.assertEqual(off, on)


if __name__ == "__main__":
    unittest.main()
