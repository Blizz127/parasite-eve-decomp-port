#!/usr/bin/env python3
"""Regression tests for pc_port_coverage.py's per-definition stub judgement.

A leaf is a `stub` only when EVERY one of its definition bodies contains a
stub marker.  A file that merely also holds a boundary stub for some other
callee must not taint the real definitions beside it (the pre-fix file-level
rule classified 246 real hand ports as stubs that way).  Prototypes and
`extern` declarations are not definitions.

Run: python3 tools/analysis/test_pc_port_coverage.py
"""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from pc_port_coverage import (  # noqa: E402
    STUB_MARKERS,
    classify,
    definition_bodies,
    is_stub_body,
)

TAINTED_FILE = r'''
#include "pe_bootstrap.h"
extern int func_80000010(int a);           /* declaration, not a definition */

int func_80000000(pe_addr_t args)
{
    /* a "}" in a comment and a string "{" must not end the body early */
    PE_StoreU32(args, 1u);
    return 1;
}

static void helper(void)
{
    Bootstrap_ReturnVoid("func_80099999", "helper");
}

void func_80000020(int x)
{
    if (x) {
        (void)Bootstrap_ReturnInt("func_80088888", "func_80000020", 0);
    }
}
'''


class DefinitionBodies(unittest.TestCase):
    def test_declarations_are_not_definitions(self) -> None:
        names = [n for n, _ in definition_bodies(TAINTED_FILE)]
        self.assertEqual(names, ["func_80000000", "func_80000020"])

    def test_body_extent_skips_comment_and_string_braces(self) -> None:
        bodies = dict(definition_bodies(TAINTED_FILE))
        self.assertIn("return 1;", bodies["func_80000000"])
        self.assertNotIn("Bootstrap", bodies["func_80000000"])

    def test_marker_judged_per_body(self) -> None:
        bodies = dict(definition_bodies(TAINTED_FILE))
        self.assertFalse(is_stub_body(bodies["func_80000000"]))
        self.assertTrue(is_stub_body(bodies["func_80000020"]))

    def test_marker_set_unchanged(self) -> None:
        self.assertEqual(
            STUB_MARKERS,
            ("BOOTSTRAP_RET", "Bootstrap_ReturnVoid", "Bootstrap_ReturnInt"),
        )

    def test_static_inline_header_definition(self) -> None:
        hdr = 'static inline void func_8006F044(void) { Bootstrap_ReturnVoid("func_8006F044", "c"); }\n'
        (name, body), = definition_bodies(hdr)
        self.assertEqual(name, "func_8006F044")
        self.assertTrue(is_stub_body(body))


class Classify(unittest.TestCase):
    leaf = {"name": "func_80000000"}

    def test_real_body_in_tainted_file_is_hand_translated(self) -> None:
        bucket, _, _ = classify(
            self.leaf, None, {"func_80000000": ["a.c"]}, {}, {"func_80000000": False}
        )
        self.assertEqual(bucket, "hand-translated")

    def test_all_bodies_stubbed_is_stub(self) -> None:
        bucket, _, _ = classify(
            self.leaf, None, {"func_80000000": ["a.c"]}, {}, {"func_80000000": True}
        )
        self.assertEqual(bucket, "stub")

    def test_absent_and_provenance_precedence(self) -> None:
        self.assertEqual(classify(self.leaf, None, {}, {}, {})[0], "absent")
        self.assertEqual(
            classify(
                self.leaf, None, {"func_80000000": ["a.c"]},
                {"func_80000000": "func_80000000"}, {"func_80000000": True},
            )[0],
            "ported-from-decomp",
        )


class OverlayCoverageTests(unittest.TestCase):
    """--overlays: leaves keyed by (overlay, name); a plain-named definition
    of a name that several overlays share is never credited."""

    @classmethod
    def setUpClass(cls):
        import pc_port_coverage as pc
        cls.rep = pc.overlay_report()
        cls.rows = {(r["overlay"], r["name"]): r["bucket"] for r in cls.rep["rows"]}

    def test_keys_are_overlay_and_name(self):
        # func_80190D08 exists in ovl_0700 and three rooms: four rows.
        keys = [k for k in self.rows if k[1] == "func_80190D08"]
        self.assertGreaterEqual(len(keys), 4)

    def test_generated_overlay_tu_is_ported_for_its_overlay_only(self):
        self.assertEqual(self.rows[("ovl_0700", "func_80190D08")], "ported-from-decomp")
        self.assertEqual(self.rows[("room_m0123i", "func_80190D08")], "ambiguous")

    def test_ambiguous_names_span_several_overlays(self):
        for name in self.rep["ambiguous_names"]:
            ovs = {k[0] for k in self.rows if k[1] == name}
            self.assertGreater(len(ovs), 1, name)


if __name__ == "__main__":
    unittest.main()
