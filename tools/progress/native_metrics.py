#!/usr/bin/env python3
"""Derive native-port progress metrics from code, CMake, tests, and evidence."""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
SOURCE = Path("pc_port/game/boot/func_80030894_port.c")
EVIDENCE = Path("docs/evidence/pe-b54kb6-30894-l9/REPORT.md")
CMAKE = Path("pc_port/CMakeLists.txt")
TEST_SOURCE = Path("pc_port/tests/test_native.c")
STATUS = Path("docs/generated/NATIVE_PORT_STATUS.md")


class MetricsError(RuntimeError):
    """A native metric cannot be derived or cross-checked."""


def one(pattern: str, text: str, label: str, flags: int = 0) -> re.Match[str]:
    matches = list(re.finditer(pattern, text, flags))
    if len(matches) != 1:
        raise MetricsError(f"expected one {label}, found {len(matches)}")
    return matches[0]


def cmake_sources(text: str, variable: str) -> list[str]:
    block = one(
        rf"set\({re.escape(variable)}\s+(.*?)\n\)",
        text,
        variable,
        re.DOTALL,
    ).group(1)
    sources: list[str] = []
    for line in block.splitlines():
        line = line.split("#", 1)[0].strip()
        if not line or line.startswith("${"):
            continue
        sources.extend(part for part in line.split() if part.endswith(".c"))
    return sources


def optional_cmake_sources(text: str, variable: str) -> list[str]:
    """Return a CMake source list when the project still declares one."""
    if not re.search(rf"set\({re.escape(variable)}\s+", text):
        return []
    return cmake_sources(text, variable)


def collect_test_translation_unit(root: Path, text: str, rel: Path) -> str:
    """Concatenate a C translation unit with its local quoted #includes.

    The native suite's cases live in `test_native.c` and the `test_*.h`
    headers it pulls in, so a static count that only reads the main file
    under-reports the expected run count.  Includes are resolved relative to
    the including file; system includes (`<...>`) and missing files are left
    alone.  Each file contributes once, which matches a single TU compile.
    """
    parts: list[str] = []

    def walk(current: Path) -> None:
        if current in seen:
            return
        seen.add(current)
        body = current.read_text(encoding="utf-8")
        parts.append(body)
        for name in re.findall(r'#\s*include\s+"([^"]+)"', body):
            candidate = (current.parent / name).resolve()
            if candidate.is_file() and candidate not in seen:
                walk(candidate)

    seen: set[Path] = set()
    walk((root / rel).resolve())
    return "\n".join(parts)


def derive_metrics(root: Path = ROOT) -> dict[str, Any]:
    source = (root / SOURCE).read_text(encoding="utf-8")
    evidence = (root / EVIDENCE).read_text(encoding="utf-8")
    cmake = (root / CMAKE).read_text(encoding="utf-8")
    tests = (root / TEST_SOURCE).read_text(encoding="utf-8")

    full = one(
        r"Full retail body:\s*\*\s*(\d+) words / 0x[0-9A-Fa-f]+ bytes, "
        r"exe (0x[0-9A-Fa-f]+)[–-](0x[0-9A-Fa-f]+) \(exclusive\)",
        source,
        "full retail window",
        re.DOTALL,
    )
    prefix_matches = list(re.finditer(
        r"Implemented prefix:\s*\*\s*(0x[0-9A-Fa-f]+)\.\.(0x[0-9A-Fa-f]+) "
        r"\((\d+) words\)",
        source,
        re.DOTALL,
    ))
    complete_matches = list(re.finditer(
        r"Complete native body:\s*\*\s*(0x[0-9A-Fa-f]+)\.\.(0x[0-9A-Fa-f]+) "
        r"\((\d+) words\)",
        source,
        re.DOTALL,
    ))
    if len(prefix_matches) + len(complete_matches) != 1:
        raise MetricsError(
            "expected exactly one implemented-prefix or complete-body declaration, found "
            f"{len(prefix_matches)} prefix and {len(complete_matches)} complete"
        )

    total_words = int(full.group(1))
    full_start = int(full.group(2), 16)
    full_end = int(full.group(3), 16)
    implementation = prefix_matches[0] if prefix_matches else complete_matches[0]
    implementation_kind = "prefix" if prefix_matches else "complete"
    prefix_start = int(implementation.group(1), 16)
    prefix_end = int(implementation.group(2), 16)
    implemented_words = int(implementation.group(3))
    if full_start != prefix_start:
        raise MetricsError("implemented prefix does not begin at retail function start")
    if (full_end - full_start) // 4 != total_words:
        raise MetricsError("full retail word count does not match address arithmetic")
    if (prefix_end - prefix_start) // 4 != implemented_words:
        raise MetricsError("implemented word count does not match address arithmetic")
    if implementation_kind == "prefix":
        cut = one(
            r'Bootstrap_ReturnInt\(\s*"([^"]+)",\s*"func_80030894",\s*0\)',
            source,
            "production cut",
            re.DOTALL,
        ).group(1)
        evidence_cut = one(
            r"^NEW_STRICT_FRONTIER=([^\s]+)$",
            evidence,
            "evidence frontier",
            re.MULTILINE,
        ).group(1)
        if cut != evidence_cut:
            raise MetricsError(f"source frontier {cut} != evidence frontier {evidence_cut}")
        if prefix_end >= full_end:
            raise MetricsError("frontier is not inside the retail function")
    else:
        cut = None
        if prefix_end != full_end or implemented_words != total_words:
            raise MetricsError("complete native body does not cover the retail function")
        if re.search(r'Bootstrap_ReturnInt\(\s*"[^"]+",\s*"func_80030894",\s*0\)', source):
            raise MetricsError("complete native body retains a func_80030894 bootstrap cut")

    source_groups = {
        variable: cmake_sources(cmake, variable)
        for variable in ("PLATFORM_SRCS", "BOOTSTRAP_SRCS", "GAME_SRCS")
    }
    explicit_port_sources = optional_cmake_sources(cmake, "PORT_SRCS")
    all_sources = [source for group in source_groups.values() for source in group]
    linked_sources = explicit_port_sources + all_sources
    missing_sources = [
        source for source in linked_sources if not (root / "pc_port" / source).is_file()
    ]
    if missing_sources:
        raise MetricsError("CMake references missing native sources: " + ", ".join(missing_sources))
    duplicate_sources = sorted(
        {source for source in linked_sources if linked_sources.count(source) > 1}
    )
    if duplicate_sources:
        raise MetricsError("duplicate native CMake sources: " + ", ".join(duplicate_sources))
    linked_source_count = len(linked_sources)
    field_runtime_target = bool(
        re.search(r"add_library\s*\(\s*pe_field_runtime\b", cmake)
    )

    # Tests are spread across test_native.c and the test_*.h headers it includes
    # (transitively).  Count in the full include closure or the expected run
    # count silently drifts below the cases the binary actually registers.
    test_text = collect_test_translation_unit(root, tests, TEST_SOURCE)
    ordinary_tests = len(re.findall(r'\bTEST\("', test_text))
    retail_disc_tests = len(re.findall(r'\bTEST_RETAIL_DISC1\("', test_text))
    static_tests = ordinary_tests + retail_disc_tests
    if static_tests == 0:
        raise MetricsError("no native TEST() cases found")
    if retail_disc_tests == 0:
        raise MetricsError("no explicitly tagged retail-Disc native tests found")
    if not re.search(r"tests_run\+\+", test_text):
        raise MetricsError("TEST() no longer visibly increments tests_run")

    return {
        "schema_version": 1,
        "production_frontier": cut,
        "frontier_owner": "func_80030894",
        "implementation_kind": implementation_kind,
        "retail_window": {
            "start": f"0x{full_start:08X}",
            "end": f"0x{full_end:08X}",
            "words": total_words,
        },
        "implemented_window": {
            "start": f"0x{prefix_start:08X}",
            "end": f"0x{prefix_end:08X}",
            "words": implemented_words,
            "percent": round(100.0 * implemented_words / total_words, 2),
        },
        "linked_runtime": {
            "platform_translation_units": len(source_groups["PLATFORM_SRCS"]),
            "bootstrap_translation_units": len(source_groups["BOOTSTRAP_SRCS"]),
            "game_translation_units": len(source_groups["GAME_SRCS"]),
            "total_translation_units": linked_source_count,
            "pe_field_runtime_library_target": field_runtime_target,
        },
        "tests": {
            "static_cases": static_tests,
            "artifact_independent_cases": ordinary_tests,
            "retail_disc_required_cases": retail_disc_tests,
            "private_full_required": static_tests,
        },
        "reachable_semantic_functions": {
            "status": "UNMEASURED",
            "reason": "no authoritative production-run reachability counter exists",
        },
        "evidence": str(EVIDENCE),
    }


def run_tests(binary: Path, tests: dict[str, int], mode: str) -> dict[str, int | str]:
    result = subprocess.run(
        [str(binary)],
        cwd=ROOT,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    output = result.stdout.decode(errors="replace")
    match = re.search(
        r"Results:\s*(\d+) run,\s*(\d+) passed,\s*(\d+) failed,\s*(\d+) skipped",
        output,
    )
    if not match:
        raise MetricsError(f"cannot parse native test summary from {binary}\n{output[-2000:]}")
    run_count, passed, failed, skipped = map(int, match.groups())
    expected_run = tests["static_cases"]
    if mode == "full":
        expected_passed = expected_run
        expected_skipped = 0
    else:
        expected_passed = tests["artifact_independent_cases"]
        expected_skipped = tests["retail_disc_required_cases"]
    if (
        result.returncode
        or run_count != expected_run
        or passed != expected_passed
        or failed
        or skipped != expected_skipped
    ):
        raise MetricsError(
            f"native test gate failed: rc={result.returncode}, "
            f"{run_count} run/{passed} passed/{failed} failed/{skipped} skipped, "
            f"expected {expected_run}/{expected_passed}/0/{expected_skipped} "
            f"in {mode} mode"
        )
    return {
        "mode": mode,
        "run": run_count,
        "passed": passed,
        "failed": failed,
        "skipped": skipped,
    }


def status_markdown(metrics: dict[str, Any]) -> str:
    implemented = metrics["implemented_window"]
    retail = metrics["retail_window"]
    linked = metrics["linked_runtime"]
    tests = metrics["tests"]
    library = "PRESENT" if linked["pe_field_runtime_library_target"] else "ABSENT"
    frontier = (
        f"**`{metrics['production_frontier']}`** in `{metrics['frontier_owner']}`"
        if metrics["production_frontier"]
        else f"**none inside `{metrics['frontier_owner']}` (complete body)**"
    )
    return f"""# Native port status

<!-- Generated by tools/progress/native_metrics.py. Do not edit by hand. -->

- Production frontier: {frontier}
- Implemented retail window: **{implemented['words']} / {retail['words']} words
  ({implemented['percent']:.2f}%)**, `{implemented['start']}..{implemented['end']}`
- Linked native translation units: **{linked['total_translation_units']}**
  ({linked['game_translation_units']} game,
  {linked['bootstrap_translation_units']} bootstrap,
  {linked['platform_translation_units']} platform, plus the explicit
  port entry/global/callback units)
- `pe_field_runtime` library target: **{library}**
- Native test inventory: **{tests['static_cases']}** total —
  **{tests['artifact_independent_cases']}** artifact-independent and
  **{tests['retail_disc_required_cases']}** requiring a legally supplied Disc 1 fixture
- Public normal/sanitizer requirement: **{tests['artifact_independent_cases']} pass,
  {tests['retail_disc_required_cases']} explicitly tagged skips, 0 failures**
- Private full requirement with Disc 1 configured: **{tests['private_full_required']} / {tests['private_full_required']}**
- Production-reachable semantic-function count: **UNMEASURED** — no
  authoritative runtime reachability counter exists yet; this must not be
  replaced by the matching-leaf count.

Evidence: [`{metrics['evidence']}`](../evidence/pe-b54kb6-30894-l9/REPORT.md).
Regenerate/check with `python3 tools/progress/native_metrics.py --write-status`
and `python3 tools/progress/native_metrics.py --check-status`.
"""


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--write-status", action="store_true")
    parser.add_argument("--check-status", action="store_true")
    parser.add_argument("--test-binary", type=Path)
    parser.add_argument(
        "--test-mode",
        choices=("full", "artifact-independent"),
        default="full",
        help="full requires every case; artifact-independent requires only tagged retail-Disc skips",
    )
    args = parser.parse_args(argv)
    try:
        metrics = derive_metrics(ROOT)
        rendered = status_markdown(metrics)
        status = ROOT / STATUS
        if args.write_status:
            status.parent.mkdir(parents=True, exist_ok=True)
            status.write_text(rendered, encoding="utf-8")
        if args.check_status:
            if not status.is_file() or status.read_text(encoding="utf-8") != rendered:
                raise MetricsError(
                    f"stale generated status {STATUS}; run with --write-status"
                )
        test_result = None
        if args.test_binary:
            test_result = run_tests(
                args.test_binary.resolve(), metrics["tests"], args.test_mode
            )
        if args.json:
            payload = dict(metrics)
            if test_result is not None:
                payload["test_run"] = test_result
            print(json.dumps(payload, indent=2))
        else:
            print(
                f"native frontier={metrics['production_frontier'] or 'complete'} "
                f"coverage={metrics['implemented_window']['words']}/"
                f"{metrics['retail_window']['words']} "
                f"linked_tus={metrics['linked_runtime']['total_translation_units']} "
                f"field_runtime={'present' if metrics['linked_runtime']['pe_field_runtime_library_target'] else 'absent'} "
                f"tests={metrics['tests']['static_cases']}"
            )
            if test_result is not None:
                print(
                    f"native tests: {test_result['run']} run, "
                    f"{test_result['passed']} passed, "
                    f"{test_result['failed']} failed, "
                    f"{test_result['skipped']} skipped "
                    f"({test_result['mode']})"
                )
    except (OSError, MetricsError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
