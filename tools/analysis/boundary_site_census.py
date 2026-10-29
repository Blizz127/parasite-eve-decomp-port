#!/usr/bin/env python3
"""Census of the boundary / bootstrap-stub call sites in production pc_port code.

Every site that ends in a CPU_BOUNDARY stop when reached is listed with the
name it reports and whether that name has host C now (a stale boundary).
Generated TUs (pc_port/game/decomp, pc_port/game/decomp_ovl) and tests are
excluded; generated-TU boundaries are reported separately (--generated).

Sites: calls of Bootstrap_Return*, BOOTSTRAP_RET*, PE_Decomp_Boundary*,
PE_D_COMP_BOUNDARY*, calls of file-local wrappers whose body forwards a
`const char *` parameter to one of those (`menu_boundary("name")`), and
`#define func_X(...) PE_D_COMP_BOUNDARYn("func_X", ...)` bindings (the
generator's form for a callee with no host C; one site per binding).

    python3 tools/analysis/boundary_site_census.py [--json OUT] [--generated]

Read-only.  The "has host C" test uses pc_port_coverage's definition parser:
a name counts as defined when some non-test pc_port file defines it with a
body that is not itself only a bootstrap stub.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import pc_port_coverage as cov  # noqa: E402

ROOT = cov.REPO_ROOT
MACROS = (r"Bootstrap_Return[A-Za-z0-9_]*|BOOTSTRAP_RET[A-Za-z0-9_]*|"
          r"PE_Decomp_Boundary(?:From|Name)?|PE_D_COMP_BOUNDARY[0-9n]?")
MAC_RE = re.compile(r"\b(" + MACROS + r")\s*\(")
DEFINE_BOUNDARY_RE = re.compile(r"#define\s+(func_[0-9A-Fa-f]{8})\s*\(\s*\.\.\.\s*\)\s+(" + MACROS
                                + r")\s*\(\s*\"([^\"]*)\"")
WRAPPER_RE = re.compile(r"static\s+(?:inline\s+)?\w[\w\s\*]*?\b(\w+)\s*\(\s*const\s+char\s*\*\s*(\w+)[^)]*\)\s*\{")
GEN_DIRS = ("pc_port/game/decomp/", "pc_port/game/decomp_ovl/")


def sources(generated: bool):
    for p in sorted(ROOT.glob("pc_port/**/*.c")) + sorted(ROOT.glob("pc_port/**/*.h")):
        rel = p.relative_to(ROOT).as_posix()
        if "/build/" in "/" + rel or "/tests/" in "/" + rel:
            continue
        if any(rel.startswith(d) for d in GEN_DIRS) != generated:
            continue
        yield p, rel


def host_definitions() -> dict[str, list[str]]:
    out: dict[str, set[str]] = {}
    for p, rel in list(sources(False)) + list(sources(True)):
        for name, body in cov.definition_bodies(p.read_text(errors="replace")):
            if not cov.is_stub_body(body) or len(body) > 400:
                out.setdefault(name, set()).add(rel)
    return {k: sorted(v) for k, v in out.items()}


def wrappers(text: str) -> set[str]:
    """File-local helpers that hand their const char * parameter to a
    boundary macro: their call sites are boundary sites too."""
    found = set()
    for m in WRAPPER_RE.finditer(text):
        name, param = m.group(1), m.group(2)
        end = cov._match_close(text, m.end() - 1, "{", "}")
        body = text[m.end():end] if end else ""
        if re.search(r"\b(" + MACROS + r")\s*\(\s*" + re.escape(param) + r"\b", body):
            found.add(name)
    return found


def census(generated: bool) -> list[dict]:
    defs = host_definitions()
    rows = []
    for p, rel in sources(generated):
        text = p.read_text(errors="replace")
        lines = text.splitlines()
        wrap = wrappers(text)
        pat = re.compile(r"\b(" + MACROS + ("|" + "|".join(map(re.escape, sorted(wrap))) if wrap else "")
                         + r")\s*\(")
        for i, ln in enumerate(lines):
            s = ln.lstrip()
            # `#define func_X(...) PE_D_COMP_BOUNDARYn("func_X", ...)` binds every
            # call of func_X in this file to a boundary (the generator's form
            # for a callee with no host C): one site per binding.
            dm = DEFINE_BOUNDARY_RE.match(s)
            if dm:
                fm = re.match(r"(?:[\w]+:)?(func_[0-9A-Fa-f]{8})$", dm.group(3))
                fn = "func_" + fm.group(1)[5:].upper() if fm else ""
                rows.append({"file": rel, "line": i + 1, "macro": "#define " + dm.group(2),
                             "name": dm.group(3), "func": fn,
                             "host_c": defs.get(fn, []) if fn else []})
                continue
            if s.startswith(("#define", "//", "*", "/*")):
                continue
            for m in pat.finditer(ln):
                if m.group(1) in wrap and re.match(r"\s*static\b", ln):
                    continue           # the wrapper's own definition line
                seg = " ".join(lines[i:i + 3])
                arg = re.search(r"\(\s*\"([^\"]*)\"", seg[seg.find(m.group(0)):])
                name = arg.group(1) if arg else ""
                fm = re.match(r"(?:[\w]+:)?(func_[0-9A-Fa-f]{8})$", name)
                fn = "func_" + fm.group(1)[5:].upper() if fm else ""
                rows.append({"file": rel, "line": i + 1, "macro": m.group(1), "name": name,
                             "func": fn, "host_c": defs.get(fn, []) if fn else []})
    return rows


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--json", type=Path)
    ap.add_argument("--generated", action="store_true",
                    help="census the generated TUs' boundary macros instead")
    ns = ap.parse_args()
    rows = census(ns.generated)
    named = [r for r in rows if r["func"]]
    stale = [r for r in named if r["host_c"]]
    print(f"boundary_site_census: {len(rows)} sites in {'generated' if ns.generated else 'production'} "
          f"code; {len(named)} name a func_ exactly; {len(stale)} of those have host C")
    for r in stale:
        print(f"  {r['file']}:{r['line']} {r['macro']} {r['name']} -> {', '.join(r['host_c'][:2])}")
    if ns.json:
        ns.json.write_text(json.dumps(rows, indent=1) + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
