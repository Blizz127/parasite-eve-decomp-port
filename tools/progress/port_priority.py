#!/usr/bin/env python3
"""Rank matching candidates by native-port relevance, with word count last."""

from __future__ import annotations

import argparse
import json
import re
import sys
from collections import Counter, deque
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
CONFIG = Path("configs/USA/port_priority.json")
STATUS = Path("docs/generated/NATIVE_CANDIDATE_PRIORITY.md")
FUNC = r"func_[0-9A-Fa-f]{8}"
ROW_RE = re.compile(
    rf"^\| `0x([0-9A-Fa-f]+)` \| `({FUNC})` \| (\d+) \| (\d+)/(\d+) \|"
)

sys.path.insert(0, str(ROOT / "tools" / "build"))
sys.path.insert(0, str(ROOT / "tools" / "progress"))
from disc1_plan import PlanError, build_plan  # noqa: E402
from native_metrics import (  # noqa: E402
    CMAKE,
    MetricsError,
    cmake_sources,
    derive_metrics,
    optional_cmake_sources,
)


class PriorityError(RuntimeError):
    """A priority input or derived invariant failed."""


def load_config(root: Path) -> dict[str, Any]:
    path = root / CONFIG
    try:
        config = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise PriorityError(f"cannot load {CONFIG}: {exc}") from exc
    if config.get("schema_version") != 1:
        raise PriorityError(f"{CONFIG}: schema_version must be 1")
    if not re.fullmatch(FUNC, str(config.get("production_root", ""))):
        raise PriorityError(f"{CONFIG}: invalid production_root")
    pool = config.get("candidate_pool")
    if not isinstance(pool, str) or not pool or not (root / pool).is_file():
        raise PriorityError(f"{CONFIG}: candidate_pool must name an existing file")
    frontier = config.get("active_frontier")
    if not isinstance(frontier, dict):
        raise PriorityError(f"{CONFIG}: active_frontier must be an object")
    # `label` names the cut still missing inside the owner.  It is null once the
    # owner body is complete, which native_metrics reports as
    # production_frontier=None.
    label = frontier.get("label")
    if label is not None and (not isinstance(label, str) or not label):
        raise PriorityError(
            f"{CONFIG}: active_frontier.label must be a non-empty string or null"
        )
    for key in ("owner", "first_excluded_pc", "evidence"):
        if not isinstance(frontier.get(key), str) or not frontier[key]:
            raise PriorityError(f"{CONFIG}: active_frontier.{key} is required")
    if not re.fullmatch(FUNC, frontier["owner"]):
        raise PriorityError(f"{CONFIG}: invalid active_frontier.owner")
    try:
        int(frontier["first_excluded_pc"], 16)
    except ValueError as exc:
        raise PriorityError(f"{CONFIG}: invalid first_excluded_pc") from exc
    callees = frontier.get("remaining_direct_callees")
    if (
        not isinstance(callees, list)
        or any(not isinstance(name, str) or not re.fullmatch(FUNC, name) for name in callees)
        or len(callees) != len(set(callees))
    ):
        raise PriorityError(f"{CONFIG}: remaining_direct_callees must be unique functions")
    if label and not callees:
        raise PriorityError(
            f"{CONFIG}: an incomplete frontier must name its remaining_direct_callees"
        )
    if not (root / frontier["evidence"]).is_file():
        raise PriorityError(f"{CONFIG}: missing frontier evidence {frontier['evidence']}")
    return config


def parse_pool(root: Path, pool: Path) -> list[dict[str, Any]]:
    text = (root / pool).read_text(encoding="utf-8")
    section: str | None = None
    rows: list[dict[str, Any]] = []
    for line in text.splitlines():
        heading = re.match(r"^## ([WSBC])(?:\s|$)", line)
        if heading:
            section = heading.group(1)
            continue
        if line.startswith("## "):
            section = None
            continue
        match = ROW_RE.match(line)
        if match and section:
            offset, name, words, callers, refs = match.groups()
            rows.append(
                {
                    "offset": int(offset, 16),
                    "name": name,
                    "words": int(words),
                    "retail_callers": int(callers),
                    "retail_refs": int(refs),
                    "shape_class": section,
                }
            )
    names = [row["name"] for row in rows]
    if not rows or len(names) != len(set(names)):
        raise PriorityError(f"{pool}: empty or duplicate candidate rows")
    return rows


def strip_c_noise(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", lambda m: " " * len(m.group(0)), text, flags=re.DOTALL)
    text = re.sub(r"//[^\n]*", lambda m: " " * len(m.group(0)), text)
    text = re.sub(
        r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
        lambda m: " " * len(m.group(0)),
        text,
    )
    return text


def function_bodies(text: str) -> dict[str, str]:
    clean = strip_c_noise(text)
    bodies: dict[str, str] = {}
    pattern = re.compile(
        rf"^[ \t]*(?:[A-Za-z_][A-Za-z0-9_]*[ \t*]+)+"
        rf"({FUNC})\s*\([^;{{}}]*\)\s*\{{",
        re.MULTILINE | re.DOTALL,
    )
    for match in pattern.finditer(clean):
        name = match.group(1)
        opening = clean.find("{", match.start())
        depth = 0
        end = None
        for index in range(opening, len(clean)):
            if clean[index] == "{":
                depth += 1
            elif clean[index] == "}":
                depth -= 1
                if depth == 0:
                    end = index + 1
                    break
        if end is None:
            raise PriorityError(f"unclosed native function body {name}")
        if name in bodies:
            raise PriorityError(f"duplicate linked native definition {name}")
        bodies[name] = clean[opening:end]
    return bodies


def linked_native_sources(root: Path) -> list[Path]:
    cmake = (root / CMAKE).read_text(encoding="utf-8")
    # PORT_SRCS is optional: the project folded it into PLATFORM/BOOTSTRAP/GAME
    # and native_metrics treats a missing list as empty.  Match that here so the
    # ranking tool tracks the same source set the metrics report.
    names: list[str] = list(optional_cmake_sources(cmake, "PORT_SRCS"))
    for variable in ("PLATFORM_SRCS", "BOOTSTRAP_SRCS", "GAME_SRCS"):
        names.extend(cmake_sources(cmake, variable))
    unique = list(dict.fromkeys(names))
    paths = [root / "pc_port" / name for name in unique]
    missing = [str(path.relative_to(root)) for path in paths if not path.is_file()]
    if missing:
        raise PriorityError("missing linked native sources: " + ", ".join(missing))
    return paths


def native_graph(root: Path) -> tuple[dict[str, set[str]], Counter[str]]:
    bodies: dict[str, str] = {}
    for path in linked_native_sources(root):
        for name, body in function_bodies(path.read_text(encoding="utf-8")).items():
            if name in bodies:
                raise PriorityError(f"duplicate linked native definition {name}")
            bodies[name] = body
    graph: dict[str, set[str]] = {}
    references: Counter[str] = Counter()
    for owner, body in bodies.items():
        calls = re.findall(rf"\b({FUNC})\s*\(", body)
        graph[owner] = set(calls) - {owner}
        references.update(name for name in calls if name != owner)
    return graph, references


def distances(graph: dict[str, set[str]], root: str) -> dict[str, int]:
    if root not in graph:
        raise PriorityError(f"production root {root} has no linked native definition")
    result = {root: 0}
    queue = deque([root])
    while queue:
        owner = queue.popleft()
        for callee in graph.get(owner, set()):
            if callee in graph and callee not in result:
                result[callee] = result[owner] + 1
                queue.append(callee)
    return result


def priority_key(row: dict[str, Any]) -> tuple[Any, ...]:
    """Port signals dominate fan-in; word count is the final semantic tie-breaker."""
    return (
        -int(row["frontier_direct"]),
        -int(row["unresolved_native_reference"]),
        -int(row["native_reachable"]),
        row["native_distance"] if row["native_distance"] is not None else 1_000_000,
        -row["native_reference_count"],
        -row["retail_callers"],
        row["words"],
        row["offset"],
    )


def derive_priority(root: Path = ROOT) -> dict[str, Any]:
    config = load_config(root)
    metrics = derive_metrics(root)
    frontier = config["active_frontier"]
    if frontier["label"] != metrics["production_frontier"]:
        raise PriorityError("priority frontier label disagrees with generated native metrics")
    if frontier["owner"] != metrics["frontier_owner"]:
        raise PriorityError("priority frontier owner disagrees with generated native metrics")
    if int(frontier["first_excluded_pc"], 16) != int(
        metrics["implemented_window"]["end"], 16
    ):
        raise PriorityError("priority frontier PC disagrees with implemented window")

    plan = build_plan(root=root)
    c_names = {unit["name"] for unit in plan["units"] if unit["kind"] == "c"}
    c_offsets = {unit["start"]: unit["name"] for unit in plan["units"] if unit["kind"] == "c"}
    graph, native_refs = native_graph(root)
    native_distance = distances(graph, config["production_root"])
    frontier_callees = set(frontier["remaining_direct_callees"])
    unknown_frontier = sorted(frontier_callees - c_names - set(graph))
    if unknown_frontier:
        raise PriorityError("frontier callees lack matching/native sources: " + ", ".join(unknown_frontier))

    candidates: list[dict[str, Any]] = []
    integrated = 0
    pool = Path(config["candidate_pool"])
    for row in parse_pool(root, pool):
        if row["name"] in c_names:
            if c_offsets.get(row["offset"]) != row["name"]:
                raise PriorityError(f"integrated candidate geometry mismatch: {row['name']}")
            integrated += 1
            continue
        row = dict(row)
        row["frontier_direct"] = row["name"] in frontier_callees
        row["native_linked"] = row["name"] in graph
        row["native_reachable"] = row["name"] in native_distance
        row["native_distance"] = native_distance.get(row["name"])
        row["native_reference_count"] = native_refs[row["name"]]
        row["unresolved_native_reference"] = bool(
            native_refs[row["name"]] and row["name"] not in graph
        )
        candidates.append(row)

    candidates.sort(key=priority_key)
    for rank, row in enumerate(candidates, 1):
        row["rank"] = rank
    return {
        "schema_version": 1,
        "matching_count": plan["counts"]["c"],
        "pool_source": str(pool),
        "pool_rows": integrated + len(candidates),
        "pool_rows_already_integrated": integrated,
        "active_candidates": len(candidates),
        "production_root": config["production_root"],
        "active_frontier": frontier,
        "frontier_dependencies": [
            {
                "name": name,
                "matching_c": name in c_names,
                "native_linked": name in graph,
                "native_reachable": name in native_distance,
            }
            for name in frontier["remaining_direct_callees"]
        ],
        "candidates": candidates,
    }


def status_markdown(result: dict[str, Any]) -> str:
    candidates = result["candidates"]
    frontier = result["active_frontier"]
    lines = [
        "# Native-prioritized matching candidates",
        "",
        "<!-- Generated by tools/progress/port_priority.py. Do not edit by hand. -->",
        "",
        f"- Matching baseline: **{result['matching_count']}** exact C spans",
        f"- Active classified Tier-2 candidates: **{result['active_candidates']}** "
        f"({result['pool_rows_already_integrated']} historical rows already integrated)",
        f"- Production root: **`{result['production_root']}`**",
        (
            f"- Active frontier: **`{frontier['label']}`** at "
            f"`{frontier['first_excluded_pc']}` in `{frontier['owner']}`"
            if frontier["label"]
            else f"- Active frontier: **none** — `{frontier['owner']}` body is complete"
        ),
        "",
        "The order is lexicographic: direct frontier dependency, unresolved native",
        "reference, statically production-reachable linked implementation, native",
        "call-graph distance/reference count, retail fan-in, and only then word count.",
        "Static reachability is a scheduling signal, not proof that a runtime path has",
        "executed. Current production-reachable semantic coverage remains separately",
        "reported as unmeasured.",
        "",
        "## Frontier dependency closure",
        "",
        "| function | matching C | native linked | statically reachable |",
        "|---|---:|---:|---:|",
    ]
    for dep in result["frontier_dependencies"]:
        lines.append(
            f"| `{dep['name']}` | {'yes' if dep['matching_c'] else 'no'} | "
            f"{'yes' if dep['native_linked'] else 'no'} | "
            f"{'yes' if dep['native_reachable'] else 'no'} |"
        )
    if not result["frontier_dependencies"]:
        lines.append(
            "| _none_ | — | — | — |"
            "  <!-- owner body complete; the frontier has no remaining cut -->"
        )
    lines.extend(
        [
            "",
            "## Current order",
            "",
            "| rank | function | class | words | frontier | native distance | native refs | retail callers |",
            "|---:|---|:---:|---:|:---:|---:|---:|---:|",
        ]
    )
    for row in candidates:
        distance = "—" if row["native_distance"] is None else str(row["native_distance"])
        lines.append(
            f"| {row['rank']} | `{row['name']}` | {row['shape_class']} | "
            f"{row['words']} | {'yes' if row['frontier_direct'] else 'no'} | "
            f"{distance} | {row['native_reference_count']} | {row['retail_callers']} |"
        )
    lines.extend(
        [
            "",
            f"Pool classification source: [`{result['pool_source']}`]"
            "(../evidence/volume-campaign-20260831/TIER2_CLASSES.md).",
            f"Frontier evidence: [`{frontier['evidence']}`]"
            "(../evidence/pe-b54kb6-30894-l9/REPORT.md).",
            "",
        ]
    )
    return "\n".join(lines)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--write-status", action="store_true")
    parser.add_argument("--check-status", action="store_true")
    parser.add_argument("--emit-json", type=Path)
    args = parser.parse_args(argv)
    try:
        result = derive_priority(ROOT)
        rendered = status_markdown(result)
        status = ROOT / STATUS
        if args.write_status:
            status.parent.mkdir(parents=True, exist_ok=True)
            status.write_text(rendered, encoding="utf-8")
        if args.check_status:
            if not status.is_file() or status.read_text(encoding="utf-8") != rendered:
                raise PriorityError(f"stale generated status {STATUS}; run with --write-status")
        if args.emit_json:
            destination = args.emit_json if args.emit_json.is_absolute() else ROOT / args.emit_json
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
        if args.json:
            print(json.dumps(result, indent=2))
        else:
            top = result["candidates"][0]["name"] if result["candidates"] else "none"
            print(
                f"port priority: {result['active_candidates']} active candidates, "
                f"{result['pool_rows_already_integrated']} integrated, top={top}"
            )
    except (OSError, PlanError, MetricsError, PriorityError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
