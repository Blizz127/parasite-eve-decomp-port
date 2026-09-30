#!/usr/bin/env python3
"""
Generate Xenogears-schema progress.json for Parasite Eve from splat config.

Reads configs/USA/disc1.yaml subsegment edges and (when present) asm/disc1/*.s
function labels. Does not require objdiff.

Usage:
    python3 tools/progress/generate_progress.py [-o build/progress.json]
"""

from __future__ import annotations

import argparse
import datetime
import json
import os
import re
import sys
from typing import Any

try:
    import yaml
except ImportError:
    print("ERROR: PyYAML required (pip install pyyaml).", file=sys.stderr)
    sys.exit(1)

DEFAULT_CONFIG = "configs/USA/disc1.yaml"
DEFAULT_OUTPUT = "build/progress.json"
MAIN_END = 0x1EE800
EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"

GLABEL_RE = re.compile(r"^glabel\s+(\S+)")
NONMATCHING_RE = re.compile(r"^nonmatching\s+(\S+)")


def find_repo_root() -> str:
    path = os.path.abspath(os.path.dirname(__file__))
    while path != "/":
        if os.path.isfile(os.path.join(path, "CLAUDE.md")):
            return path
        path = os.path.dirname(path)
    return os.getcwd()


def parse_subsegments(config_path: str) -> list[dict[str, Any]]:
    with open(config_path, encoding="utf-8") as fh:
        data = yaml.safe_load(fh)

    for seg in data.get("segments", []):
        if seg.get("name") == "main":
            raw = seg.get("subsegments", [])
            out: list[dict[str, Any]] = []
            for entry in raw:
                if not isinstance(entry, list) or len(entry) < 2:
                    continue
                offset = int(entry[0], 16) if isinstance(entry[0], str) else int(entry[0])
                kind = entry[1]
                name = entry[2] if len(entry) > 2 else None
                out.append({"offset": offset, "kind": kind, "name": name})
            out.sort(key=lambda x: x["offset"])
            return out
    raise SystemExit(f"ERROR: no 'main' segment with subsegments in {config_path}")


def attach_sizes(subsegments: list[dict[str, Any]]) -> list[dict[str, Any]]:
    for i, seg in enumerate(subsegments):
        end = MAIN_END if i + 1 >= len(subsegments) else subsegments[i + 1]["offset"]
        seg["size"] = end - seg["offset"]
        if seg["size"] <= 0:
            raise SystemExit(
                f"ERROR: non-positive size at 0x{seg['offset']:X} "
                f"(end 0x{end:X})"
            )
    return subsegments


def count_asm_functions(asm_path: str) -> int:
    if not os.path.isfile(asm_path):
        return 0
    count = 0
    with open(asm_path, encoding="utf-8", errors="replace") as fh:
        for line in fh:
            if GLABEL_RE.match(line) or NONMATCHING_RE.match(line):
                count += 1
    return count


def asm_file_for_offset(repo: str, offset: int, kind: str) -> str | None:
    if kind != "asm":
        return None
    # Prefix rodata uses data/800.rodata.s; mid rodata is separate.
    if offset == 0x800:
        return os.path.join(repo, "asm/disc1/data/800.rodata.s")
    if offset == 0x818A0:
        return os.path.join(repo, "asm/disc1/data/818A0.rodata.s")
    return os.path.join(repo, f"asm/disc1/{offset:X}.s")


def unit_measures(
    total_code: int,
    matched_code: int,
    total_functions: int,
    matched_functions: int,
) -> dict[str, Any]:
    pct = (matched_code / total_code * 100.0) if total_code else 0.0
    fpct = (matched_functions / total_functions * 100.0) if total_functions else 0.0
    return {
        "total_code": str(total_code),
        "matched_code": str(matched_code),
        "matched_code_percent": round(pct, 6),
        "total_functions": total_functions,
        "matched_functions": matched_functions,
        "matched_functions_percent": round(fpct, 6),
        "total_units": 1,
        "matched_data_percent": 100.0 if matched_code == total_code else 0.0,
        "complete_data_percent": 100.0 if matched_code == total_code else 0.0,
    }


def build_report(repo: str, config_path: str) -> dict[str, Any]:
    subsegments = attach_sizes(parse_subsegments(config_path))
    code_segs = [s for s in subsegments if s["kind"] in ("asm", "c")]

    c_leaves = [s for s in code_segs if s["kind"] == "c"]
    asm_segs = [s for s in code_segs if s["kind"] == "asm"]

    matched_code = sum(s["size"] for s in c_leaves)
    total_code = sum(s["size"] for s in code_segs)
    matched_functions = len(c_leaves)

    asm_func_total = 0
    asm_available = os.path.isdir(os.path.join(repo, "asm/disc1"))
    metadata_note = None

    units: list[dict[str, Any]] = []

    for seg in c_leaves:
        name = seg["name"] or f"0x{seg['offset']:X}"
        units.append({
            "name": name,
            "measures": unit_measures(seg["size"], seg["size"], 1, 1),
            "sections": [{"name": ".text", "size": str(seg["size"]), "metadata": {}}],
            "functions": [{"name": name, "size": str(seg["size"]), "metadata": {}}],
            "metadata": {"progress_categories": ["c_leaves"]},
        })

    for seg in asm_segs:
        offset_hex = f"{seg['offset']:X}"
        asm_path = asm_file_for_offset(repo, seg["offset"], seg["kind"])
        funcs = count_asm_functions(asm_path) if asm_path else 0
        asm_func_total += funcs
        units.append({
            "name": f"asm/{offset_hex}",
            "measures": unit_measures(seg["size"], 0, funcs, 0),
            "sections": [{"name": ".text", "size": str(seg["size"]), "metadata": {}}],
            "functions": [],
            "metadata": {"progress_categories": ["remaining_asm"]},
        })

    if asm_available and asm_func_total == 0:
        metadata_note = (
            "asm/disc1 present but no glabel/nonmatching labels counted; "
            "run scripts/split_us.sh to refresh split output."
        )
    elif not asm_available:
        metadata_note = (
            "asm/disc1 not present locally; total_functions counts matched C "
            "leaves only. Run scripts/split_us.sh for asm function totals."
        )
        asm_func_total = 0

    total_functions = matched_functions + asm_func_total
    matched_code_percent = (matched_code / total_code * 100.0) if total_code else 0.0

    cat_c_code = matched_code
    cat_c_funcs = matched_functions
    cat_asm_code = total_code - matched_code
    cat_asm_funcs = asm_func_total

    categories = [
        {
            "id": "c_leaves",
            "name": "Matched C",
            "measures": {
                "total_code": str(cat_c_code),
                "matched_code": str(cat_c_code),
                "matched_code_percent": 100.0 if cat_c_code else 0.0,
                "total_functions": cat_c_funcs,
                "matched_functions": cat_c_funcs,
                "matched_functions_percent": 100.0 if cat_c_funcs else 0.0,
            },
        },
        {
            "id": "remaining_asm",
            "name": "Remaining asm",
            "measures": {
                "total_code": str(cat_asm_code),
                "matched_code": "0",
                "matched_code_percent": 0.0,
                "total_functions": cat_asm_funcs,
                "matched_functions": 0,
                "matched_functions_percent": 0.0,
            },
        },
    ]

    measures = {
        "total_code": str(total_code),
        "matched_code": str(matched_code),
        "matched_code_percent": round(matched_code_percent, 6),
        "total_data": "0",
        "total_functions": total_functions,
        "matched_functions": matched_functions,
        "matched_functions_percent": round(
            (matched_functions / total_functions * 100.0) if total_functions else 0.0,
            6,
        ),
        "fuzzy_match_percent": round(matched_code_percent, 6),
        "total_units": len(units),
    }

    report: dict[str, Any] = {
        "version": 2,
        "generated": datetime.datetime.now().strftime("%Y-%m-%d %H:%M"),
        "measures": measures,
        "categories": categories,
        "units": units,
        "metadata": {
            "project": "Parasite Eve (USA) SLUS-006.62",
            "sha1": EXE_SHA1,
            "config": config_path,
            "generator": "tools/progress/generate_progress.py",
            "fuzzy_source": "exact_leaf_bytes_v1",
            "note": (
                "fuzzy_match_percent mirrors matched_code_percent until objdiff "
                "report is wired in."
            ),
        },
    }
    if metadata_note:
        report["metadata"]["asm_note"] = metadata_note
    return report


def main() -> None:
    repo = find_repo_root()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "-c", "--config",
        default=os.path.join(repo, DEFAULT_CONFIG),
        help="splat config yaml",
    )
    parser.add_argument(
        "-o", "--output",
        default=os.path.join(repo, DEFAULT_OUTPUT),
        help="output progress.json path",
    )
    args = parser.parse_args()

    if not os.path.isfile(args.config):
        print(f"ERROR: config not found: {args.config}", file=sys.stderr)
        sys.exit(1)

    report = build_report(repo, args.config)
    os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
    with open(args.output, "w", encoding="utf-8") as fh:
        json.dump(report, fh, indent=2)
        fh.write("\n")

    m = report["measures"]
    print(f"Wrote {args.output}")
    print(
        f"  code: {float(m['matched_code_percent']):.2f}% "
        f"({m['matched_code']}/{m['total_code']} bytes)"
    )
    print(
        f"  functions: {m['matched_functions']}/{m['total_functions']} "
        f"({m['matched_functions_percent']:.2f}%)"
    )
    print(f"  units: {m['total_units']} ({len([u for u in report['units'] if 'c_leaves' in u['metadata'].get('progress_categories', [])])} C + "
          f"{len([u for u in report['units'] if 'remaining_asm' in u['metadata'].get('progress_categories', [])])} asm)")


if __name__ == "__main__":
    main()
