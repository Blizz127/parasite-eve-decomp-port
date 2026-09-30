import unittest

from maspsx import MaspsxProcessor


class TestExplicitLoStorePassthrough(unittest.TestCase):
    """gcc-2.8.1-psx cc1 emits absolute stores as an explicit pair
    ``lui $b,%hi(SYM)`` / ``op $r,%lo(SYM)($b)`` (2.7.2 only ever emits the
    ``op $r,SYM`` macro). The ``%lo`` store is already a single instruction
    with a LO16 reloc and must pass through untouched — the load arm always
    did this; the store arm used to re-expand it into the 4-word ``$at``
    macro, which broke every ERA_CC1_VER=2.8.1 leaf with an absolute store."""

    @staticmethod
    def process(lines):
        return MaspsxProcessor(lines).process_lines()

    def test_lo_store_in_call_delay_slot_is_left_alone(self):
        lines = [
            "lui\t$2,%hi(D_8009B438) # high",
            ".set\tnoreorder",
            ".set\tnomacro",
            "jal\tfunc_80085DC4",
            "sw\t$4,%lo(D_8009B438)($2)",
            ".set\tmacro",
            ".set\treorder",
        ]
        out = self.process(lines)
        self.assertNotIn("# EXPAND_AT START", out)
        self.assertTrue(
            any(line.startswith("sw\t$4,%lo(D_8009B438)($2)") for line in out),
            out,
        )
        # The slot instruction follows the jal directly: no nop is inserted.
        jal = next(i for i, line in enumerate(out) if line.startswith("jal"))
        self.assertTrue(out[jal + 1].startswith("sw\t$4,%lo(D_8009B438)($2)"), out)

    def test_lo_byte_and_half_stores_pass_through(self):
        for op in ("sb", "sh", "sw"):
            out = self.process([f"{op}\t$5,%lo(D_800A0000)($3)"])
            self.assertEqual(len(out), 1, out)
            self.assertTrue(out[0].startswith(f"{op}\t$5,%lo(D_800A0000)($3)"), out)

    def test_macro_store_keeps_its_legacy_path(self):
        # The 2.7.2 forms are unchanged by the passthrough: a bare absolute
        # symbol store is left to GNU as in both dialects, and an indexed
        # symbolic store still takes the $at macro path under ASPSX 2.21.
        for addiu_at in (False, True):
            out = MaspsxProcessor(["sw\t$4,D_8009B438"], addiu_at=addiu_at).process_lines()
            self.assertEqual(out, ["sw\t$4,D_8009B438"])
        out = MaspsxProcessor(["sw\t$4,D_8009B438($3)"], addiu_at=True).process_lines()
        self.assertTrue(any("lui\t$at,%hi(D_8009B438)" in line for line in out), out)


if __name__ == "__main__":
    unittest.main()
