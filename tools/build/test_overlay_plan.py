#!/usr/bin/env python3
"""Regression tests for the rebuild-target parametrization (overlay lane).

Everything here runs without game data: synthetic trees carry a fake overlay
blob whose SHA-1 is computed on the fly.  Two invariants are pinned:

  * the EXE target is byte-for-byte the historical behaviour (plan hash,
    linker script, generated manifest do not depend on the target machinery);
  * an overlay target resolves geometry, VRAM naming, paths, header-less
    linker script and inherited build profiles from the manifest alone.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from disc1_plan import (  # noqa: E402
    PlanError,
    build_plan,
    plan_target,
    render_linker_script,
    verification_manifest,
)
from disc1_preflight import run_checks  # noqa: E402
from overlay_targets import (  # noqa: E402
    MAIN_TARGET,
    TargetError,
    configured_overlay_ids,
    load_manifest,
    load_target,
    parse_simple_yaml,
)

ROOT = Path(__file__).resolve().parents[2]
FAKE_BLOB = bytes(range(256)) * 16  # 0x1000 bytes
FAKE_SHA1 = hashlib.sha1(FAKE_BLOB).hexdigest()

EXE_PROFILES = {
    "schema_version": 1,
    "default_profile": "era_o2_g0",
    "profiles": {
        "era_o2_g0": {"toolchain": "era", "flags": ["-O2", "-G0"]},
        "era_o1_g0": {"toolchain": "era", "flags": ["-O1", "-G0"]},
    },
    "assignments": {},
}


def manifest_text(with_config: bool = True, sha1: str = FAKE_SHA1) -> str:
    config_lines = (
        "    config: configs/USA/overlays/ovl_test.yaml\n"
        "    build_profiles: configs/USA/overlays/ovl_test_build_profiles.json\n"
        if with_config
        else ""
    )
    return (
        "# synthetic manifest\n"
        "schema_version: 1\n"
        "image:\n"
        "  name: PE.IMG\n"
        "  extracted: build/extracted/disc1/PE.IMG\n"
        "  size: 4096\n"
        "  sectors: 2\n"
        "  sha1: 146c0ce7308bf9fdc2ba5a84230e198db0663f3b\n"
        "  disc_lba: 1013\n"
        "exe:\n"
        "  extracted: build/extracted/disc1/SLUS_006.62\n"
        "  sha1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b\n"
        "  range_table: 0x80093150\n"
        "  range_table_entries: 21\n"
        "  dest_table: 0x8001160C\n"
        "  dest_table_entries: 4\n"
        "  room_table: 0x80093378\n"
        "  room_table_entries: 438\n"
        "  room_base_lba: 1013\n"
        "rooms:\n"
        "  chunk2_vram: 0x8018EFE8\n"
        "  extract_dir: build/extracted/disc1/peimg\n"
        "overlays:\n"
        "  ovl_test:\n"
        "    kind: subsystem\n"
        "    description: \"synthetic overlay # not a comment\"\n"
        "    range_index: 0\n"
        "    sector_start: 0x0\n"
        "    sector_end: 0x2\n"
        "    size: 0x1000\n"
        "    dest_index: 0\n"
        "    vram: 0x80200000\n"
        f"    sha1: {sha1}\n"
        "    blob: build/extracted/disc1/peimg/ovl_test.bin\n"
        "    text_start: 0x10\n"
        "    text_end: 0xF00\n"
        + config_lines
    )


OVL_YAML = f"""name: synthetic overlay
sha1: {FAKE_SHA1}
options:
  platform: psx
segments:
  - name: main
    type: code
    start: 0x0
    vram: 0x80200000
    subsegments:
      - [0x0, bin]
      - [0x10, asm]
      - [0x40, c, func_80200040]
      - [0x60, asm]
      - [0xF00, bin]
  - [0x1000]
"""


def make_overlay_tree(root: Path, *, yaml_text: str = OVL_YAML, with_config: bool = True) -> Path:
    (root / "configs/USA/overlays").mkdir(parents=True)
    (root / "src/overlays/ovl_test").mkdir(parents=True)
    (root / "build/extracted/disc1/peimg").mkdir(parents=True)
    (root / "configs/USA/overlays/manifest.yaml").write_text(manifest_text(with_config))
    (root / "configs/USA/overlays/ovl_test.yaml").write_text(yaml_text)
    (root / "configs/USA/disc1_build_profiles.json").write_text(json.dumps(EXE_PROFILES))
    (root / "configs/USA/overlays/ovl_test_build_profiles.json").write_text(
        json.dumps(
            {
                "schema_version": 1,
                "profiles_from": "configs/USA/disc1_build_profiles.json",
                "assignments": {"era_o1_g0": ["func_80200040"]},
            }
        )
    )
    (root / "src/overlays/ovl_test/func_80200040.c").write_text("void func_80200040(void) {}\n")
    (root / "build/extracted/disc1/peimg/ovl_test.bin").write_bytes(FAKE_BLOB)
    return root


class ManifestReaderTests(unittest.TestCase):
    def test_subset_reader_parses_nested_mappings_scalars_and_comments(self) -> None:
        data = parse_simple_yaml(manifest_text())
        self.assertEqual(data["schema_version"], 1)
        self.assertEqual(data["overlays"]["ovl_test"]["vram"], 0x80200000)
        self.assertEqual(data["overlays"]["ovl_test"]["size"], 0x1000)
        self.assertEqual(data["overlays"]["ovl_test"]["description"], "synthetic overlay # not a comment")
        self.assertEqual(data["rooms"]["extract_dir"], "build/extracted/disc1/peimg")

    def test_subset_reader_rejects_lists_and_bad_indentation(self) -> None:
        with self.assertRaisesRegex(TargetError, "unsupported YAML line"):
            parse_simple_yaml("overlays:\n  - a\n")
        with self.assertRaisesRegex(TargetError, "inconsistent indentation"):
            parse_simple_yaml("a:\n  b: 1\n   c: 2\n")
        with self.assertRaisesRegex(TargetError, "duplicate key"):
            parse_simple_yaml("a: 1\na: 2\n")

    def test_committed_manifest_agrees_with_pyyaml_when_available(self) -> None:
        text = (ROOT / "configs/USA/overlays/manifest.yaml").read_text(encoding="utf-8")
        ours = parse_simple_yaml(text)
        try:
            import yaml  # noqa: PLC0415
        except ImportError:
            self.skipTest("PyYAML not installed")
        self.assertEqual(ours, yaml.safe_load(text))

    def test_committed_manifest_loads_and_names_ovl_0700(self) -> None:
        data = load_manifest(ROOT)
        self.assertIn("ovl_0700", data["overlays"])
        self.assertIn("ovl_0700", configured_overlay_ids(ROOT))
        target = load_target("ovl_0700", root=ROOT)
        self.assertEqual(target.vram, 0x8018EFF0)
        self.assertEqual(target.load_size, 0x5B800)
        self.assertFalse(target.has_header)
        self.assertEqual(target.src_dir, "src/overlays/ovl_0700")
        entry = data["overlays"]["ovl_0700"]
        self.assertEqual(entry["text_start"], 0x6C)
        self.assertEqual(entry["text_end"], 0xCFD4)

    def test_unknown_or_unconfigured_target_is_rejected(self) -> None:
        with self.assertRaisesRegex(TargetError, "unknown target"):
            load_target("ovl_nope", root=ROOT)
        with tempfile.TemporaryDirectory() as temporary:
            root = make_overlay_tree(Path(temporary), with_config=False)
            with self.assertRaisesRegex(TargetError, "no splat config yet"):
                load_target("ovl_test", root=root)

    def test_default_target_is_the_exe(self) -> None:
        self.assertIs(load_target(None, root=ROOT), MAIN_TARGET)
        self.assertIs(load_target("disc1", root=ROOT), MAIN_TARGET)
        self.assertEqual(MAIN_TARGET.file_start, 0x800)
        self.assertEqual(MAIN_TARGET.file_end, 0x1EE800)
        self.assertEqual(MAIN_TARGET.vram, 0x80010000)
        self.assertEqual(MAIN_TARGET.header_source, "asm/disc1/header.s")


class OverlayPlanTests(unittest.TestCase):
    def test_overlay_plan_geometry_paths_and_inherited_profiles(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = make_overlay_tree(Path(temporary))
            target = load_target("ovl_test", root=root)
            plan = build_plan(root=root, target=target)
            units = plan["units"]
            self.assertEqual([u["kind"] for u in units], ["bin", "asm", "c", "asm", "bin"])
            self.assertEqual(units[0]["start"], 0)
            self.assertEqual(units[-1]["end"], 0x1000)
            self.assertEqual(sum(u["size"] for u in units), 0x1000)
            self.assertEqual(units[2]["vram"], 0x80200040)
            self.assertEqual(units[2]["source"], "src/overlays/ovl_test/func_80200040.c")
            self.assertEqual(units[2]["object"], "build/src/overlays/ovl_test/func_80200040.c.o")
            self.assertEqual(units[2]["profile"], "era_o1_g0")
            self.assertEqual(units[2]["flags"], ["-O1", "-G0"])
            self.assertEqual(units[0]["source"], "assets/overlays/ovl_test/0.bin")
            self.assertEqual(units[0]["primary_section"], ".data")
            self.assertEqual(units[1]["source"], "asm/overlays/ovl_test/10.s")
            self.assertIsNone(plan["header"])
            self.assertEqual(plan["file"], {
                "header_size": 0, "load_start": 0, "load_end": 0x1000,
                "load_size": 0x1000, "load_vram": 0x80200000,
            })
            self.assertEqual(plan["counts"]["bin"], 2)
            self.assertEqual(plan["expected_sha1"], FAKE_SHA1)
            self.assertEqual(plan_target(plan).id, "ovl_test")
            linker = render_linker_script(plan)
            self.assertNotIn(".header", linker)
            self.assertIn("    .main 0x80200000 : AT(0x0)", linker)
            self.assertIn("from configs/USA/overlays/ovl_test.yaml.", linker)
            manifest = verification_manifest(plan)
            self.assertEqual(manifest["matching_c_count"], 1)

    def test_overlay_plan_hash_ignores_the_target_descriptor(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = make_overlay_tree(Path(temporary))
            plan = build_plan(root=root, target=load_target("ovl_test", root=root))
            stripped = {k: v for k, v in plan.items() if k not in ("target", "plan_sha256")}
            canonical = json.dumps(stripped, sort_keys=True, separators=(",", ":")).encode()
            self.assertEqual(plan["plan_sha256"], hashlib.sha256(canonical).hexdigest())

    def test_overlay_vram_naming_and_manifest_sha_are_enforced(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = make_overlay_tree(
                Path(temporary),
                yaml_text=OVL_YAML.replace("func_80200040", "func_80200044"),
            )
            (root / "src/overlays/ovl_test/func_80200044.c").write_text("void f(void) {}\n")
            profiles = root / "configs/USA/overlays/ovl_test_build_profiles.json"
            data = json.loads(profiles.read_text())
            data["assignments"] = {}
            profiles.write_text(json.dumps(data))
            with self.assertRaisesRegex(PlanError, "names 0x80200044"):
                build_plan(root=root, target=load_target("ovl_test", root=root))
        with tempfile.TemporaryDirectory() as temporary:
            root = make_overlay_tree(Path(temporary))
            (root / "configs/USA/overlays/ovl_test.yaml").write_text(
                OVL_YAML.replace(FAKE_SHA1, "0" * 40)
            )
            with self.assertRaisesRegex(PlanError, "!= manifest sha1"):
                build_plan(root=root, target=load_target("ovl_test", root=root))

    def test_overlay_geometry_must_close_over_the_blob(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = make_overlay_tree(
                Path(temporary), yaml_text=OVL_YAML.replace("  - [0x1000]", "  - [0xFFC]")
            )
            with self.assertRaisesRegex(PlanError, "final row must be"):
                build_plan(root=root, target=load_target("ovl_test", root=root))

    def test_profiles_from_cannot_coexist_with_profiles(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = make_overlay_tree(Path(temporary))
            path = root / "configs/USA/overlays/ovl_test_build_profiles.json"
            data = json.loads(path.read_text())
            data["profiles"] = EXE_PROFILES["profiles"]
            path.write_text(json.dumps(data))
            with self.assertRaisesRegex(PlanError, "not both"):
                build_plan(root=root, target=load_target("ovl_test", root=root))


    def test_local_profiles_extend_inherited_definitions(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = make_overlay_tree(Path(temporary))
            path = root / "configs/USA/overlays/ovl_test_build_profiles.json"
            data = json.loads(path.read_text())
            data["local_profiles"] = {
                "ovl_test_dispatch": {
                    "toolchain": "era",
                    "flags": ["-O2", "-G0"],
                    "environment": {"MASPSX_DISPATCH_FOLD": "jtbl_80200000"},
                }
            }
            data["assignments"] = {"ovl_test_dispatch": ["func_80200040"]}
            path.write_text(json.dumps(data))
            plan = build_plan(root=root, target=load_target("ovl_test", root=root))
            unit = next(u for u in plan["units"] if u.get("name") == "func_80200040")
            self.assertEqual(unit["environment"]["MASPSX_DISPATCH_FOLD"], "jtbl_80200000")

    def test_local_profiles_may_not_shadow_inherited_names(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = make_overlay_tree(Path(temporary))
            path = root / "configs/USA/overlays/ovl_test_build_profiles.json"
            data = json.loads(path.read_text())
            data["local_profiles"] = {"era_o1_g0": {"toolchain": "era", "flags": []}}
            path.write_text(json.dumps(data))
            with self.assertRaisesRegex(PlanError, "shadow inherited"):
                build_plan(root=root, target=load_target("ovl_test", root=root))


    def test_local_profiles_must_be_an_object(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = make_overlay_tree(Path(temporary))
            path = root / "configs/USA/overlays/ovl_test_build_profiles.json"
            data = json.loads(path.read_text())
            data["local_profiles"] = ["not", "an", "object"]
            path.write_text(json.dumps(data))
            with self.assertRaisesRegex(PlanError, "local_profiles must be an object"):
                build_plan(root=root, target=load_target("ovl_test", root=root))


class OverlayPreflightTests(unittest.TestCase):
    def run_preflight(self, root: Path, target: str | None):
        args = argparse.Namespace(
            root=root,
            target=target,
            config=None,
            profiles=None,
            deep=False,
            only=[],
            jobs=2,
        )
        findings, summary = run_checks(args)
        return findings, summary

    def _write_local(self, root: Path, local) -> None:
        path = root / "configs/USA/overlays/ovl_test_build_profiles.json"
        data = json.loads(path.read_text())
        data["local_profiles"] = local
        path.write_text(json.dumps(data))

    def test_preflight_flags_local_profile_shadowing(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = make_overlay_tree(Path(temporary))
            self._write_local(root, {"era_o1_g0": {"toolchain": "era", "flags": []}})
            findings, _ = self.run_preflight(root, "ovl_test")
            self.assertTrue(any(f.check == "profile-ref" and "shadow inherited" in f.message for f in findings))

    def test_preflight_flags_non_object_local_profiles(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = make_overlay_tree(Path(temporary))
            self._write_local(root, "era_o2_g0")
            findings, _ = self.run_preflight(root, "ovl_test")
            self.assertTrue(any(f.check == "profile-ref" and "must be an object" in f.message for f in findings))

    def test_preflight_accepts_valid_local_profile(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = make_overlay_tree(Path(temporary))
            self._write_local(root, {"ovl_test_x": {"toolchain": "era", "flags": ["-O2", "-G0"]}})
            findings, _ = self.run_preflight(root, "ovl_test")
            self.assertEqual([], [f for f in findings if f.check == "profile-ref"])

    def test_overlay_fast_preflight_passes_on_a_valid_tree(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = make_overlay_tree(Path(temporary))
            findings, summary = self.run_preflight(root, "ovl_test")
            self.assertEqual([f.message for f in findings], [])
            self.assertEqual(summary["counts"]["c"], 1)

    def test_overlay_preflight_reports_its_own_geometry_and_sources(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = make_overlay_tree(Path(temporary))
            (root / "src/overlays/ovl_test/func_80200040.c").unlink()
            findings, _ = self.run_preflight(root, "ovl_test")
            self.assertTrue(
                any("no src/overlays/ovl_test/func_80200040.c" in f.message for f in findings),
                [f.message for f in findings],
            )
        with tempfile.TemporaryDirectory() as temporary:
            root = make_overlay_tree(
                Path(temporary), yaml_text=OVL_YAML.replace("  - [0x1000]", "  - [0x1004]")
            )
            findings, _ = self.run_preflight(root, "ovl_test")
            self.assertTrue(any("final row must be" in f.message for f in findings))

    def test_unknown_target_is_a_finding_not_a_crash(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = make_overlay_tree(Path(temporary))
            findings, _ = self.run_preflight(root, "ovl_missing")
            self.assertEqual(findings[0].check, "target")


class MainTargetInvarianceTests(unittest.TestCase):
    """The EXE path must not depend on the overlay machinery at all."""

    def test_exe_plan_carries_the_historical_geometry_and_paths(self) -> None:
        plan = build_plan(root=ROOT)
        self.assertEqual(plan["file"], {
            "header_size": 0x800, "load_start": 0x800, "load_end": 0x1EE800,
            "load_size": 0x1EE000, "load_vram": 0x80010000,
        })
        self.assertEqual(plan["header"], {
            "source": "asm/disc1/header.s",
            "object": "build/asm/disc1/header.s.o",
            "section": ".data",
            "size": 0x800,
        })
        self.assertNotIn("bin", plan["counts"])
        self.assertEqual(plan["authority"], "configs/USA/disc1.yaml")
        self.assertTrue(all(u["source"].startswith(("src/", "asm/disc1/")) for u in plan["units"]))
        # The hash is computed before the descriptor is attached.
        stripped = {k: v for k, v in plan.items() if k not in ("target", "plan_sha256")}
        canonical = json.dumps(stripped, sort_keys=True, separators=(",", ":")).encode()
        self.assertEqual(plan["plan_sha256"], hashlib.sha256(canonical).hexdigest())
        linker = render_linker_script(plan)
        self.assertTrue(linker.startswith(
            "/* Generated by tools/build/disc1_plan.py from configs/USA/disc1.yaml.\n"
            " * Do not edit: YAML owns order and span geometry. */\n"
            "SECTIONS\n{\n    .header : AT(0)\n    {\n"
            "        build/asm/disc1/header.s.o(.data)\n    }\n\n"
            "    .main 0x80010000 : AT(0x800)\n    {\n"
        ))

    def test_exe_preflight_namespace_without_target_still_works(self) -> None:
        # Older callers build a Namespace without `target`; it must mean the EXE.
        args = argparse.Namespace(
            root=ROOT,
            config=Path("configs/USA/disc1.yaml"),
            profiles=Path("configs/USA/disc1_build_profiles.json"),
            deep=False,
            only=[],
            jobs=2,
        )
        findings, summary = run_checks(args)
        self.assertEqual([f.message for f in findings], [])
        self.assertEqual(summary["config"], str(ROOT / "configs/USA/disc1.yaml"))


if __name__ == "__main__":
    os.environ.pop("PYTHONWARNINGS", None)
    unittest.main()
