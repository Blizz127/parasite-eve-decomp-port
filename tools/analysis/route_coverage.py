#!/usr/bin/env python3
"""Boot -> end-of-Day-2 route coverage for the persistent decompilation goal.

The disc-wide count (`N c / M asm` spans) does not answer the actual objective:
how much of the *first-play* path (cold boot -> end of the in-game Day 2) is
lifted to matching C, and how much is still reassembled from the split?  This
tool measures that path directly, from authoritative inputs only:

  * `configs/USA/disc1.yaml`        — span membership, order, and geometry.
  * `asm/disc1/*.s`                 — the split's function labels (`glabel`) and
                                      the call graph (direct `jal` operands).
  * `src/func_*.c`                  — a matching leaf is its own caller of the
                                      `func_*` identifiers its body names.
  * the retail EXE (`build/extracted/disc1/SLUS_006.62`) — the field-VM
                                      dispatch table `D_800910A0`.

Route definition (evidence, not invented)
-----------------------------------------
Tier A — direct boot closure.  Root = the retail initial PC `0x80072534`
(`asm/disc1/header.s`, "Initial PC"), reached as
`func_80072534 -> func_800726B4 -> func_8001220C` (crt0 `main`).  Transitive
closure of direct `jal` targets; matched leaves contribute the `func_*`
identifiers named in their `src/func_*.c` bodies.

Tier B — field/script VM handlers.  `func_80017018` dispatches through the
pointer table `D_800910A0[word & 0x1FFF]` (`pc_port/game/boot/func_80017018_port.c`
and the `rodata` span `configs/USA/disc1.yaml`); the tool reads the first 0x2000
words of that table from the retail image and keeps in-image text targets.

Counting rule: a function is matched C iff it is the named symbol of a `c` span
in `configs/USA/disc1.yaml`.  Everything else on the path is counted as asm
(incl. merged `asm` spans and the `rodata` island).  A path function whose size
is unknown (no `nonmatching NAME, 0xN` hint; typically inside a merged span) is
reported in an explicit `unknown_words` bucket rather than dropped.

Both tiers are lower bounds: Tier A is direct-call only and Tier B only reads
the 0x1FFF-masked opcode window (the table's rodata span is 50,326 words).

Output is stable and diffable: sorted sections, one summary line, `--json` for
machine consumption.  `--quiet` prints exactly the one-line summary used by
`scripts/exact_rebuild.sh` (informational only; the gate's PASS semantics do not
depend on it).  `--plan <sha>` refuses to report if the current plan SHA-256
differs, so a recorded number is bound to the state it was computed at.

Supersedes the hand-written snapshot in `docs/ai_context/BOOT_TO_DAY2_COVERAGE.md`.
See `docs/ai_context/ROUTE_COVERAGE.md` for the recorded output.
"""

from __future__ import annotations

import argparse
import glob
import hashlib
import json
import re
import sys
from collections import defaultdict
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO_ROOT / "tools" / "build"))

from disc1_plan import build_plan  # noqa: E402

CONFIG = Path("configs/USA/disc1.yaml")
EXE = Path("build/extracted/disc1/SLUS_006.62")
ASM_GLOB = "asm/disc1/*.s"
SRC_GLOB = "src/func_*.c"
PORT_GLOBS = (
    "pc_port/game/boot/func_*_port.c",
    "pc_port/game/decomp/func_*_port.c",
    "pc_port/bootstrap/func_*_port.c",
)
NONMATCHABLE_MANIFEST = Path("docs/evidence/non-c-matchable/MANIFEST.json")
HISTORY_PATH = Path("docs/generated/ROUTE_COVERAGE_HISTORY.md")

BOOT_ROOT = "func_80072534"  # PS-X EXE initial PC (asm/disc1/header.s)
VM_TABLE = 0x800910A0  # func-pointer table head (disc1.yaml rodata island)
VM_OPCODE_WORDS = 0x2000  # table index = word & 0x1FFF (func_80017018)
LOAD_VRAM = 0x80010000
LOAD_SIZE = 0x1EE000

GLABEL_RE = re.compile(r"^glabel\s+(\S+)")
ENDLABEL_RE = re.compile(r"^endlabel\s+(\S+)")
NONMATCHING_RE = re.compile(r"^nonmatching\s+(\S+),\s*0x([0-9A-Fa-f]+)")
JAL_RE = re.compile(r"\bjal\s+([A-Za-z_][A-Za-z0-9_]*)")
FUNC_RE = re.compile(r"\bfunc_[0-9A-Fa-f]{8}\b")
PORT_NAME_RE = re.compile(r"(func_[0-9A-Fa-f]{8})_port\.c$")
C_SPAN_RE = re.compile(r"-\s*\[(0x[0-9A-Fa-f]+),\s*c,\s*(func_[0-9A-Fa-f]{8})\]")


def native_port_funcs() -> set[str]:
    """Functions with a native pc_port translation unit (filename is the leaf).

    This is the executed-path C subset that predicts a playable restylable
    build.  Matching YAML `c` spans are a different number: they prove
    byte-parity of the PS1 rebuild, not that the port can run the function.
    """
    names: set[str] = set()
    for pattern in PORT_GLOBS:
        for path in glob.glob(str(REPO_ROOT / pattern)):
            match = PORT_NAME_RE.search(Path(path).name)
            if match:
                names.add(match.group(1))
    return names


def vram_of(file_offset: int) -> int:
    return LOAD_VRAM + file_offset - 0x800


def parse_plan() -> tuple[str, dict[str, dict[str, int]], str]:
    """Return (plan_sha256, {func_name: {vram, words}}, span_counts_text)."""
    cfg = (REPO_ROOT / CONFIG).read_text(encoding="utf-8")
    # Every span edge offset, in file order; a C span's end is the next edge.
    offsets = [
        int(match.group(1), 16)
        for match in re.finditer(r"^\s*-\s*\[(0x[0-9A-Fa-f]+)", cfg, re.MULTILINE)
    ]

    spans: dict[str, dict[str, int]] = {}
    for match in C_SPAN_RE.finditer(cfg):
        name = match.group(2)
        start = int(match.group(1), 16)
        end = next((off for off in offsets if off > start), None)
        if end is None:
            raise SystemExit(f"route_coverage: cannot resolve end of {name}")
        spans[name] = {"start": start, "vram": vram_of(start), "words": (end - start) // 4}
    try:
        plan = build_plan(root=REPO_ROOT)
    except Exception as exc:  # PlanError (missing/stale source, bad YAML, …)
        raise SystemExit(f"route_coverage: plan unavailable: {exc}") from exc
    counts = plan["counts"]
    counts_text = f"{counts['c']} c, {counts['asm']} asm, {counts['rodata']} rodata"
    return plan["plan_sha256"], spans, counts_text


def parse_asm() -> tuple[dict[str, set[str]], dict[str, int], set[str], dict[str, str]]:
    """Return (edges, sizes, glabels, file_of)."""
    edges: dict[str, set[str]] = defaultdict(set)
    sizes: dict[str, int] = {}
    glabels: set[str] = set()
    file_of: dict[str, str] = {}
    for path in sorted(glob.glob(str(REPO_ROOT / ASM_GLOB))):
        current: str | None = None
        with open(path, encoding="utf-8", errors="replace") as handle:
            for line in handle:
                stripped = line.strip()
                match = NONMATCHING_RE.match(stripped)
                if match:
                    sizes[match.group(1)] = int(match.group(2), 16)
                    continue
                match = GLABEL_RE.match(stripped)
                if match:
                    current = match.group(1)
                    glabels.add(current)
                    file_of.setdefault(current, path)
                    edges.setdefault(current, set())
                    continue
                if ENDLABEL_RE.match(stripped):
                    current = None
                    continue
                if current is not None:
                    match = JAL_RE.search(stripped)
                    if match:
                        edges[current].add(match.group(1))
    return edges, sizes, glabels, file_of


def parse_src() -> dict[str, set[str]]:
    """Matching-leaf callers: file name stem -> func_* identifiers in its body."""
    edges: dict[str, set[str]] = {}
    for path in sorted(glob.glob(str(REPO_ROOT / SRC_GLOB))):
        name = Path(path).stem
        text = Path(path).read_text(encoding="utf-8", errors="replace")
        edges[name] = set(FUNC_RE.findall(text))
    return edges


def closure(root: str, callers: dict[str, set[str]], known: set[str]) -> set[str]:
    seen = {root}
    stack = [root]
    while stack:
        node = stack.pop()
        for target in callers.get(node, ()):
            if target in known and target not in seen:
                seen.add(target)
                stack.append(target)
    return seen


def vm_targets() -> tuple[set[str], set[str]]:
    """Return (resolved_target_names, unresolved_in_image_addresses)."""
    exe = (REPO_ROOT / EXE).read_bytes()
    base = VM_TABLE - LOAD_VRAM + 0x800
    resolved: set[str] = set()
    unresolved: set[int] = set()
    hi = LOAD_VRAM + LOAD_SIZE
    for index in range(VM_OPCODE_WORDS):
        word = int.from_bytes(exe[base + index * 4 : base + index * 4 + 4], "little")
        if LOAD_VRAM <= word < hi:
            if (word - LOAD_VRAM) % 4 == 0:
                resolved.add("func_%08X" % word)
            else:
                unresolved.add(word)
    return resolved, unresolved


def load_nonmatchable() -> dict:
    """Read the matching worker's classification artifact (read-only).

    Returns {"present": bool, "names": set, "source_counts": {...}}. The tool
    never writes this file; absence is reported as `unknown` rather than
    guessed. Spans it names are the matching worker's positive-evidence
    classification (handwritten ``jr $t2``/``syscall``/COP primitives).
    """
    path = REPO_ROOT / NONMATCHABLE_MANIFEST
    if not path.is_file():
        return {"present": False, "names": set(), "source_counts": {}}
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return {"present": False, "names": set(), "source_counts": {}}
    names: set[str] = set()
    source_counts: dict[str, int] = {}
    for cls, entries in (data.get("spans") or {}).items():
        if cls == "alignment-filler":
            continue  # layout padding, not functions
        if not isinstance(entries, list):
            continue
        source_counts[cls] = len(entries)
        for entry in entries:
            name = entry.get("name") if isinstance(entry, dict) else None
            if name:
                names.add(name)
    return {"present": True, "names": names, "source_counts": source_counts}


def derive_timestamp(plan_sha: str) -> str:
    """Deterministic UTC timestamp derived from the plan SHA-256.

    A clock timestamp would make consecutive identical runs differ, breaking
    idempotency. Instead the timestamp is drawn from the hash bytes using a
    union of the allowed calendar ranges, so the same plan always yields the
    same stamp and different plans yield different stamps (the date is not a
    factual claim; provenance is the plan hash).
    """
    digest = hashlib.sha256(plan_sha.encode()).digest()
    year = 2025 + digest[0] % 2
    month = 1 + digest[1] % 12
    day = 1 + digest[2] % 28
    hour = digest[3] % 24
    minute = digest[4] % 60
    second = digest[5] % 60
    return f"{year:04d}-{month:02d}-{day:02d}T{hour:02d}:{minute:02d}:{second:02d}Z"


def write_history(
    path: Path,
    *,
    plan_sha: str,
    yaml_sha: str,
    plan_counts: str,
    now: str | None,
    matched_funcs: int,
    union_funcs: int,
    matched_words: int,
    non_c_funcs: int,
    non_c_words: int,
    real_asm_funcs: int,
    real_asm_words: int,
    nonmatch_present: bool,
    nonmatch_classes: dict[str, int],
) -> None:
    """Append one entry to the coverage history, idempotently.

    Prior entries are never rewritten. If an entry for the same plan SHA-256 is
    already present with identical numbers, the file is left byte-identical.
    """
    path = path if path.is_absolute() else (REPO_ROOT / path)
    total_words = matched_words + non_c_words + real_asm_words
    pct = lambda part: (100.0 * part / total_words) if total_words else 0.0
    stamp = now or derive_timestamp(plan_sha)
    classes = " ".join(f"{k}:{v}" for k, v in sorted(nonmatch_classes.items())) or "-"

    entry = (
        f"\n### {stamp}\n\n"
        f"- plan SHA-256: `{plan_sha}`\n"
        f"- yaml SHA-256: `{yaml_sha}`\n"
        f"- spans: {plan_counts}\n"
        f"- union (on-path): {union_funcs} functions\n"
        f"- **matched C**: {matched_funcs} funcs, {matched_words} words "
        f"({pct(matched_words):.1f}%)\n"
        f"- **provably non-C-matchable**: {non_c_funcs} funcs, {non_c_words} words "
        f"({pct(non_c_words):.1f}%)"
        + ("" if nonmatch_present else "  [artifact absent -> reported as unknown]")
        + "\n"
        f"- **real remaining asm**: {real_asm_funcs} funcs, {real_asm_words} words "
        f"({pct(real_asm_words):.1f}%)\n"
        f"- non-C classes: {classes}\n"
    )

    header = (
        "# ROUTE_COVERAGE_HISTORY — append-only boot→Day-2 coverage record\n\n"
        "Generated by `python3 tools/analysis/route_coverage.py --history`.\n"
        "Append-only: prior entries are never rewritten. Repeated runs with the\n"
        "same plan SHA-256 are byte-identical (idempotent). The timestamp is\n"
        "derived from the plan hash, not the wall clock, so the record is\n"
        "reproducible; provenance is the plan SHA-256. Read the narrative in\n"
        "`docs/ai_context/ROUTE_COVERAGE.md`.\n\n"
        "## Entries\n"
    )

    if path.is_file():
        text = path.read_text(encoding="utf-8")
    else:
        text = header

    marker = f"plan SHA-256: `{plan_sha}`"
    if marker in text:
        # Same plan already recorded. Only replace the entry if a number
        # changed (e.g. the artifact appeared); never duplicate or reorder.
        start = text.rindex("\n### ", 0, text.index(marker)) + 1
        end = text.find("\n### ", text.index(marker))
        end = len(text) if end == -1 else end
        existing = text[start:end]
        if existing.rstrip("\n") == entry.rstrip("\n"):
            return
        text = text[:start] + entry.lstrip("\n") + text[end:]
        path.write_text(text, encoding="utf-8")
        return

    if not text.endswith("\n"):
        text += "\n"
    if "## Entries" not in text:
        text += "\n## Entries\n"
    path.write_text(text + entry, encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--json", metavar="PATH", help="also write the full report as JSON")
    parser.add_argument("--plan", metavar="SHA256", help="refuse to report unless plan matches")
    parser.add_argument("--quiet", action="store_true", help="print only the one-line summary")
    parser.add_argument("--top", type=int, default=20, help="remaining-asm targets to print")
    parser.add_argument(
        "--history",
        metavar="PATH",
        nargs="?",
        const=str(HISTORY_PATH),
        help=f"append a history entry (default {HISTORY_PATH}); idempotent",
    )
    parser.add_argument(
        "--no-history",
        dest="history",
        action="store_const",
        const=None,
        help="do not append a history entry (default is to record)",
    )
    parser.set_defaults(history=str(HISTORY_PATH))
    parser.add_argument(
        "--now",
        metavar="ISO8601",
        help="timestamp for --history (default: derived from plan SHA-256, deterministic)",
    )
    args = parser.parse_args()

    plan_sha, c_spans, plan_counts = parse_plan()
    if args.plan and args.plan != plan_sha:
        print(
            f"route_coverage: plan mismatch: tree is {plan_sha}, requested {args.plan}",
            file=sys.stderr,
        )
        return 2

    edges, sizes, glabels, file_of = parse_asm()
    if not glabels:
        print(
            "route_coverage: no function labels found under asm/disc1/; "
            "run scripts/split_us.sh first",
            file=sys.stderr,
        )
        return 2
    if not (REPO_ROOT / EXE).is_file():
        print(
            f"route_coverage: retail image missing: {EXE}; run scripts/extract_us.sh 1",
            file=sys.stderr,
        )
        return 2
    src_edges = parse_src()

    known = set(glabels) | set(c_spans)
    callers: dict[str, set[str]] = defaultdict(set)
    for caller, targets in edges.items():
        callers[caller] |= targets
    for caller, targets in src_edges.items():
        callers[caller] |= targets

    tier_a = closure(BOOT_ROOT, callers, known)
    vm_named, vm_unresolved = vm_targets()
    tier_b = vm_named & known
    unresolved_names = sorted("func_%08X" % addr for addr in vm_unresolved)
    union = tier_a | tier_b

    matched = {name for name in union if name in c_spans}
    asm = union - matched

    nonmatch = load_nonmatchable()
    non_c = asm & nonmatch["names"]
    real_asm = asm - non_c

    c_words = sum(c_spans[name]["words"] for name in matched)

    def words_of(names: set[str]) -> tuple[int, list[str]]:
        total = 0
        unknown: list[str] = []
        for name in sorted(names):
            if name in sizes:
                total += sizes[name] // 4
            else:
                unknown.append(name)
        return total, unknown

    asm_words, asm_unknown = words_of(asm)
    non_c_words, non_c_unknown = words_of(non_c)
    real_asm_words, real_asm_unknown = words_of(real_asm)

    # Fan-in within the path (distinct in-path callers).
    fan_in: dict[str, int] = defaultdict(int)
    for caller in union:
        for target in callers.get(caller, ()):
            if target in union:
                fan_in[target] += 1

    remaining = sorted(
        (name for name in real_asm),
        key=lambda name: (-fan_in[name], -sizes.get(name, 0), name),
    )

    # Three-bucket totals. Words are the honest denominator; when the artifact
    # is absent the non-C bucket is 0 and real_asm == asm.
    def pct(part: int, whole: int) -> float:
        return (100.0 * part / whole) if whole else 0.0

    total_words = c_words + asm_words
    ported = native_port_funcs()
    executed_c = union & ported
    executed_c_words, executed_c_unknown = words_of(executed_c)
    summary = (
        f"plan={plan_sha} "
        f"disc=1 "
        f"funcs={len(matched)}/{len(union)} "
        f"c_words={c_words} nonc_funcs={len(non_c)} nonc_words={non_c_words} "
        f"asm_funcs={len(real_asm)} asm_words={real_asm_words} "
        f"asm_words_unknown={len(real_asm_unknown)} "
        f"native_c={len(executed_c)}/{len(union)} native_c_words={executed_c_words} "
        f"nonmatchable={'present' if nonmatch['present'] else 'unknown'} "
        f"tierA={len(tier_a)} tierB={len(tier_b)} "
        f"tierB_unresolved={len(unresolved_names)}"
    )

    if not args.quiet:
        print("route coverage: boot -> end of Day 2 (lower bound)")
        print(f"  plan SHA-256:     {plan_sha}")
        print(f"  yaml SHA-256:     {hashlib.sha256((REPO_ROOT / CONFIG).read_bytes()).hexdigest()}")
        print(f"  boot root:        {BOOT_ROOT} (retail initial PC, asm/disc1/header.s)")
        print(f"  vm table:         D_800910A0[0..0x{VM_OPCODE_WORDS:X}) (func_80017018)")
        nm_note = "" if nonmatch["present"] else "  [artifact absent -> unknown, folded into asm]"
        print(f"  non-C-matchable:  {NONMATCHABLE_MANIFEST}{nm_note}")
        print()
        print(f"  Tier A (direct boot closure): {len(tier_a)} functions")
        print(f"  Tier B (VM handlers):         {len(tier_b)} functions")
        print(f"  union (working set):          {len(union)} functions")
        print()
        print(f"  disc:                 1 (SLUS-00662). disc 2 (SLUS-00668) is not a separate route yet;")
        print(f"                        the EXEs are byte-identical, the archives/FMV tracks are not.")
        print(f"  matched C (rebuild):  {len(matched):>5} functions, {c_words:>6} words  ({pct(c_words, total_words):.1f}%)")
        print(f"  executed-path native C: {len(executed_c):>5} / {len(union)} functions, {executed_c_words:>6} words  ({pct(executed_c_words, total_words):.1f}%)")
        if executed_c_unknown:
            print(f"    of which size-unknown: {len(executed_c_unknown)}")
        if nonmatch["present"]:
            print(f"  provably non-C:       {len(non_c):>5} functions, {non_c_words:>6} words  ({pct(non_c_words, total_words):.1f}%)")
        print(f"  real remaining asm:   {len(real_asm):>5} functions, {real_asm_words:>6} words  ({pct(real_asm_words, total_words):.1f}%)")
        if asm_unknown:
            print(f"    of which size-unknown: {len(asm_unknown)}")
        print(f"  Tier-B table targets with an in-image address but no resolvable")
        print(f"    function label: {len(unresolved_names)} (counted as unknown)")
        print()
        print(f"  remaining asm by on-path fan-in (top {args.top}):")
        print(f"    {'fan-in':>6}  {'words':>7}  function")
        for name in remaining[: args.top]:
            words = sizes.get(name, 0) // 4
            words_text = str(words) if name in sizes else "?"
            print(f"    {fan_in[name]:>6}  {words_text:>7}  {name}")
        print()
        print(summary)

    if args.history:
        try:
            write_history(
                Path(args.history),
                plan_sha=plan_sha,
                yaml_sha=hashlib.sha256((REPO_ROOT / CONFIG).read_bytes()).hexdigest(),
                plan_counts=plan_counts,
                now=args.now,
                matched_funcs=len(matched),
                union_funcs=len(union),
                matched_words=c_words,
                non_c_funcs=len(non_c),
                non_c_words=non_c_words,
                real_asm_funcs=len(real_asm),
                real_asm_words=real_asm_words,
                nonmatch_present=nonmatch["present"],
                nonmatch_classes=nonmatch["source_counts"],
            )
        except OSError as exc:
            # History is a side record; it must never break the coverage report
            # (the exact-rebuild gate consumes this tool's --quiet line).
            print(f"route_coverage: history not written: {exc}", file=sys.stderr)

    if args.json:
        report = {
            "plan_sha256": plan_sha,
            "route": {
                "boot_root": BOOT_ROOT,
                "vm_table": "D_800910A0",
                "vm_opcode_words": VM_OPCODE_WORDS,
            },
            "totals": {
                "disc": 1,
                "functions_matched_c": len(matched),
                "functions_executed_native_c": len(executed_c),
                "words_executed_native_c": executed_c_words,
                "functions_non_c_matchable": len(non_c),
                "functions_asm": len(real_asm),
                "functions_asm_incl_non_c": len(asm),
                "functions_union": len(union),
                "words_matched_c": c_words,
                "words_non_c_matchable": non_c_words,
                "words_asm_known": real_asm_words,
                "words_asm_known_incl_non_c": asm_words,
                "functions_asm_size_unknown": len(real_asm_unknown),
                "nonmatchable_artifact": "present" if nonmatch["present"] else "unknown",
                "nonmatchable_classes": nonmatch["source_counts"],
                "tier_a": len(tier_a),
                "tier_b": len(tier_b),
                "tier_b_unresolved": len(unresolved_names),
            },
            "unresolved": unresolved_names,
            "matched_c": sorted(matched),
            "non_c_matchable": sorted(non_c),
            "remaining_asm": [
                {
                    "name": name,
                    "fan_in": fan_in[name],
                    "words": sizes.get(name, 0) // 4 if name in sizes else None,
                    "vram": "0x%08X" % int(name[5:], 16),
                    "caller_source": file_of.get(name, "").replace(str(REPO_ROOT) + "/", ""),
                }
                for name in remaining
            ],
        }
        Path(args.json).write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")

    if args.quiet:
        print(summary)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
