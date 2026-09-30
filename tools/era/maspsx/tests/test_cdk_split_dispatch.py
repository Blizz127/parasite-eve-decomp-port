import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

# gcc-2.7.2-cdk split-address switch dispatch (func_80071A84, retail 0x80071D24:
# beqz v0 / sll v0,v1,2 (slot) / lui at,%hi(jtbl) / addu at,at,v0 / lw v0,%lo(jtbl)(at)).
CDK = [
    ".text",
    "sltu\t$2,$3,45",
    ".set\tnoreorder",
    ".set\tnomacro",
    "beq\t$2,$0,$L113",
    "lui\t$2,%hi($L116) # high",
    ".set\tmacro",
    ".set\treorder",
    "",
    "addiu\t$2,$2,%lo($L116) # low",
    "sll\t$3,$3,2",
    "addu\t$3,$3,$2",
    "lw\t$2,0($3)",
    "#nop",
    "j\t$2",
    ".rdata",
    ".align\t3",
    "$L116:",
    ".word\t$L40",
    ".text",
    "$L40:",
    "j\t$31",
]


class TestCdkSplitDispatch(unittest.TestCase):
    @staticmethod
    def process(lines, enabled, **kw):
        env = {"MASPSX_CDK_SPLIT_DISPATCH": "1" if enabled else "0"}
        with patch.dict(os.environ, env):
            return MaspsxProcessor(list(lines), **kw).process_lines()

    def test_rewrites_to_indexed_table_load(self):
        out = self.process(CDK, True)
        i = out.index("beq\t$2,$0,$L113")
        self.assertEqual(out[i + 1], "sll\t$2,$3,2")
        self.assertFalse(any(l.startswith("lui\t$2,%hi($L116)") for l in out), out)
        self.assertFalse(any(l.startswith("addiu") and "%lo($L116)" in l for l in out), out)
        self.assertIn("lw\t$2,%lo($L116)($at)", out)

    def test_composes_with_dispatch_fold(self):
        with patch.dict(os.environ, {"MASPSX_THREE_WORD_SYMBOL_STORE": "1",
                                     "MASPSX_DISPATCH_FOLD": "jtbl_80011644"}):
            out = self.process(CDK, True)
        self.assertIn("lui\t$at,%hi(jtbl_80011644)", out)
        self.assertIn("lw\t$2,%lo(jtbl_80011644)($at)", out)

    def test_gate_off_is_identical(self):
        with patch.dict(os.environ, {"MASPSX_CDK_SPLIT_DISPATCH": "0"}):
            ref = MaspsxProcessor(list(CDK)).process_lines()
        with patch.dict(os.environ, {}, clear=False):
            os.environ.pop("MASPSX_CDK_SPLIT_DISPATCH", None)
            unset = MaspsxProcessor(list(CDK)).process_lines()
        self.assertEqual(ref, unset)
        self.assertTrue(any(l.startswith("lui\t$2,%hi($L116)") for l in ref), ref)

    def test_requires_jump_through_loaded_register(self):
        lines = [l if l != "j\t$2" else "j\t$4" for l in CDK]
        out = self.process(lines, True)
        self.assertTrue(any(l.startswith("lui\t$2,%hi($L116)") for l in out), out)

    def test_requires_distinct_index_register(self):
        lines = [l.replace("$3", "$2") if l.startswith(("sll", "addu", "lw\t$2,0(")) else l
                 for l in CDK]
        out = self.process(lines, True)
        self.assertTrue(any(l.startswith("lui\t$2,%hi($L116)") for l in out), out)


if __name__ == "__main__":
    unittest.main()
