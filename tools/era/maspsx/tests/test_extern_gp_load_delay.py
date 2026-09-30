import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments

# Local knobs that must not leak in from the caller's shell (the production
# per-leaf opt-ins are env-gated).  "0" is "off" for every one of them.
_QUIET_KNOBS = {
    "MASPSX_FILL_STORE_DELAY_SLOT": "0",
    "MASPSX_FILL_EPILOGUE_DELAY_SLOT": "0",
    "MASPSX_FILL_BRANCH_DELAY_SLOT": "0",
    "MASPSX_THREE_WORD_SYMBOL_STORE": "0",
    "MASPSX_SYMBOL_LOAD_DEST_TEMP": "0",
    "MASPSX_SYMBOL_AT_TEMP": "0",
}


def _wrap(body):
    """Wrap a body in the minimal cc1 function scaffolding maspsx expects."""
    return [
        "\t.text",
        "\t.ent\tfunc",
        "func:",
        *body,
        "\t.end\tfunc",
    ]


# The retail func_800339A0 shape (0x800339FC): cc1 -O2 -G8 declares the
# halfword global with `.extern SYM, 2`, so GNU as places it gp-relative and
# the store macro collapses to ONE instruction.  The loaded $2 is consumed by
# that single `sh`, so the MIPS-I load-delay nop is mandatory:
#   lhu $v0,0x0($v1) / nop / sh $v0,0x114($gp)
HALF_STORE = _wrap(
    [
        "\t.extern\tD_8009CE84, 2",
        "lhu\t$2,0($3)",
        "#nop",
        "sh\t$2,D_8009CE84",
        "j\t$31",
    ]
)

# The retail func_80046DFC shape (0x80046E7C): a gp word copied to a gp word.
#   lw $v0,0x19C($gp) / nop / sw $v0,0x248($gp)
WORD_COPY = _wrap(
    [
        "\t.extern\tD_8009CF0C, 4",
        "\t.extern\tD_8009CFB8, 4",
        "lw\t$2,D_8009CF0C",
        "#nop",
        "sw\t$2,D_8009CFB8",
        "j\t$31",
    ]
)

# The retail func_80012700 shape (0x80012708): a struct word copied to a gp word.
#   lw $v0,0x24($a2) / nop / sw $v0,0x8C($gp)
STRUCT_TO_GP = _wrap(
    [
        "\t.extern\tD_8009CDFC, 4",
        "lw\t$2,36($6)",
        "#nop",
        "sw\t$2,D_8009CDFC",
        "j\t$31",
    ]
)

# An indexed symbolic load whose base register was just loaded.  Whatever the
# macro expands to, its FIRST word reads the base register, so the nop is
# needed whether the symbol is gp-relative or absolute.
INDEXED_THROUGH_LOADED_BASE = _wrap(
    [
        "\t.extern\tD_8009CE00, 4",
        "lw\t$2,0($4)",
        "#nop",
        "lw\t$3,D_8009CE00($2)",
        "j\t$31",
    ]
)


def _without_extern(lines):
    return [line for line in lines if not line.lstrip().startswith(".extern")]


class TestExternGpLoadDelay(unittest.TestCase):
    @staticmethod
    def process(lines, sdata_limit, nop_at_expansion):
        with patch.dict(os.environ, _QUIET_KNOBS):
            processor = MaspsxProcessor(
                lines,
                sdata_limit=sdata_limit,
                nop_at_expansion=nop_at_expansion,
            )
            return strip_comments(processor.process_lines())

    def assert_nop_between(self, out, first, second):
        """`first` must be immediately followed by exactly one `nop`, then `second`."""
        i = out.index(first)
        self.assertEqual(
            out[i : i + 3],
            [first, "nop", second],
            f"expected a load-delay nop between {first!r} and {second!r}; got {out[i:i+3]!r}",
        )

    def assert_no_nop_between(self, out, first, second):
        i = out.index(first)
        self.assertEqual(
            out[i : i + 2],
            [first, second],
            f"expected {first!r} to be followed directly by {second!r}; got {out[i:i+2]!r}",
        )

    # --- the three retail breakers, at ASPSX >= 2.30 semantics --------------

    def test_half_store_gets_load_delay_nop_at_2_30(self):
        out = self.process(HALF_STORE, sdata_limit=8, nop_at_expansion=False)
        self.assert_nop_between(out, "lhu\t$2,0($3)", "sh\t$2,D_8009CE84")

    def test_gp_word_copy_gets_load_delay_nop_at_2_30(self):
        out = self.process(WORD_COPY, sdata_limit=8, nop_at_expansion=False)
        self.assert_nop_between(out, "lw\t$2,D_8009CF0C", "sw\t$2,D_8009CFB8")

    def test_struct_to_gp_word_gets_load_delay_nop_at_2_30(self):
        out = self.process(STRUCT_TO_GP, sdata_limit=8, nop_at_expansion=False)
        self.assert_nop_between(out, "lw\t$2,36($6)", "sw\t$2,D_8009CDFC")

    def test_indexed_load_through_loaded_base_gets_nop_at_2_30(self):
        out = self.process(
            INDEXED_THROUGH_LOADED_BASE, sdata_limit=8, nop_at_expansion=False
        )
        i = out.index("lw\t$2,0($4)")
        self.assertEqual(out[i + 1], "nop", out[i : i + 4])

    # --- the fix is gated on the symbol actually resolving gp-relative -----

    def test_absolute_symbol_keeps_no_nop_at_2_30(self):
        # No `.extern` size: GNU as expands `sh $2,SYM` to `lui $at` / `sh`, and
        # the `lui` fills the load-delay slot.  Retail has 138 such sites with
        # no nop and zero with one (ASPSX 2.21 inserted a spurious nop here).
        out = self.process(
            _without_extern(HALF_STORE), sdata_limit=8, nop_at_expansion=False
        )
        self.assert_no_nop_between(out, "lhu\t$2,0($3)", "sh\t$2,D_8009CE84")

    def test_extern_larger_than_sdata_limit_is_absolute(self):
        lines = [
            line.replace(".extern\tD_8009CE84, 2", ".extern\tD_8009CE84, 16")
            for line in HALF_STORE
        ]
        out = self.process(lines, sdata_limit=8, nop_at_expansion=False)
        self.assert_no_nop_between(out, "lhu\t$2,0($3)", "sh\t$2,D_8009CE84")

    def test_sdata_limit_zero_disables_the_classification(self):
        # -G0: nothing is gp-relative, so the `.extern` size must be ignored
        # (cc1 -G0 does not emit sized `.extern` lines, but the gate must not
        # depend on that).
        out = self.process(HALF_STORE, sdata_limit=0, nop_at_expansion=False)
        self.assert_no_nop_between(out, "lhu\t$2,0($3)", "sh\t$2,D_8009CE84")

    def test_unrelated_register_needs_no_nop(self):
        lines = [line.replace("sh\t$2,", "sh\t$4,") for line in HALF_STORE]
        out = self.process(lines, sdata_limit=8, nop_at_expansion=False)
        self.assert_no_nop_between(out, "lhu\t$2,0($3)", "sh\t$4,D_8009CE84")

    # --- parsing and non-interference ----------------------------------------

    def test_extern_without_space_after_comma(self):
        lines = [
            line.replace(".extern\tD_8009CE84, 2", ".extern\tD_8009CE84,2")
            for line in HALF_STORE
        ]
        out = self.process(lines, sdata_limit=8, nop_at_expansion=False)
        self.assert_nop_between(out, "lhu\t$2,0($3)", "sh\t$2,D_8009CE84")

    def test_store_line_is_left_for_gas_to_place(self):
        # The fix only feeds the hazard decision; the gp-relative rewrite stays
        # GNU as's job (it already honours the sized `.extern`), so the store
        # is emitted verbatim rather than as an explicit %gp_rel operand.
        out = self.process(HALF_STORE, sdata_limit=8, nop_at_expansion=False)
        self.assertIn("sh\t$2,D_8009CE84", out)
        self.assertFalse(any("%gp_rel" in line for line in out), out)

    def test_extern_line_is_passed_through(self):
        out = self.process(HALF_STORE, sdata_limit=8, nop_at_expansion=False)
        self.assertIn(".extern\tD_8009CE84, 2", out)

    # --- ASPSX < 2.30 semantics are unchanged --------------------------------

    def test_2_21_still_emits_the_nop(self):
        for lines in (HALF_STORE, _without_extern(HALF_STORE)):
            out = self.process(lines, sdata_limit=8, nop_at_expansion=True)
            self.assert_nop_between(out, "lhu\t$2,0($3)", "sh\t$2,D_8009CE84")

    def test_2_21_output_identical_with_and_without_sized_extern_classification(self):
        # At < 2.30 the version-gated `nop_at_expansion` clause already covers
        # every $at-consuming successor, so the new clause changes nothing.
        with_limit = self.process(HALF_STORE, sdata_limit=8, nop_at_expansion=True)
        no_limit = self.process(HALF_STORE, sdata_limit=0, nop_at_expansion=True)
        self.assertEqual(with_limit, no_limit)


if __name__ == "__main__":
    unittest.main()
