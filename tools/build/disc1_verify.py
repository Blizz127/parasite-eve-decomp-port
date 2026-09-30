#!/usr/bin/env python3
"""Verify Disc 1 authority, artifacts, and exact matching status."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Any

from disc1_plan import (
    DEFAULT_STATUS,
    PlanError,
    build_plan,
    matching_status,
    plan_target,
    verification_manifest,
    write_generated,
)
from overlay_targets import MAIN_TARGET, TargetError, load_target


ROOT = Path(__file__).resolve().parents[2]
EXE = Path(MAIN_TARGET.retail)
CANDIDATE = Path(MAIN_TARGET.candidate)


class VerifyError(RuntimeError):
    """A verification gate failed."""


def gate(number: int, total: int, label: str) -> None:
    print(f"[{number}/{total}] {label}")


def pass_line(message: str) -> None:
    print(f"  PASS {message}")


def sha1(path: Path) -> str:
    return hashlib.sha1(path.read_bytes()).hexdigest()


def check_status(plan: dict[str, Any]) -> None:
    path = ROOT / DEFAULT_STATUS
    if not path.is_file():
        raise VerifyError(
            f"missing generated status {path.relative_to(ROOT)}; "
            "run python3 tools/build/disc1_plan.py --write-status"
        )
    expected = matching_status(plan)
    if path.read_text(encoding="utf-8") != expected:
        raise VerifyError(
            f"stale generated status {path.relative_to(ROOT)}; "
            "run python3 tools/build/disc1_plan.py --write-status"
        )


def tracked_sources(plan: dict[str, Any]) -> None:
    result = subprocess.run(
        ["git", "ls-files", "-z"],
        cwd=ROOT,
        stdout=subprocess.PIPE,
        check=True,
    )
    tracked = set(result.stdout.decode().split("\0"))
    missing = [
        unit["source"]
        for unit in plan["units"]
        if unit["kind"] == "c" and unit["source"] not in tracked
    ]
    if missing:
        raise VerifyError("YAML C sources are not tracked: " + ", ".join(missing))
    if not plan_target(plan).is_main:
        # The accepted-residual / nonmatching-disposition ledgers are EXE-only
        # (they enumerate src/func_*.c); an overlay's namespace is its own
        # src/overlays/<id>/ directory and every tracked file there must be a
        # YAML C span.
        src_dir = plan_target(plan).src_dir
        extra = sorted(
            path for path in tracked
            if re.fullmatch(rf"{re.escape(src_dir)}/func_[0-9A-Fa-f]{{8}}\.c", path)
            and path not in {u["source"] for u in plan["units"] if u["kind"] == "c"}
        )
        if extra:
            raise VerifyError(
                "tracked overlay sources without a YAML C span: " + ", ".join(extra)
            )
        return

    yaml_sources = {
        unit["source"] for unit in plan["units"] if unit["kind"] == "c"
    }
    tracked_function_sources = {
        path
        for path in tracked
        if re.fullmatch(r"src/func_[0-9A-Fa-f]{8}\.c", path)
    }
    residual_policy_path = ROOT / "docs/acceptance/MATCHING_RESIDUAL_POLICY.md"
    residual_policy = residual_policy_path.read_text(encoding="utf-8")
    residual_names = set(re.findall(r"func_[0-9A-Fa-f]{8}", residual_policy))
    count_match = re.search(
        r"There are also \*\*(\d+) `ACCEPTED-RESIDUAL` leaves\*\*",
        residual_policy,
    )
    if not count_match or int(count_match.group(1)) != len(residual_names):
        raise VerifyError(
            "accepted-residual published count does not equal its function inventory"
        )
    parked = (ROOT / "docs/ai_context/parked_blockers.json").read_text(
        encoding="utf-8"
    )
    missing_park_records = sorted(name for name in residual_names if name not in parked)
    if missing_park_records:
        raise VerifyError(
            "accepted residuals missing parked records: " + ", ".join(missing_park_records)
        )
    disposition_path = ROOT / "configs/USA/disc1_nonmatching_sources.json"
    try:
        dispositions = json.loads(disposition_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise VerifyError(f"cannot load nonmatching source dispositions: {exc}") from exc
    if dispositions.get("schema_version") != 1 or not isinstance(
        dispositions.get("sources"), dict
    ):
        raise VerifyError("nonmatching source dispositions have invalid schema")
    declared = dispositions["sources"]
    extra_sources = tracked_function_sources - yaml_sources
    if set(declared) != extra_sources:
        missing_declarations = sorted(extra_sources - set(declared))
        stale_declarations = sorted(set(declared) - extra_sources)
        raise VerifyError(
            "nonmatching source disposition bijection failed: "
            f"undeclared={missing_declarations}, stale={stale_declarations}"
        )
    allowed_dispositions = {
        "accepted-residual",
        "nonmatching-native-cut",
        "rejected-tail-draft",
        "in-progress-checkpoint",
    }
    for source, record in declared.items():
        if not isinstance(record, dict) or record.get("disposition") not in allowed_dispositions:
            raise VerifyError(f"invalid nonmatching disposition for {source}")
        evidence = record.get("evidence")
        if not isinstance(evidence, str) or not (ROOT / evidence).is_file():
            raise VerifyError(f"missing nonmatching evidence for {source}")
        is_residual = Path(source).stem in residual_names
        if is_residual != (record["disposition"] == "accepted-residual"):
            raise VerifyError(f"residual policy/disposition disagreement for {source}")
    file_scope_assembly = []
    for source in sorted(tracked_function_sources):
        text = (ROOT / source).read_text(encoding="utf-8")
        # file-scope only: an in-function tie/barrier is indented; a real
        # top-level asm statement starts in column 0.
        if re.search(r"(?m)^__asm__\s*\(", text):
            file_scope_assembly.append(source)
    if file_scope_assembly:
        raise VerifyError(
            "file-scope assembly cannot be matching/residual C: "
            + ", ".join(file_scope_assembly)
        )


def public_checks(plan: dict[str, Any]) -> None:
    manifest = verification_manifest(plan)
    spans = manifest["spans"]
    names = [span["name"] for span in spans]
    sources = [span["source"] for span in spans]
    objects = [span["object"] for span in spans]
    if not (len(names) == len(set(names)) == plan["counts"]["c"]):
        raise VerifyError("C symbol mapping is not bijective")
    if len(sources) != len(set(sources)) or len(objects) != len(set(objects)):
        raise VerifyError("C source/object mapping is not bijective")
    if any(source.replace("src/", "build/src/") + ".o" != obj for source, obj in zip(sources, objects)):
        raise VerifyError("C source/object derivation invariant failed")

    build_wrapper = (ROOT / "scripts/build_us.sh").read_text(encoding="utf-8")
    verify_wrapper = (ROOT / "scripts/verify_us.sh").read_text(encoding="utf-8")
    if "func_" in build_wrapper or "func_" in verify_wrapper:
        raise VerifyError("per-leaf symbol leaked back into a build/verify wrapper")


def local_generated_checks(plan: dict[str, Any]) -> None:
    header = plan.get("header")
    expected_sources = [
        *([header["source"]] if header else []),
        *(u["source"] for u in plan["units"]),
    ]
    missing = [source for source in expected_sources if not (ROOT / source).is_file()]
    if missing:
        raise VerifyError(
            "missing split source(s): "
            + ", ".join(missing[:12])
            + (f" (+{len(missing)-12} more)" if len(missing) > 12 else "")
        )
    check_ignore = subprocess.run(
        ["git", "check-ignore", "--stdin"],
        cwd=ROOT,
        input=("\n".join(expected_sources) + "\n").encode(),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    ignored = set(check_ignore.stdout.decode().splitlines())
    generated = [source for source in expected_sources if not source.startswith("src/")]
    not_ignored = [source for source in generated if source not in ignored]
    if not_ignored:
        raise VerifyError("split artifacts are not git-ignored: " + ", ".join(not_ignored))


def exact_checks(plan: dict[str, Any]) -> None:
    target = plan_target(plan)
    exe = ROOT / target.retail
    candidate = ROOT / target.candidate
    if not exe.is_file():
        raise VerifyError(f"missing retail {'executable' if target.is_main else 'overlay'} {target.retail}")
    if not candidate.is_file():
        build_cmd = "scripts/build_us.sh" if target.is_main else f"tools/build/disc1_build.py --target {target.id}"
        raise VerifyError(
            f"missing candidate {target.candidate}; run {build_cmd} first"
        )
    original_hash = sha1(exe)
    candidate_hash = sha1(candidate)
    if original_hash != plan["expected_sha1"]:
        raise VerifyError(
            f"retail SHA-1 {original_hash} != config {plan['expected_sha1']}"
        )
    if candidate_hash != plan["expected_sha1"]:
        raise VerifyError(
            f"candidate SHA-1 {candidate_hash} != retail {plan['expected_sha1']}"
        )
    original = exe.read_bytes()
    rebuilt = candidate.read_bytes()
    if rebuilt != original:
        raise VerifyError("candidate hash matched but bytes differ (impossible collision gate)")
    for unit in plan["units"]:
        if unit["kind"] != "c":
            continue
        if rebuilt[unit["start"] : unit["end"]] != original[unit["start"] : unit["end"]]:
            raise VerifyError(f"packed C span differs: {unit['name']}")
    pass_line(f"retail SHA-1 {original_hash}")
    pass_line(f"candidate SHA-1 {candidate_hash}")
    pass_line(f"all {plan['counts']['c']} packed C spans equal retail")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--public",
        action="store_true",
        help="artifact-independent authority gate (no retail image or split required)",
    )
    parser.add_argument(
        "--target",
        default=None,
        metavar="ID",
        help="verify target: disc1 (default, the EXE) or an overlay id from "
        "configs/USA/overlays/manifest.yaml",
    )
    args = parser.parse_args(argv)
    total = 4 if args.public else 7
    try:
        target = load_target(args.target, root=ROOT)
        gate(1, total, "YAML span geometry and build-profile model")
        plan = build_plan(root=ROOT, require_generated=not args.public, target=target)
        pass_line(
            f"{plan['counts']['units']} spans = {plan['counts']['c']} c + "
            f"{plan['counts']['asm']} asm + {plan['counts']['rodata']} rodata"
            + (f" + {plan['counts']['bin']} bin" if plan["counts"].get("bin") else "")
        )
        pass_line(f"load geometry closes at 0x{plan['file']['load_size']:X}")

        gate(2, total, "Generated build/verification bijection")
        public_checks(plan)
        pass_line(f"{plan['counts']['c']} YAML C spans -> source -> object -> verify span")

        gate(3, total, "Tracked C sources and generated published status")
        tracked_sources(plan)
        pass_line("every YAML C source is tracked")
        if target.is_main:
            check_status(plan)
            pass_line("every extra tracked function C has a nonmatching disposition")
            pass_line("published matching count equals YAML")
        else:
            pass_line(f"every tracked {target.src_dir}/func_*.c is a YAML C span")

        gate(4, total, "Deterministic generated plans")
        # Verification is read-only with respect to the build tree. Emit into
        # a private temporary directory so a Docker-created build owned by a
        # different UID cannot make this gate fail (or tempt a chmod/chown).
        with tempfile.TemporaryDirectory(prefix="pe-disc1-verify-") as temporary:
            output = Path(temporary)
            write_generated(plan, output)
            generated_manifest = json.loads(
                (output / "disc1_verify_manifest.json").read_text(encoding="utf-8")
            )
        if generated_manifest != verification_manifest(plan):
            raise VerifyError("generated verifier manifest is not deterministic")
        pass_line(f"plan SHA-256 {plan['plan_sha256']}")
        if args.public:
            print("\nPUBLIC_VERIFY=PASS" if target.is_main else f"\nPUBLIC_VERIFY[{target.id}]=PASS")
            print(f"matching-C count: {plan['counts']['c']} (from YAML)")
            return 0

        gate(5, total, "Split-generated source coverage")
        local_generated_checks(plan)
        pass_line("all YAML-derived split sources exist and are ignored")

        gate(6, total, "Split prerequisite gate")
        split_command = (
            [str(ROOT / "scripts/split_us.sh"), "--check"]
            if target.is_main
            else [str(ROOT / "scripts/split_overlay.sh"), target.id, "--check"]
        )
        split = subprocess.run(
            split_command,
            cwd=ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=False,
        )
        split_label = " ".join(Path(part).name if index == 0 else part for index, part in enumerate(split_command))
        if split.returncode:
            raise VerifyError(
                f"scripts/{split_label} failed:\n"
                + split.stdout.decode(errors="replace")
            )
        pass_line(f"scripts/{split_label}")

        gate(7, total, "Exact candidate and packed-span verification")
        exact_checks(plan)
    except (PlanError, TargetError, VerifyError, subprocess.CalledProcessError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1

    print("\nVERIFY_US=PASS" if target.is_main else f"\nVERIFY_TARGET[{target.id}]=PASS")
    print(f"matching-C count: {plan['counts']['c']} (from YAML)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
