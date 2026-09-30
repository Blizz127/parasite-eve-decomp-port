#!/usr/bin/env python3
"""pc_port coverage — map every matched `c` span to its pc_port counterpart.

This is the inventory / gap-analysis half of "port what we have decompiled
over".  It answers, from authoritative inputs only:

  * `configs/USA/disc1.yaml`  — the matched-`c` span list (name + file/VMA/words).
  * `src/func_*.c`            — the verified decompiled body of each leaf: its
                                outgoing `func_*` calls and the data symbols it
                                declares (`extern`), i.e. its dependency surface.
  * `pc_port/**/*.c`          — which `func_*` symbols the native port defines,
                                and where.

Every matched leaf is classified into exactly one bucket:

  absent          no pc_port definition anywhere and no decomp-derived shadow
  stub            every pc_port definition body of the leaf contains a
                  bootstrap/stub marker (the leaf's OWN body, not merely a
                  file that also holds a boundary stub for some other callee)
  hand-translated a real pc_port definition written independently of `src/`
  ported-from-decomp
                  a pc_port definition that carries a decomp provenance header
                  (`pc_port/game/decomp/*_port.c`), i.e. derived from the
                  verified matched leaf

`ported-from-decomp` is detected structurally: the generated port TUs live in
`pc_port/game/decomp/` and carry a `decomp-source:` provenance line naming the
matched leaf's VMA/span.  Nothing here invents classifications; an unknown
`asm`-only symbol is reported as absent, never guessed.

Output is stable/diffable (sorted sections, one summary line); `--json` emits a
machine-readable report.  Re-run it as matching advances — it re-reads the YAML
and `src/` every time.

Exit status is 0 for a successful report (low coverage is not an error).
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
CONFIG = Path("configs/USA/disc1.yaml")
SRC_GLOB = "src/func_*.c"
PORT_GLOB = "pc_port/**/*.c"
# Headers can carry `static inline` definitions (e.g. psx_compat.h); they are
# scanned for definition BODIES only.
PORT_HEADER_GLOB = "pc_port/**/*.h"
DECOMP_PORT_DIR = Path("pc_port/game/decomp")
PORT_SUFFIX = "_port.c"

LOAD_VRAM = 0x80010000
HEADER_BYTES = 0x800

C_SPAN_RE = re.compile(
    r"-\s*\[(0x[0-9A-Fa-f]+),\s*c,\s*(func_[0-9A-Fa-f]{8})\]"
)
EDGE_RE = re.compile(r"^[ \t]*-[ \t]*\[(0x[0-9A-Fa-f]+)", re.MULTILINE)
FUNC_CALL_RE = re.compile(r"\b(func_[0-9A-Fa-f]{8})\s*\(")
FUNC_ANY_RE = re.compile(r"\b(func_[0-9A-Fa-f]{8})\b")
DATA_SYM_RE = re.compile(r"\b(D_[0-9A-Fa-f]{7,8})\b")
EXTERN_RE = re.compile(r"^\s*extern\s+(.+?);", re.MULTILINE)
# A generated decomp port states its provenance as a line comment.
PROVENANCE_RE = re.compile(r"decomp-source:\s*(func_[0-9A-Fa-f]{8})")
# A real pc_port definition of a func_ symbol at file scope.
DEF_RE = re.compile(
    r"^[A-Za-z_][A-Za-z0-9_ \t\*]*?\b(func_[0-9A-Fa-f]{8})\s*\(", re.MULTILINE
)
STUB_MARKERS = ("BOOTSTRAP_RET", "Bootstrap_ReturnVoid", "Bootstrap_ReturnInt")


def vram_of(file_offset: int) -> int:
    return LOAD_VRAM + file_offset - HEADER_BYTES


def parse_matched() -> list[dict]:
    cfg = (REPO_ROOT / CONFIG).read_text(encoding="utf-8")
    offsets = [int(m.group(1), 16) for m in EDGE_RE.finditer(cfg)]
    leaves: list[dict] = []
    for m in C_SPAN_RE.finditer(cfg):
        start = int(m.group(1), 16)
        end = next((o for o in offsets if o > start), None)
        if end is None:
            raise SystemExit(f"pc_port_coverage: cannot resolve end of {m.group(2)}")
        leaves.append(
            {
                "name": m.group(2),
                "file_offset": start,
                "file_size": end - start,
                "vram": vram_of(start),
                "words": (end - start) // 4,
            }
        )
    names = [leaf["name"] for leaf in leaves]
    if len(names) != len(set(names)):
        dupes = sorted({n for n in names if names.count(n) > 1})
        raise SystemExit(f"pc_port_coverage: duplicate c spans: {dupes}")
    return sorted(leaves, key=lambda leaf: leaf["name"])


def parse_src() -> dict[str, dict]:
    """name -> {callees, globals, lines, path}."""
    out: dict[str, dict] = {}
    for path in sorted(REPO_ROOT.glob(SRC_GLOB)):
        name = path.stem
        text = path.read_text(encoding="utf-8", errors="replace")
        callees = sorted(set(FUNC_CALL_RE.findall(text)) - {name})
        globals_ = sorted(set(DATA_SYM_RE.findall(text)))
        out[name] = {
            "callees": callees,
            "globals": globals_,
            "lines": text.count("\n") + 1,
            "path": str(path.relative_to(REPO_ROOT)),
        }
    return out


def _skip_ws_comments(text: str, i: int) -> int:
    n = len(text)
    while i < n:
        if text[i].isspace():
            i += 1
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            i = n if j < 0 else j + 2
        elif text.startswith("//", i):
            j = text.find("\n", i)
            i = n if j < 0 else j + 1
        else:
            break
    return i


def _match_close(text: str, i: int, op: str, cl: str) -> int | None:
    """`text[i] == op`; return the index just past its matching `cl`.

    Comments and string/char literals are skipped so a `}` inside a
    `"..."` or `/* ... */` does not end the body early.
    """
    depth = 0
    n = len(text)
    while i < n:
        c = text[i]
        if text.startswith("/*", i):
            j = text.find("*/", i + 2)
            i = n if j < 0 else j + 2
            continue
        if text.startswith("//", i):
            j = text.find("\n", i)
            i = n if j < 0 else j + 1
            continue
        if c in "\"'":
            i += 1
            while i < n and text[i] != c:
                i += 2 if text[i] == "\\" else 1
            i += 1
            continue
        if c == op:
            depth += 1
        elif c == cl:
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    return None


def definition_bodies(text: str) -> list[tuple[str, str]]:
    """Return [(func name, body text)] for every file-scope definition.

    A `DEF_RE` hit whose parameter list is followed by `{` is a definition;
    one followed by `;` (a prototype / `extern` declaration) is not.
    """
    out: list[tuple[str, str]] = []
    for m in DEF_RE.finditer(text):
        close = _match_close(text, m.end() - 1, "(", ")")
        if close is None:
            continue
        k = _skip_ws_comments(text, close)
        if k >= len(text) or text[k] != "{":
            continue
        end = _match_close(text, k, "{", "}")
        if end is None:
            continue
        out.append((m.group(1), text[k:end]))
    return out


def is_stub_body(body: str) -> bool:
    return any(marker in body for marker in STUB_MARKERS)


def parse_ports() -> tuple[dict[str, list[str]], dict[str, str], dict[str, bool]]:
    """Return (definitions, provenance, stub_only).

    definitions: func name -> sorted list of pc_port source files (`.c`, or a
                 header with a `static inline` body) that DEFINE it — a
                 prototype or `extern` declaration is not a definition
    provenance:  func name -> matched-leaf name a decomp port claims to derive
                 from (only for files under pc_port/game/decomp/)
    stub_only:   func name -> True when EVERY definition body of that name
                 contains a stub marker (judged per body, not per file)
    """
    definitions: dict[str, set[str]] = {}
    provenance: dict[str, str] = {}
    any_real: dict[str, bool] = {}
    paths = sorted(REPO_ROOT.glob(PORT_GLOB)) + sorted(REPO_ROOT.glob(PORT_HEADER_GLOB))
    for path in paths:
        if "build" in path.relative_to(REPO_ROOT).parts:
            continue
        # Overlay TUs define `<ovl>__func_X` through a rename macro; they are
        # counted by --overlays, never as plain EXE definitions.
        if "decomp_ovl" in path.parts:
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        rel = str(path.relative_to(REPO_ROOT))
        for name, body in definition_bodies(text):
            definitions.setdefault(name, set()).add(rel)
            any_real[name] = any_real.get(name, False) or not is_stub_body(body)
        is_decomp_port = (REPO_ROOT / DECOMP_PORT_DIR) in path.parents
        if is_decomp_port:
            for m in PROVENANCE_RE.finditer(text):
                provenance.setdefault(m.group(1), m.group(1))
    return (
        {k: sorted(v) for k, v in definitions.items()},
        provenance,
        {k: not v for k, v in any_real.items()},
    )


def classify(
    leaf: dict, src: dict | None, definitions: dict, provenance: dict, stub_only: dict
) -> tuple[str, list[str], str]:
    """Return (bucket, defining_files, note)."""
    name = leaf["name"]
    files = definitions.get(name, [])
    if provenance.get(name) == name:
        return "ported-from-decomp", files, "decomp-derived shadow TU"
    if not files:
        return "absent", files, "no pc_port definition"
    if stub_only.get(name, False):
        return "stub", files, "every definition body is a bootstrap/stub provider"
    return "hand-translated", files, "independent pc_port implementation"


# ── overlay coverage (step 2 of the overlay-port mechanism) ──────────────
OVL_CONFIG_DIR = Path("configs/USA/overlays")
OVL_PORT_DIR = Path("pc_port/game/decomp_ovl")
OVL_SPAN_RE = re.compile(
    r"^\s*-\s*\[\s*(0x[0-9A-Fa-f]+)\s*(?:,\s*(\w+)\s*(?:,\s*(\w+))?)?\s*\]",
    re.MULTILINE)
OVL_PROVENANCE_RE = re.compile(r"decomp-source-overlay:\s*(\w+)/(func_[0-9A-Fa-f]{8})")


def parse_overlay_leaves() -> list[dict]:
    """Every matched `c` leaf of every overlay config: {overlay, name, words}."""
    out: list[dict] = []
    for path in sorted((REPO_ROOT / OVL_CONFIG_DIR).glob("*.yaml")):
        if path.name == "manifest.yaml":
            continue
        text = path.read_text(encoding="utf-8")
        spans = [(int(m.group(1), 16), m.group(2), m.group(3))
                 for m in OVL_SPAN_RE.finditer(text)]
        offsets = sorted({o for o, _, _ in spans})
        for off, kind, name in spans:
            if kind != "c" or not name:
                continue
            end = next((o for o in offsets if o > off), off)
            out.append({"overlay": path.stem, "name": name,
                        "words": (end - off) // 4})
    return out


def overlay_report() -> dict:
    """Classify overlay leaves keyed by (overlay, name).

    ported-from-decomp: a gen_overlay_ports.py TU with that provenance;
    hand-translated / stub: a namespaced `<ovl>__func_X` definition, or a
      plain `func_X` definition when the name exists in exactly ONE overlay;
    ambiguous: the name exists in several overlays (different code at the
      same shared-arena address) and pc_port has only a plain `func_X`
      definition — it cannot be credited to any one overlay;
    absent: nothing.
    """
    leaves = parse_overlay_leaves()
    owners: dict[str, set[str]] = {}
    for leaf in leaves:
        owners.setdefault(leaf["name"], set()).add(leaf["overlay"])
    derived: set[tuple[str, str]] = set()
    for path in sorted((REPO_ROOT / OVL_PORT_DIR).glob("*/*_port.c")):
        text = path.read_text(encoding="utf-8", errors="replace")
        for m in OVL_PROVENANCE_RE.finditer(text):
            derived.add((m.group(1), m.group(2)))
    definitions, _prov, stub_only = parse_ports()
    ns_defs: dict[str, bool] = {}
    for path in sorted(REPO_ROOT.glob(PORT_GLOB)):
        if "build" in path.relative_to(REPO_ROOT).parts or "decomp_ovl" in path.parts:
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        for m in re.finditer(r"^[A-Za-z_][\w \t\*]*?\b(\w+__func_[0-9A-Fa-f]{8})\s*\(",
                             text, re.MULTILINE):
            ns_defs[m.group(1)] = True
    rows = []
    for leaf in leaves:
        ov, name = leaf["overlay"], leaf["name"]
        if (ov, name) in derived:
            bucket = "ported-from-decomp"
        elif f"{ov}__{name}" in ns_defs:
            bucket = "hand-translated"
        elif name in definitions and len(owners[name]) == 1:
            bucket = "stub" if stub_only.get(name, False) else "hand-translated"
        elif name in definitions:
            bucket = "ambiguous"
        else:
            bucket = "absent"
        rows.append({**leaf, "bucket": bucket,
                     "port_files": definitions.get(name, []) if bucket != "ported-from-decomp" else []})
    return {"rows": rows,
            "ambiguous_names": sorted({r["name"] for r in rows if r["bucket"] == "ambiguous"})}


def print_overlay_report(rep: dict, list_bucket: str | None) -> int:
    rows = rep["rows"]
    buckets = ("ported-from-decomp", "hand-translated", "stub", "ambiguous", "absent")
    if list_bucket:
        if list_bucket not in buckets:
            raise SystemExit(f"pc_port_coverage: unknown overlay bucket {list_bucket!r}")
        for r in rows:
            if r["bucket"] == list_bucket:
                print(f"{r['overlay']}/{r['name']}")
        return 0
    total_words = sum(r["words"] for r in rows)
    print("pc_port coverage — matched overlay `c` spans -> native port")
    print(f"  matched overlay leaves: {len(rows)}  ({total_words} words)")
    print("")
    for b in buckets:
        n = sum(1 for r in rows if r["bucket"] == b)
        w = sum(r["words"] for r in rows if r["bucket"] == b)
        print(f"  {b + ':':<19} {n:>4} leaves  {w:>6} words")
    per: dict[str, dict] = {}
    for r in rows:
        d = per.setdefault(r["overlay"], {b: 0 for b in buckets})
        d[r["bucket"]] += 1
    print("")
    for ov in sorted(per):
        d = per[ov]
        if ov.startswith("ovl_") or d["ported-from-decomp"] or d["hand-translated"] or d["ambiguous"]:
            print(f"  {ov:<14} " + " ".join(f"{b.split('-')[0]}={d[b]}" for b in buckets))
    if rep["ambiguous_names"]:
        print("")
        print("  ambiguous plain-named pc_port definitions (name exists in several overlays;"
              " not credited — annotate as <ovl>__func_X):")
        for n in rep["ambiguous_names"]:
            ovs = sorted({r["overlay"] for r in rows if r["name"] == n})
            print(f"    {n}: {' '.join(ovs)}")
    counts = {b: sum(1 for r in rows if r["bucket"] == b) for b in buckets}
    print("overlay_summary=" + " ".join(f"{b}={counts[b]}" for b in buckets))
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--json", action="store_true", help="machine-readable output")
    ap.add_argument(
        "--list-bucket",
        metavar="BUCKET",
        help="print the names in one bucket (ported-from-decomp, hand-translated, stub, absent)",
    )
    ap.add_argument(
        "--deps",
        metavar="NAME",
        help="print the dependency surface of one matched leaf",
    )
    ap.add_argument(
        "--overlays",
        action="store_true",
        help="report overlay leaves keyed by (overlay, name) instead of the EXE",
    )
    args = ap.parse_args()

    if args.overlays:
        rep = overlay_report()
        if args.json:
            json.dump(rep, sys.stdout, indent=2, sort_keys=True)
            sys.stdout.write("\n")
            return 0
        return print_overlay_report(rep, args.list_bucket)

    leaves = parse_matched()
    src = parse_src()
    definitions, provenance, stub_only = parse_ports()

    report = []
    for leaf in leaves:
        name = leaf["name"]
        body = src.get(name)
        bucket, files, note = classify(leaf, body, definitions, provenance, stub_only)
        report.append(
            {
                **leaf,
                "bucket": bucket,
                "port_files": files,
                "note": note,
                "callees": body["callees"] if body else [],
                "globals": body["globals"] if body else [],
                "src_lines": body["lines"] if body else 0,
                "has_src": body is not None,
            }
        )

    if args.deps:
        entry = next((r for r in report if r["name"] == args.deps), None)
        if entry is None:
            raise SystemExit(f"pc_port_coverage: {args.deps} is not a matched c span")
        print(f"{entry['name']}  vram=0x{entry['vram']:08X}  words={entry['words']}")
        print(f"  bucket:   {entry['bucket']} ({entry['note']})")
        print(f"  src:      {entry['src_lines']} lines")
        print(f"  globals:  {len(entry['globals'])} -> {' '.join(entry['globals'])}")
        print(f"  callees:  {len(entry['callees'])} -> {' '.join(entry['callees'])}")
        return 0

    counts = {b: 0 for b in ("ported-from-decomp", "hand-translated", "stub", "absent")}
    for entry in report:
        counts[entry["bucket"]] += 1

    if args.list_bucket:
        if args.list_bucket not in counts:
            raise SystemExit(
                f"pc_port_coverage: unknown bucket {args.list_bucket!r}; "
                f"expected one of {', '.join(counts)}"
            )
        for entry in report:
            if entry["bucket"] == args.list_bucket:
                print(entry["name"])
        return 0

    if args.json:
        json.dump(
            {
                "matched_leaves": len(report),
                "buckets": counts,
                "leaves": report,
            },
            sys.stdout,
            indent=2,
            sort_keys=True,
        )
        sys.stdout.write("\n")
        return 0

    total_words = sum(r["words"] for r in report)
    ported_words = sum(r["words"] for r in report if r["bucket"] == "ported-from-decomp")
    print("pc_port coverage — matched `c` spans -> native port")
    print(f"  matched c leaves: {len(report)}  ({total_words} words)")
    print("")
    for bucket in ("ported-from-decomp", "hand-translated", "stub", "absent"):
        words = sum(r["words"] for r in report if r["bucket"] == bucket)
        print(
            f"  {bucket + ':':<19} {counts[bucket]:>4} leaves  {words:>6} words"
        )
    print("")
    print(
        f"  decomp-derived coverage: {ported_words}/{total_words} words "
        f"({100.0 * ported_words / total_words:.1f}%)"
    )
    print(
        "summary="
        f"matched={len(report)} "
        f"from_decomp={counts['ported-from-decomp']} "
        f"hand={counts['hand-translated']} "
        f"stub={counts['stub']} "
        f"absent={counts['absent']}"
    )

    if os.environ.get("PC_PORT_COVERAGE_VERBOSE"):
        print("")
        print("  leaves not derived from their matched source:")
        for entry in report:
            if entry["bucket"] != "ported-from-decomp":
                print(f"    {entry['bucket']:<18} {entry['name']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
