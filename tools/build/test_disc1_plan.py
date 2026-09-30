#!/usr/bin/env python3
"""Regression tests for YAML-derived Disc 1 build authority."""

from __future__ import annotations

import json
import hashlib
import struct
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from disc1_plan import (  # noqa: E402
    PlanError,
    build_plan,
    matching_status,
    remove_stale_generated_asm_units,
    render_linker_script,
    verification_manifest,
)
from wrapper_matrix import MatrixError, generate_matrix  # noqa: E402


ROOT = Path(__file__).resolve().parents[2]


class Disc1PlanTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.plan = build_plan(root=ROOT)

    def test_geometry_is_complete_and_strict(self) -> None:
        units = self.plan["units"]
        self.assertEqual(units[0]["start"], 0x800)
        self.assertEqual(units[-1]["end"], 0x1EE800)
        self.assertTrue(all(unit["end"] > unit["start"] for unit in units))
        self.assertTrue(
            all(left["end"] == right["start"] for left, right in zip(units, units[1:]))
        )
        self.assertEqual(sum(unit["size"] for unit in units), 0x1EE000)

    def test_c_build_and_verify_mapping_is_bijective(self) -> None:
        c_units = [unit for unit in self.plan["units"] if unit["kind"] == "c"]
        manifest = verification_manifest(self.plan)
        self.assertEqual(len(c_units), self.plan["counts"]["c"])
        self.assertEqual(len(manifest["spans"]), len(c_units))
        for field in ("name", "source", "object"):
            values = [unit[field] for unit in c_units]
            self.assertEqual(len(values), len(set(values)))
        self.assertTrue(all(unit["profile"] for unit in c_units))
        self.assertTrue(all(unit["toolchain"] in {"era", "modern"} for unit in c_units))

    def test_linker_covers_every_object_without_hand_lists(self) -> None:
        linker = render_linker_script(self.plan)
        for unit in self.plan["units"]:
            self.assertEqual(linker.count(unit["object"] + "("), 4)
            self.assertIn(
                f"{unit['object']}({unit['primary_section']})",
                linker,
            )

    def test_status_is_derived_from_plan(self) -> None:
        self.assertIn(
            f"Exact matching-C spans: **{self.plan['counts']['c']}**",
            matching_status(self.plan),
        )

    def test_duplicate_or_zero_length_edge_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "configs/USA").mkdir(parents=True)
            (root / "src").mkdir()
            (root / "src/func_80010000.c").write_text("int func_80010000(void);\n")
            (root / "configs/USA/disc1.yaml").write_text(
                "sha1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b\n"
                "      - [0x800, c, func_80010000]\n"
                "      - [0x800, asm]\n"
                "  - [0x1EE800]\n"
            )
            (root / "configs/USA/disc1_build_profiles.json").write_text(
                json.dumps(
                    {
                        "schema_version": 1,
                        "default_profile": "era",
                        "profiles": {
                            "era": {"toolchain": "era", "flags": ["-O2", "-G0"]}
                        },
                        "assignments": {},
                    }
                )
            )
            with self.assertRaisesRegex(PlanError, "non-increasing edge"):
                build_plan(root=root)

    def test_stale_profile_assignment_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "configs/USA").mkdir(parents=True)
            (root / "src").mkdir()
            (root / "src/func_80010000.c").write_text("int func_80010000(void);\n")
            (root / "configs/USA/disc1.yaml").write_text(
                "sha1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b\n"
                "      - [0x800, c, func_80010000]\n"
                "  - [0x1EE800]\n"
            )
            (root / "configs/USA/disc1_build_profiles.json").write_text(
                json.dumps(
                    {
                        "schema_version": 1,
                        "default_profile": "era",
                        "profiles": {
                            "era": {"toolchain": "era", "flags": ["-O2", "-G0"]},
                            "modern": {"toolchain": "modern", "flags": []},
                        },
                        "assignments": {"modern": ["func_80010008"]},
                    }
                )
            )
            with self.assertRaisesRegex(PlanError, "stale build override"):
                build_plan(root=root)

    def test_split_cleanup_removes_superseded_ignored_asm_unit(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "configs/USA").mkdir(parents=True)
            (root / "src").mkdir()
            (root / "asm/disc1").mkdir(parents=True)
            (root / ".gitignore").write_text("/asm/disc1/\n", encoding="utf-8")
            subprocess.run(["git", "init", "-q", str(root)], check=True)
            (root / "configs/USA/disc1_build_profiles.json").write_text(
                json.dumps(
                    {
                        "schema_version": 1,
                        "default_profile": "era",
                        "profiles": {
                            "era": {"toolchain": "era", "flags": ["-O2", "-G0"]}
                        },
                        "assignments": {},
                    }
                ),
                encoding="utf-8",
            )
            config = root / "configs/USA/disc1.yaml"
            config.write_text(
                "sha1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b\n"
                "      - [0x800, asm]\n"
                "  - [0x1EE800]\n",
                encoding="utf-8",
            )
            old_unit = root / "asm/disc1/800.s"
            old_unit.write_text("/* 800 80010000 00000000 */ nop\n", encoding="utf-8")
            self.assertEqual(
                subprocess.run(
                    ["git", "-C", str(root), "check-ignore", "-q", "--", "asm/disc1/800.s"],
                    check=False,
                ).returncode,
                0,
            )

            (root / "src/func_80010000.c").write_text(
                "int func_80010000(void) { return 0; }\n", encoding="utf-8"
            )
            config.write_text(
                "sha1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b\n"
                "      - [0x800, c, func_80010000]\n"
                "  - [0x1EE800]\n",
                encoding="utf-8",
            )
            plan = build_plan(root=root)
            removed = remove_stale_generated_asm_units(root, plan)

            self.assertEqual(removed, [Path("asm/disc1/800.s")])
            self.assertFalse(old_unit.exists())

    def test_wrapper_matrix_requires_full_exact_word_equality(self) -> None:
        retail = struct.pack("<II", 0x03E00008, 0)
        plan = {
            "expected_sha1": hashlib.sha1(retail).hexdigest(),
            "units": [
                {
                    "kind": "c",
                    "name": "func_80010000",
                    "start": 0,
                    "end": 8,
                    "profile": "era_o2_g0",
                }
            ],
        }
        spec = {
            "schema_version": 1,
            "family": "test wrappers",
            "members": [
                {
                    "name": "func_80010000",
                    "function_hood": "exact-start table reference",
                    "normalized_object_words": ["0x03E00008", "0x00000000"],
                }
            ],
        }
        rendered = generate_matrix(spec, plan, retail, retail)
        self.assertIn("| EXACT |", rendered)
        spec["members"][0]["normalized_object_words"][1] = "0x00000001"
        with self.assertRaisesRegex(MatrixError, "object mismatch at word 1"):
            generate_matrix(spec, plan, retail, retail)


if __name__ == "__main__":
    unittest.main()
