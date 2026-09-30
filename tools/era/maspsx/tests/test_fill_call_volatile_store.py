import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


class TestFillCallVolatileStore(unittest.TestCase):
    """Patch 18: MASPSX_FILL_CALL_VOLATILE_STORE.

    Retail func_80076354 0x800763C4: `jal func_800773D0` / `sw $v1,0($v0)`
    (the volatile DMA CHCR kick). cc1 emits the volatile store before the
    `jal` (reorg never slots a volatile insn) and maspsx adds a `nop` slot.
    """

    VOLATILE_CALL = [
        "lw\t$2,D_8009586C",
        "ori\t$3,$3,0x0002",
        "#.set\tvolatile",
        "sw\t$3,0($2)",
        "#.set\tnovolatile",
        "jal\tfunc_800773D0",
        "lw\t$2,D_8009586C",
    ]

    @staticmethod
    def process(lines, on=True):
        with patch.dict(os.environ, {"MASPSX_FILL_CALL_VOLATILE_STORE": "0"}):
            out = MaspsxProcessor(lines, fill_call_volatile_store=on).process_lines()
        return [x for x in strip_comments(out) if x and not x.startswith(".")]

    def around_call(self, res, name):
        i = next(k for k, x in enumerate(res) if x.startswith("jal") and name in x)
        return res[i - 1 : i + 2]

    def test_positive_volatile_store_into_jal_slot(self):
        res = self.process(self.VOLATILE_CALL)
        self.assertEqual(
            ["ori\t$3,$3,0x0002", "jal\tfunc_800773D0", "sw\t$3,0($2)"],
            self.around_call(res, "func_800773D0"),
        )

    def test_positive_environment_flag(self):
        with patch.dict(os.environ, {"MASPSX_FILL_CALL_VOLATILE_STORE": "1"}):
            out = MaspsxProcessor(self.VOLATILE_CALL).process_lines()
        res = [x for x in strip_comments(out) if x and not x.startswith(".")]
        self.assertEqual("sw\t$3,0($2)", self.around_call(res, "func_800773D0")[2])

    def test_negative_flag_off_keeps_nop_slot(self):
        res = self.process(self.VOLATILE_CALL, on=False)
        self.assertEqual(
            ["sw\t$3,0($2)", "jal\tfunc_800773D0", "nop"],
            self.around_call(res, "func_800773D0"),
        )

    def test_negative_value_register_ra(self):
        # HC1: jal writes $31 before the slot runs.
        lines = ["sw\t$31,0($2)", "jal\tfoo"]
        self.assertEqual(self.process(lines, on=False), self.process(lines))

    def test_negative_base_register_ra(self):
        # HC1: nor may the moved store address through $31.
        lines = ["sw\t$2,4($31)", "jal\tfoo"]
        self.assertEqual(self.process(lines, on=False), self.process(lines))

    def test_negative_jalr(self):
        # HC2: register-indirect calls are refused.
        lines = ["sw\t$3,0($4)", "jal\t$2"]
        self.assertEqual(self.process(lines, on=False), self.process(lines))

    def test_negative_plain_j(self):
        # HC2: `j <label>` is patch 11's (stack stores only), not this knob.
        lines = ["sw\t$3,0($4)", "j\t$L5", "$L5:", "addu\t$2,$3,$4"]
        self.assertEqual(self.process(lines, on=False), self.process(lines))

    def test_negative_label_between(self):
        # HC5: a label means the call is a branch target.
        lines = ["sw\t$3,0($4)", "$L2:", "jal\tfoo"]
        self.assertEqual(self.process(lines, on=False), self.process(lines))

    def test_negative_symbol_store(self):
        # A symbolic store is a macro (patch 8's second-word arm), not this.
        lines = ["sw\t$3,D_80001000", "jal\tfoo"]
        self.assertEqual(self.process(lines, on=False), self.process(lines))

    def test_negative_large_offset(self):
        # HC3: an out-of-range offset expands to an $at macro.
        lines = ["sw\t$3,56200($4)", "jal\tfoo"]
        self.assertEqual(self.process(lines, on=False), self.process(lines))

    def test_negative_noreorder_block(self):
        # HC5: inside cc1's noreorder block the slot is cc1's.
        lines = [".set\tnoreorder", "sw\t$3,0($4)", "jal\tfoo", "nop",
                 ".set\treorder"]
        self.assertEqual(self.process(lines, on=False), self.process(lines))

    def test_negative_not_before_call(self):
        lines = ["sw\t$3,0($4)", "addu\t$2,$3,$4", "jal\tfoo"]
        self.assertEqual(self.process(lines, on=False), self.process(lines))


if __name__ == "__main__":
    unittest.main()
