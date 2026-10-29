#!/usr/bin/env python3
"""dangling_callback_census.py -- EXE code addresses the port cannot dispatch.

A guest code address reaches native code only through the generated registry
(`zz_dispatch.c`, an fnv-0 row for an always-resident EXE function) or an
explicit arm of a hand dispatcher (`case 0x...:` / `== 0x...`).  An address
that some code or data *stores* but neither resolves stops the game when it is
called (menu_callback, PE_EffectCallback and PE_GuestCall all raise a boundary
under the default stop policy).  The boundary-site census does not see these:
the stop is in a generic dispatcher, far from the store.

Sources scanned for EXE function starts (configs/USA/disc1.yaml `c`/`asm` rows):
  literal  -- 0x8001xxxx..0x800Dxxxx hex literals in production pc_port C
              (tests and the registry excluded, comments stripped);
  exe      -- every aligned 32-bit word of the retail EXE image;
  overlay  -- every aligned 32-bit word of each PE.IMG overlay blob.

A literal inside a boundary call (`PE_D_COMP_BOUNDARYn`, `PE_Decomp_Boundary`,
`Bootstrap_*`, `operation_boundary`, `card_frontier`) is reported as
`boundary-arg`: that call is already a boundary site.  A literal after `!=` is a
`compare` (a sentinel test).  Everything else unresolved is `stored`.

No game data is written; the retail files are read-only inputs.
"""
from __future__ import annotations

import argparse
import json
import re
import struct
import sys
from collections import defaultdict
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
MAIN = REPO.parents[1] if REPO.parent.name == "build" else REPO
DEF_EXE = MAIN / "build/extracted/disc1/SLUS_006.62"
DEF_PEIMG = MAIN / "build/extracted/disc1/peimg"

ROW_RE = re.compile(r"\s*\{0x([0-9A-F]{8})u,\s*0x[0-9A-F]+u,\s*0x([0-9A-F]{8})u,")
YAML_RE = re.compile(r"-\s*\[\s*0x[0-9A-Fa-f]+\s*,\s*(c|asm)\s*,\s*func_([0-9A-Fa-f]{8})")
LIT_RE = re.compile(r"0x(80[01][0-9A-Fa-f]{5}|800[2-9A-Da-d][0-9A-Fa-f]{4})u?\b")
ARM_RE = re.compile(r"(?:case\s+0x([0-9A-Fa-f]{8})u?\s*:|==\s*0x([0-9A-Fa-f]{8})u?)")
BOUNDARY_CALL_RE = re.compile(
    r"(PE_D_COMP_BOUNDARY\d|PE_Decomp_Boundary\w*|Bootstrap_\w+|operation_boundary|card_frontier)\s*\(")
NE_RE = re.compile(r"!=\s*$")


def strip_comments(t: str) -> str:
    t = re.sub(r"/\*.*?\*/", lambda m: re.sub(r"[^\n]", " ", m.group(0)), t, flags=re.S)
    return re.sub(r"//[^\n]*", "", t)


def function_starts() -> dict[int, str]:
    out = {}
    for line in (REPO / "configs/USA/disc1.yaml").read_text().splitlines():
        m = YAML_RE.search(line)
        if m:
            out[int(m.group(2), 16)] = m.group(1)
    return out


def registry() -> set[int]:
    path = REPO / "pc_port/game/decomp_ovl/_dispatch/zz_dispatch.c"
    rows = set()
    for line in path.read_text().splitlines():
        m = ROW_RE.match(line)
        if m and m.group(2) == "00000000":
            rows.add(int(m.group(1), 16))
    return rows


def production_sources() -> dict[str, str]:
    out = {}
    for f in sorted((REPO / "pc_port").rglob("*.c")):
        rel = f.relative_to(REPO).as_posix()
        if "/tests/" in rel or rel.endswith("zz_dispatch.c"):
            continue
        out[rel] = strip_comments(f.read_text(errors="replace"))
    return out


def in_boundary_call(text: str, pos: int) -> bool:
    """True when the literal at pos is an argument of a boundary call on the
    same statement (or the value of a `#define func_X(...) <boundary>` line)."""
    start = max(text.rfind(";", 0, pos), text.rfind("{", 0, pos), text.rfind("\n#", 0, pos))
    return bool(BOUNDARY_CALL_RE.search(text, start + 1, pos))


def words(blob: bytes):
    for off in range(0, len(blob) - 3, 4):
        yield off, struct.unpack_from("<I", blob, off)[0]


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--exe", default=str(DEF_EXE), help="retail SLUS_006.62 (read-only)")
    ap.add_argument("--peimg-dir", default=str(DEF_PEIMG), help="extracted PE.IMG overlay blobs")
    ap.add_argument("--json", help="write every finding here")
    args = ap.parse_args()

    starts = function_starts()
    rows = registry()
    srcs = production_sources()
    arms = {int(a or b, 16) for t in srcs.values() for a, b in ARM_RE.findall(t)}
    resolved = rows | arms
    findings = []

    lit = defaultdict(lambda: {"stored": set(), "boundary-arg": set(), "compare": set()})
    for rel, t in srcs.items():
        for m in LIT_RE.finditer(t):
            a = int(m.group(1), 16)
            if a in starts and a not in resolved:
                if NE_RE.search(t, max(0, m.start() - 8), m.start()):
                    kind = "compare"          # a sentinel test, not a store
                elif in_boundary_call(t, m.start()):
                    kind = "boundary-arg"
                else:
                    kind = "stored"
                lit[a][kind].add(rel)
    for a in sorted(lit):
        for kind in ("stored", "boundary-arg", "compare"):
            if lit[a][kind]:
                findings.append({"addr": f"{a:08X}", "source": "literal", "kind": kind,
                                 "where": sorted(lit[a][kind])})

    exe = Path(args.exe)
    if exe.is_file():
        b = exe.read_bytes()
        t_addr, t_size = struct.unpack_from("<II", b, 0x18)
        hits = defaultdict(list)
        for off, w in words(b[0x800:0x800 + t_size]):
            if w in starts and w not in resolved:
                hits[w].append(f"{t_addr + off:08X}")
        for a in sorted(hits):
            findings.append({"addr": f"{a:08X}", "source": "exe", "kind": "stored", "where": hits[a]})
    else:
        print(f"dangling_callback_census: no EXE at {exe}; exe scan skipped", file=sys.stderr)

    peimg = Path(args.peimg_dir)
    blobs = sorted(peimg.glob("*.bin")) if peimg.is_dir() else []
    ohits = defaultdict(set)
    for f in blobs:
        for _off, w in words(f.read_bytes()):
            if w in starts and w not in resolved:
                ohits[w].add(f.stem)
    for a in sorted(ohits):
        findings.append({"addr": f"{a:08X}", "source": "overlay", "kind": "stored",
                         "where": sorted(ohits[a])})

    print(f"dangling_callback_census: {len(starts)} EXE function starts, {len(rows)} registry "
          f"rows, {len(arms)} dispatcher arms, {len(blobs)} overlay blobs")
    for f in findings:
        where = ", ".join(f["where"][:4]) + (" ..." if len(f["where"]) > 4 else "")
        print(f"  {f['addr']} {f['source']:<8} {f['kind']:<13} {starts[int(f['addr'], 16)]:<4} {where}")
    stored = [f for f in findings if f["kind"] == "stored"]
    print(f"dangling_callback_census: {len(stored)} stored unresolved, "
          f"{sum(f['kind'] == 'boundary-arg' for f in findings)} boundary arguments, "
          f"{sum(f['kind'] == 'compare' for f in findings)} comparisons")
    if args.json:
        Path(args.json).write_text(json.dumps(findings, indent=1) + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
