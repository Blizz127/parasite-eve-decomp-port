#!/usr/bin/env python3
"""Audit pc_port hand ports against their byte-exact matched-C leaves.

For every matched `c` span in `configs/USA/disc1.yaml` whose symbol is already
defined by a hand-written pc_port TU, this compares semantic signatures of the
**matched leaf** (authoritative, byte-exact against retail) and the **hand
port**:

  * return type
  * guest data addresses touched (`D_XXXXXXXX` and raw `0x8xxxxxxx` literals)
  * numeric constants / bit masks
  * callee set

A signature that disagrees is a *candidate divergence*: it may be a real bug
(the matched leaf is authority) or a legitimate host-adaptation difference
(e.g. the hand port reaches a subsystem through a helper).  This tool does not
decide; it ranks and prints evidence so a human inspects the top of the list.

Read-only.  Never writes to `src/`, `pc_port/`, or the manifest.
"""
from __future__ import annotations

import argparse
import collections
import importlib.util
import json
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]

_spec = importlib.util.spec_from_file_location(
    "gen_decomp_ports", REPO_ROOT / "tools/analysis/gen_decomp_ports.py")
_gen = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_gen)  # type: ignore[union-attr]

COMMENT_RE = re.compile(r"/\*.*?\*/|//[^\n]*", re.S)
FUNC_DEF_RE = re.compile(
    r"^[ \t]*(?:static[ \t]+)?"
    r"(?:void|int|unsigned|signed|char|short|long|float|double|[A-Za-z_]\w*_t|"
    r"[A-Za-z_]\w*)[ \t\*]*?\b(func_[0-9A-Fa-f]{8})\s*\(([^;{]*?)\)\s*\{",
    re.MULTILINE)
ADDR_LIT_RE = re.compile(r"\b0x(80[01][0-9A-Fa-f]{5})[uUlL]*\b")
DATA_NAME_RE = re.compile(r"\bD_([0-9A-Fa-f]{8})\b")
NUM_LIT_RE = re.compile(
    r"\b(0[xX][0-9A-Fa-f]+|\d+)[uUlL]*\b")
CALL_RE = re.compile(r"\b(func_[0-9A-Fa-f]{8})\s*\(")


def strip_comments(text: str) -> str:
    return COMMENT_RE.sub(" ", text)


def function_body(text: str, name: str) -> str | None:
    """Brace-matched body of `name` (first definition), comments stripped."""
    stripped = strip_comments(text)
    for m in FUNC_DEF_RE.finditer(stripped):
        if m.group(1) != name:
            continue
        i = stripped.index("{", m.end() - 1)
        depth = 0
        for j in range(i, len(stripped)):
            if stripped[j] == "{":
                depth += 1
            elif stripped[j] == "}":
                depth -= 1
                if depth == 0:
                    return stripped[i + 1:j]
        return None
    return None


def macro_map(text: str) -> dict[str, str]:
    """Object-like `#define NAME value` pairs from a translation unit."""
    out: dict[str, str] = {}
    for m in re.finditer(
            r"^[ \t]*#[ \t]*define[ \t]+([A-Za-z_]\w*)[ \t]+"
            r"([^\n(][^\n]*)$", text, re.MULTILINE):
        val = m.group(2).strip()
        val = re.sub(r"/\*.*?\*/|//.*$", "", val).strip()
        val = re.sub(r"/(?![/*])", "", val)  # doc-comment slashes
        out[m.group(1)] = val
    return out


def expand_macros(text: str, macros: dict[str, str], depth: int = 4) -> str:
    """Best-effort textual expansion so literals match the leaf's symbols."""
    for _ in range(depth):
        changed = False
        def repl(m: re.Match) -> str:
            nonlocal changed
            name = m.group(0)
            if name in macros:
                changed = True
                return "(" + macros[name] + ")"
            return name
        text = re.sub(r"\b[A-Za-z_]\w*\b", repl, text)
        if not changed:
            break
    return text


def fold_address_offsets(text: str, depth: int = 6) -> str:
    """Fold `0x80xxxxxx + 0xNN` (and `-`) into one literal.

    Hand ports routinely name a base register (`0x8009CD70u + 0x3CCu`) where
    the matched leaf names the symbol directly; without folding, every such
    pair looks like a missing/extra address."""
    pat = re.compile(
        r"0x(80[01][0-9A-Fa-f]{5})[uUlL]*\s*([+-])\s*0x([0-9A-Fa-f]+)[uUlL]*")
    for _ in range(depth):
        def repl(m: re.Match) -> str:
            base = int(m.group(1), 16)
            off = int(m.group(3), 16)
            val = base + off if m.group(2) == "+" else base - off
            return f"0x{val:08x}u"
        new = pat.sub(repl, text)
        if new == text:
            break
        text = new
    return text


def signature(body: str, macros: dict[str, str] | None = None) -> dict:
    """Normalised semantic signature of a function body."""
    if macros:
        body = expand_macros(body, macros)
    body = fold_address_offsets(body)
    addrs: set[int] = set()
    for m in DATA_NAME_RE.finditer(body):
        addrs.add(int(m.group(1), 16))
    for m in ADDR_LIT_RE.finditer(body):
        addrs.add(int(m.group(1), 16))
    consts: set[int] = set()
    for m in NUM_LIT_RE.finditer(body):
        tok = m.group(1)
        if tok[:2].lower() == "0x":
            tok = tok[2:] or "0"
            consts.add(int(tok, 16))
        else:
            consts.add(int(tok, 10))
    # Drop guest addresses from the constant set; they are compared separately.
    consts -= addrs
    consts = {c for c in consts if c > 0xFF}
    callees = set(CALL_RE.findall(body))
    return {"addrs": addrs, "consts": consts, "callees": callees}


def return_type(text: str, name: str) -> str:
    stripped = strip_comments(text)
    for m in FUNC_DEF_RE.finditer(stripped):
        if m.group(1) != name:
            continue
        head = stripped[m.start():m.start() + 200]
        rt = head[:head.find(name)]
        rt = re.sub(r"\b(static|extern|inline|__\w+)\b", " ", rt)
        return re.sub(r"\s+", " ", rt).strip()
    return "?"


def pc_port_files() -> dict[str, list[Path]]:
    """symbol -> hand-written pc_port files defining it (decomp TUs excluded)."""
    out: dict[str, list[Path]] = collections.defaultdict(list)
    roots = [REPO_ROOT / "pc_port"]
    for root in roots:
        for path in root.rglob("*.c"):
            if "/game/decomp/" in path.as_posix():
                continue
            try:
                text = path.read_text(encoding="utf-8", errors="replace")
            except OSError:
                continue
            for m in FUNC_DEF_RE.finditer(strip_comments(text)):
                out[m.group(1)].append(path)
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--json", metavar="PATH", help="write full report JSON")
    ap.add_argument("--top", type=int, default=40, help="rows to print")
    ap.add_argument("--only", action="append", default=[])
    ap.add_argument("--route-only", action="store_true")
    ap.add_argument("--route-json", metavar="PATH",
                    default="/tmp/route.json",
                    help="route_coverage.py --json output (for --route-only)")
    args = ap.parse_args()

    matched = _gen.parse_matched()
    route = set()
    try:
        route = set(json.loads(Path(args.route_json).read_text())["matched_c"])
    except (OSError, KeyError, json.JSONDecodeError):
        pass
    defs = pc_port_files()

    fan = collections.Counter()
    for path in (REPO_ROOT / "src").rglob("*.c"):
        text = path.read_text(encoding="utf-8", errors="replace")
        for m in CALL_RE.finditer(text):
            fan[m.group(1)] += 1

    rows = []
    for name in sorted(matched):
        paths = defs.get(name)
        if not paths:
            continue
        if args.only and name not in set(args.only):
            continue
        if args.route_only and name not in route:
            continue
        src_path = REPO_ROOT / "src" / f"{name}.c"
        if not src_path.exists():
            continue
        src_text = src_path.read_text(encoding="utf-8", errors="replace")
        src_body = function_body(src_text, name)
        if src_body is None:
            continue
        hand_body = ""
        hand_path = paths[0]
        for p in paths:
            b = function_body(
                p.read_text(encoding="utf-8", errors="replace"), name)
            if b is not None:
                hand_body = b
                hand_path = p
                break
        if not hand_body:
            continue
        a = signature(src_body)
        b = signature(hand_body, macro_map(
            hand_path.read_text(encoding="utf-8", errors="replace")))
        rt_src = return_type(src_text, name)
        rt_hand = return_type(
            hand_path.read_text(encoding="utf-8", errors="replace"), name)
        missing = sorted(a["addrs"] - b["addrs"])
        extra = sorted(b["addrs"] - a["addrs"])
        cmiss = sorted(a["consts"] - b["consts"])
        cextra = sorted(b["consts"] - a["consts"])
        callee_miss = sorted(a["callees"] - b["callees"])
        callee_extra = sorted(b["callees"] - a["callees"])
        # Shape similarity: a hand port that is a *different* (helper-layer)
        # function shows a very different body size, so its address/constant
        # differences are not evidence of a wrong global.  A near-same-size
        # body with a specific missing address/constant is.
        ls, lh = len(src_body), len(hand_body)
        ratio = min(ls, lh) / max(ls, lh, 1)
        # Semantic signal excludes guest addresses that the hand port simply
        # reaches through a helper; constants and return type are the sharpest.
        score = len(cmiss) * 4 + len(cextra) * 2
        if rt_src != rt_hand:
            score += 12
        score += int((len(missing) + len(extra)) * 3 * ratio)
        rows.append({
            "name": name, "route": name in route, "fan_in": fan[name],
            "words": matched[name]["words"], "hand": str(hand_path.relative_to(REPO_ROOT)),
            "ret_src": rt_src, "ret_hand": rt_hand,
            "shape": round(ratio, 2),
            "addr_missing": [f"{x:08X}" for x in missing],
            "addr_extra": [f"{x:08X}" for x in extra],
            "const_missing": cmiss, "const_extra": cextra,
            "callee_missing": callee_miss, "callee_extra": callee_extra,
            "score": score,
        })

    rows.sort(key=lambda r: (-r["route"], -r["score"], -r["fan_in"], -r["words"]))
    if args.json:
        Path(args.json).write_text(
            json.dumps(rows, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    clean = [r for r in rows if r["score"] == 0]
    print(f"conflict audit: {len(rows)} compared, {len(clean)} signature-clean, "
          f"{len(rows) - len(clean)} candidates")
    print(f"  on route: {sum(1 for r in rows if r['route'])}, "
          f"candidates: {sum(1 for r in rows if r['route'] and r['score'])}")
    print()
    for r in rows[: args.top]:
        if r["score"] == 0:
            continue
        tag = "ROUTE" if r["route"] else "     "
        print(f"{tag} score={r['score']:4d} fan={r['fan_in']:3d} w={r['words']:3d} "
              f"{r['name']}  [{r['hand']}]")
        if r["ret_src"] != r["ret_hand"]:
            print(f"        RET  src={r['ret_src']!r}  hand={r['ret_hand']!r}")
        if r["addr_missing"]:
            print(f"        addr src-only : {', '.join(r['addr_missing'])}")
        if r["addr_extra"]:
            print(f"        addr hand-only: {', '.join(r['addr_extra'])}")
        if r["const_missing"]:
            print("        const src-only : " +
                  ", ".join(f"{c:x}" for c in r["const_missing"]))
        if r["const_extra"]:
            print("        const hand-only: " +
                  ", ".join(f"{c:x}" for c in r["const_extra"]))
        if r["callee_missing"]:
            print(f"        callee src-only : {', '.join(r['callee_missing'])}")
        if r["callee_extra"]:
            print(f"        callee hand-only: {', '.join(r['callee_extra'])}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
