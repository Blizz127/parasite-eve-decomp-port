#!/usr/bin/env python3
"""Corpus sweep for a maspsx env knob: flag-off identity + knob-on change list.

For every era-toolchain C leaf of the EXE plan and of every configured overlay
plan, the leaf is compiled exactly like ``disc1_build.compile_era`` does
(cpp -> cc1 -> MASPSX_FORCE_ABSOLUTE_SYMBOLS rewrite -> maspsx with the leaf's
own flags), and the cc1 output is run through maspsx three times:

  * OFF  : the working-tree maspsx with ``<KNOB>=0``
  * BASE : ``--baseline`` maspsx.py (e.g. a copy with the patch removed, or the
           git-HEAD file) with ``<KNOB>=0``
  * ON   : the working-tree maspsx with ``<KNOB>=1``

Flag-off identity is OFF == BASE on the raw text (stricter than bytes).  The
knob-on change list compares ON and OFF with comment-only lines/tails removed,
so a change means an instruction-level difference.  A leaf whose own profile
already sets ``<KNOB>=1`` is reported separately (its OFF run is the
counterfactual).

Usage:
  python3 tools/analysis/maspsx_knob_sweep.py --knob MASPSX_DIV_NO_REUSE_NOP \
      --baseline build/fanout/tooling2/base16/maspsx.py [--jobs 12] [--json out]
  --candidate <maspsx.py> replaces the working-tree maspsx in OFF/ON (develop
  on a copy while the live tree is frozen). --default-check also compares the
  candidate with the knob UNSET against the baseline with it unset (use it to
  prove that flipping a knob's default changes no leaf).
Exit status: 1 when any flag-off (or default-check) difference or failure exists.
"""
from __future__ import annotations

import argparse
import concurrent.futures as cf
import json
import os
import subprocess
import sys
import tempfile
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/build"))
import disc1_build  # noqa: E402
import disc1_plan  # noqa: E402
import overlay_targets  # noqa: E402


def era_units(include_overlays: bool) -> list[tuple[str, dict]]:
    plans = [("disc1", disc1_plan.build_plan(ROOT))]
    if include_overlays:
        for ovl in overlay_targets.configured_overlay_ids(ROOT):
            target = overlay_targets.load_target(ovl, ROOT)
            try:
                plans.append((ovl, disc1_plan.build_plan(ROOT, target=target)))
            except disc1_plan.PlanError as exc:
                # another lane may be mid-edit on an overlay config; report
                # it loudly rather than aborting the whole corpus sweep
                print(f"SKIPPED overlay plan {ovl}: {exc}", file=sys.stderr)
    out = []
    for tag, plan in plans:
        for unit in plan["units"]:
            if unit["kind"] == "c" and unit["toolchain"] == "era":
                out.append((tag, unit))
    return out


def normalize(text: bytes) -> list[str]:
    lines = []
    for raw in text.decode(errors="replace").splitlines():
        line = raw.split(" #", 1)[0].rstrip()
        if not line.strip() or line.lstrip().startswith("#"):
            continue
        lines.append(line.strip())
    return lines


def maspsx(script: Path, assembly: Path, unit: dict, extra: dict,
           drop: str | None = None) -> tuple[int, bytes]:
    env = os.environ.copy()
    env.update(unit["environment"])
    env.update(extra)
    if drop:
        env.pop(drop, None)
    command = [
        sys.executable,
        str(script),
        f"--aspsx-version={unit['environment'].get('ERA_ASPSX_VER', disc1_build.ERA_ASPSX_VER_DEFAULT)}",
        disc1_build.maspsx_sdata_flag(list(unit["flags"])),
    ]
    if unit["environment"].get("MASPSX_EXPAND_LI") != "1":
        command.append("--dont-expand-li")
    if unit["environment"].get("MASPSX_EXPAND_DIV") == "1":
        command.append("--expand-div")
    if unit["environment"].get("MASPSX_USE_COMM_SECTION") == "1":
        command.append("--use-comm-section")
    command.append(str(assembly))
    r = subprocess.run(command, capture_output=True, env=env, cwd=ROOT,
                       stdin=subprocess.DEVNULL)
    return r.returncode, r.stdout + (b"\n#STDERR\n" + r.stderr if r.returncode else b"")


def sweep_one(args, tag: str, unit: dict) -> dict:
    name = f"{tag}:{unit['name']}"
    env = os.environ.copy()
    env.update(unit["environment"])
    cpp, cc1 = disc1_build.era_compiler(unit["environment"])
    with tempfile.TemporaryDirectory(prefix="knobsweep-", dir=args.workdir) as tmp:
        tmp = Path(tmp)
        pre, asm = tmp / "x.i", tmp / "x.s"
        with pre.open("wb") as stream:
            subprocess.run([str(ROOT / cpp), str(ROOT / unit["source"])], env=env,
                           cwd=ROOT, stdout=stream, stderr=subprocess.DEVNULL,
                           check=True)
        r = subprocess.run([str(ROOT / cc1), "-quiet", *unit["flags"], str(pre),
                            "-o", str(asm)], env=env, cwd=ROOT, capture_output=True)
        if r.returncode:
            return {"leaf": name, "status": "cc1fail"}
        forced = unit["environment"].get("MASPSX_FORCE_ABSOLUTE_SYMBOLS")
        if forced:
            disc1_build.force_absolute_symbols(asm, forced)
        off = maspsx(args.candidate, asm, unit, {args.knob: "0"})
        base = maspsx(args.baseline, asm, unit, {args.knob: "0"})
        on = maspsx(args.candidate, asm, unit, {args.knob: "1"})
        if args.default_check:
            unset = unit["environment"].get(args.knob)
            if unset is None:
                d_cand = maspsx(args.candidate, asm, unit, {}, drop=args.knob)
                d_base = maspsx(args.baseline, asm, unit, {}, drop=args.knob)
                default_same = d_cand == d_base
            else:
                default_same = True  # profile sets the knob explicitly
    status = "IDENT" if off == base else "DIFF"
    if args.default_check and not default_same:
        status = "DEFAULT_DIFF"
    if off[0] or base[0]:
        status = "MASPSX_FAIL"
    return {
        "leaf": name,
        "status": status,
        "profile": unit["profile"],
        "profile_sets_knob": unit["environment"].get(args.knob) == "1",
        "on_changes": on[0] != off[0] or normalize(on[1]) != normalize(off[1]),
    }


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--knob", required=True)
    ap.add_argument("--baseline", type=Path, required=True,
                    help="maspsx.py driver of the baseline copy")
    ap.add_argument("--candidate", type=Path,
                    default=ROOT / disc1_build.MASPSX,
                    help="maspsx.py under test (default: the working tree)")
    ap.add_argument("--default-check", action="store_true")
    ap.add_argument("--jobs", type=int, default=12)
    ap.add_argument("--no-overlays", action="store_true")
    ap.add_argument("--only", nargs="*", help="restrict to these leaf names")
    ap.add_argument("--workdir", default=os.environ.get("TMPDIR") or None)
    ap.add_argument("--json", type=Path, help="write per-leaf rows here")
    args = ap.parse_args(argv)
    args.baseline = args.baseline.resolve()
    args.candidate = args.candidate.resolve()
    units = era_units(not args.no_overlays)
    if args.only:
        units = [(t, u) for t, u in units if u["name"] in set(args.only)]
    with cf.ThreadPoolExecutor(args.jobs) as ex:
        rows = list(ex.map(lambda tu: sweep_one(args, *tu), units))
    rows.sort(key=lambda r: r["leaf"])
    if args.json:
        args.json.write_text("".join(json.dumps(r) + "\n" for r in rows))
    print(f"{args.knob}: {len(rows)} era leaves, "
          f"{dict(Counter(r['status'] for r in rows))}")
    bad = [r["leaf"] for r in rows if r["status"] != "IDENT"]
    print("flag-off non-identical:", bad or "none")
    changed = [r for r in rows if r.get("on_changes")]
    print(f"knob-on changes ({len(changed)}):",
          ", ".join(f"{r['leaf']}{'*' if r['profile_sets_knob'] else ''}"
                    for r in changed) or "none")
    print("(* = the leaf's own profile already sets the knob)")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
