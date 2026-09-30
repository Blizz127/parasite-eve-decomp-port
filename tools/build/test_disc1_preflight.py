#!/usr/bin/env python3
"""Regression tests for the Disc 1 fast preflight validator.

Each test seeds one historical defect class into a minimal synthetic tree and
asserts the validator reports it with an actionable finding. The five classes
are exactly the ones that previously surfaced only after a ~30 minute
split/build cycle (or a real mid-run failure).
"""

from __future__ import annotations

import json
import shutil
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from disc1_preflight import (  # noqa: E402
    check_c_sources,
    check_geometry,
    check_profiles,
    check_yaml_syntax,
    parse_rows,
    run_checks,
)

REPO_ROOT = Path(__file__).resolve().parents[2]

GOOD_YAML = """sha1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
name: preflight fixture
segments:
  - [0x800, c, func_80010000]
  - [0x810, c, func_80010010]
  - [0x820, asm]
  - [0x1EE800]
"""

GOOD_PROFILES = {
    "schema_version": 1,
    "default_profile": "era",
    "profiles": {"era": {"toolchain": "era", "flags": ["-O2", "-G0"]}},
    "assignments": {},
}


def make_tree(root: Path, yaml_text: str = GOOD_YAML, profiles: dict | None = None) -> Path:
    (root / "configs/USA").mkdir(parents=True)
    (root / "src").mkdir()
    (root / "src/func_80010000.c").write_text("int func_80010000(void);\n")
    (root / "src/func_80010010.c").write_text("int func_80010010(void);\n")
    (root / "configs/USA/disc1.yaml").write_text(yaml_text)
    (root / "configs/USA/disc1_build_profiles.json").write_text(
        json.dumps(profiles if profiles is not None else GOOD_PROFILES)
    )
    return root


class PreflightTests(unittest.TestCase):
    def test_valid_synthetic_tree_passes(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = make_tree(Path(temporary))
            rc = run_checks_for(root)
            self.assertEqual(rc, [])

    def test_yaml_syntax_error_is_caught_with_line(self) -> None:
        # A hard-wrapped comment continuation at column 0, the live blocker.
        broken = GOOD_YAML.replace(
            "  - [0x800, c, func_80010000]",
            "  # comment about func_80010000\ncursor by\n"
            "  - [0x800, c, func_80010000]",
        )
        with tempfile.TemporaryDirectory() as temporary:
            root = make_tree(Path(temporary), yaml_text=broken)
            findings = check_yaml_syntax(root / "configs/USA/disc1.yaml")
            self.assertTrue(findings)
            self.assertEqual(findings[0].check, "yaml-syntax")
            self.assertIn("disc1.yaml:", findings[0].message)

    def test_overlap_and_duplicate_vma_is_caught(self) -> None:
        broken = GOOD_YAML.replace(
            "  - [0x810, c, func_80010010]",
            "  - [0x800, asm]",
        )
        with tempfile.TemporaryDirectory() as temporary:
            root = make_tree(Path(temporary), yaml_text=broken)
            rows = parse_rows(root / "configs/USA/disc1.yaml")
            findings = check_geometry(root / "configs/USA/disc1.yaml", rows)
            self.assertTrue(any("duplicate VMA" in f.message for f in findings))
            self.assertTrue(any("does not ascend" in f.message for f in findings))

    def test_gap_is_caught_by_plan_authority(self) -> None:
        # A geometry gap breaks the close-over-image invariant; the fast check
        # must surface a finding rather than silently pass.
        with tempfile.TemporaryDirectory() as temporary:
            root = make_tree(Path(temporary))
            config = root / "configs/USA/disc1.yaml"
            config.write_text(GOOD_YAML.replace("  - [0x1EE800]", "  - [0x1EE7FC]"))
            findings = run_checks_for(root)
            self.assertTrue(findings)

    def test_missing_c_source_is_caught(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = make_tree(Path(temporary))
            (root / "src/func_80010010.c").unlink()
            rows = parse_rows(root / "configs/USA/disc1.yaml")
            findings = check_c_sources(
                root / "configs/USA/disc1.yaml", root, rows
            )
            self.assertTrue(any("no src/func_80010010.c" in f.message for f in findings))

    def test_bad_profile_reference_is_caught(self) -> None:
        profiles = json.loads(json.dumps(GOOD_PROFILES))
        profiles["assignments"] = {"ghost_profile": ["func_80010000"]}
        with tempfile.TemporaryDirectory() as temporary:
            root = make_tree(Path(temporary), profiles=profiles)
            rows = parse_rows(root / "configs/USA/disc1.yaml")
            findings = check_profiles(
                root / "configs/USA/disc1_build_profiles.json", rows
            )
            self.assertTrue(
                any("unknown profile" in f.message for f in findings)
            )

    def test_stale_assignment_is_caught(self) -> None:
        profiles = json.loads(json.dumps(GOOD_PROFILES))
        profiles["assignments"] = {"era": ["func_80010000"]}  # duplicates default
        with tempfile.TemporaryDirectory() as temporary:
            root = make_tree(Path(temporary), profiles=profiles)
            rows = parse_rows(root / "configs/USA/disc1.yaml")
            findings = check_profiles(
                root / "configs/USA/disc1_build_profiles.json", rows
            )
            self.assertTrue(any("duplicates the default" in f.message for f in findings))

    def test_era_cc1_version_is_selected_from_profile_environment(self) -> None:
        # ERA_CC1_VER mirrors ERA_ASPSX_VER: absent -> the hard-coded 2.7.2
        # pair (byte-identical default path); 2.8.1 -> the sibling directory.
        import disc1_build  # noqa: PLC0415

        self.assertEqual(
            disc1_build.era_compiler({}),
            (disc1_build.ERA_CPP, disc1_build.ERA_CC1),
        )
        self.assertEqual(
            disc1_build.era_compiler({"ERA_CC1_VER": "2.7.2", "ERA_ASPSX_VER": "2.30"}),
            (disc1_build.ERA_CPP, disc1_build.ERA_CC1),
        )
        cpp, cc1 = disc1_build.era_compiler({"ERA_CC1_VER": "2.8.1"})
        self.assertEqual(cpp, Path("tools/era/gcc-2.8.1-psx/cpp"))
        self.assertEqual(cc1, Path("tools/era/gcc-2.8.1-psx/cc1"))

    @unittest.skipUnless(
        (REPO_ROOT / "tools/era/gcc-2.7.2-psx/cc1").is_file()
        and (REPO_ROOT / "tools/mipsel-host/bin/mipsel-linux-gnu-as").is_file(),
        "era + mipsel toolchain not present",
    )
    def test_deep_size_check_catches_oversized_span(self) -> None:
        # A C leaf whose declared span is larger than what the era toolchain
        # emits is the historical 0x94-vs-0x90 class.
        yaml_text = GOOD_YAML.replace(
            "  - [0x810, c, func_80010010]",
            "  - [0x80C, c, func_80010010]",
        )
        with tempfile.TemporaryDirectory() as temporary:
            root = make_tree(Path(temporary), yaml_text=yaml_text)
            (root / "src/func_80010000.c").write_text(
                "int func_80010000(void) { return 0; }\n"
            )
            (root / "src/func_80010010.c").write_text(
                "int func_80010010(void) { return 0; }\n"
            )
            rc = run_checks_for(root, deep=True, only=["func_80010000"])
            self.assertTrue(
                any(f.check == "deep-size" for f in rc),
                msg=f"expected a deep-size finding, got {[f.check for f in rc]}",
            )


def run_checks_for(root: Path, deep: bool = False, only: list[str] | None = None):
    import argparse

    args = argparse.Namespace(
        root=root,
        config=Path("configs/USA/disc1.yaml"),
        profiles=Path("configs/USA/disc1_build_profiles.json"),
        deep=deep,
        only=only or [],
        jobs=4,
    )
    findings, _summary = run_checks(args)
    return findings


if __name__ == "__main__":
    unittest.main()
