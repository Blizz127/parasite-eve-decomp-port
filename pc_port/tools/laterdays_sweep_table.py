#!/usr/bin/env python3
"""Table for a warp sweep (room-entry evidence only).

usage: laterdays_sweep_table.py SWEEP_DIR [SWEEP_DIR ...]
       laterdays_sweep_table.py --runs 'RUNS_GLOB'      (one log per run dir, e.g. 'runs/g-*')

SWEEP_DIR is laterdays_warp_sweep.sh output: one <room>-<story>/run.log per
room. With --runs, every matching dir is <prefix>-<room>-<story> with
logs/*.log (day5_route_regress.py --room/--story start legs).

Per target room: frames spent in it after the warp, battle entries there,
whether a battle reached mode 3 (death), the next room the game entered (story
in brackets; m0438i is Game Over, the title token resets), the rooms entered
after it, whether the disc-2 mount line is present, and the marker lines
(laterdays_room_table.py's boundary/stop/shim/stub/not-ported/unresolved/fatal
set) over every post-warp visit with story >= 0x80. No game bytes are read.
"""
from __future__ import annotations

import argparse
import glob
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from laterdays_room_table import BAD, parse  # noqa: E402


def row(room: str, story: str, path: str) -> tuple[str, int]:
    r = parse(f"{room}-{story}", path)
    allv = [v for v in r["visits"] if not v["pre_warp"]]
    lines = open(path, errors="replace").read().splitlines()
    mounted = "yes" if any("disc 2 in the drive (mounted)" in l for l in lines) else "no"
    ti = next((i for i, v in enumerate(allv) if v["room"] == room), None)
    if ti is None:
        return f"| {room} | {story} | 0 | - | - | (target not entered) | - | {mounted} | - |", 0
    v = allv[ti]
    tok = f"{v['token']:08X}"
    death = any(l.startswith("BATTLE_EDGE") and f"token={tok}" in l and l.endswith("mode=3") for l in lines)
    nxt = f"{allv[ti + 1]['room']} ({allv[ti + 1]['story_in']:X})" if ti + 1 < len(allv) else "(run end)"
    after = sorted({x["room"] for x in allv[ti + 1:] if 0x80 <= x["story_in"] < 0x1000})
    post = [x for x in allv if 0x80 <= x["story_in"] < 0x1000]
    bad = sum(x["counts"][k] for x in post for k in BAD)
    det = "; ".join(d["text"][:120] for x in post for d in x["details"])[:300]
    return (f"| {room} | {story} | {v['frames']} | {v['battles']} | {'yes' if death else 'no'} | {nxt} | "
            f"{', '.join(after) or '-'} | {mounted} | {bad}{' ' + det if det else ''} |"), bad


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("dirs", nargs="*")
    ap.add_argument("--runs", default="")
    a = ap.parse_args()
    items = []
    for d in a.dirs:
        for sub in sorted(os.listdir(d)):
            p = os.path.join(d, sub, "run.log")
            if os.path.isfile(p) and "-" in sub:
                room, story = sub.rsplit("-", 1)
                items.append((room, story, p))
    if a.runs:
        for d in sorted(glob.glob(a.runs)):
            logs = glob.glob(os.path.join(d, "logs", "*.log"))
            name = os.path.basename(d).split("-", 1)[1] if "-" in os.path.basename(d) else os.path.basename(d)
            if logs and "-" in name:
                room, story = name.rsplit("-", 1)
                items.append((room, story, logs[0]))
    print("| target | warp story | frames in target | battles there | battle death (mode 3) | then | "
          "rooms entered after | disc 2 mounted | markers |")
    print("|---|---|--:|--:|---|---|---|---|---|")
    total = 0
    for room, story, p in items:
        line, bad = row(room, story, p)
        total += bad
        print(line)
    print(f"\nrooms: {len(items)}; marker lines over all post-warp visits: {total}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
