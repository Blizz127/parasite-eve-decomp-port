#!/usr/bin/env python3
"""Warp-entry plan for the scripted rooms that no walk or warp run entered
past the Day-1 end (sweep 2026-10-21, helper pe-sweep).

usage: sweep_untouched_plan.py REPORT.md OUT.json

REPORT.md is docs/evidence/laterdays-native-20261021/REPORT.md. The plan is
read from its "Untested reachability" section:
  - the story-hints table: every scripted room no walk entered (269 rows,
    columns room / in closure / story constants; "none" = the script never
    tests the story word);
  - two class lines: "warp-entered only ... : 83" and "never touched by any run
    past the Day-1 end: 186", written as ranges over consecutive scripted rooms.

The ranges are expanded over the 269-room set. Every expansion must match its
printed count, and the classes must be disjoint and cover the 269 rooms, or the
script stops with an error.

Plan rules (heuristic story choice, recorded per row):
  story  = the smallest immediate story constant in the room's script (hint);
           "none" -> 0x08 for m0001i..m0034i (a Day-1 story value the route
           visits), 0x80 for m0035i..m0274i (Day-2 start), 0x2D6 for
           m0275i and up (the Day-6 opening story).
  disc   = 2 when the story is >= 0x218 (the Day-5 later-day runs mount Disc 2
           from 0x218), otherwise 1. Every run also gets Disc 2 as the
           tray disc (--disc2-image), as the later-day runs do.
  frames = 4000 (the Day-6 entry sweep of the same lane).
"""
from __future__ import annotations

import json
import re
import sys

ROW_RE = re.compile(r"^\|\s*(m\d{4}i)\s*\|\s*(yes|no)\s*\|\s*([^|]*?)\s*\|\s*$")
TOKEN_RE = re.compile(r"m(\d{4})i(?:\.\.m(\d{4})i)?(?: \((\d+)\))?")
FRAMES = 4000
DAY5_DISC2_STORY = 0x218


def room_num(name: str) -> int:
    return int(name[1:5])


def parse_hints(lines: list[str]) -> dict[str, list[int]]:
    hdr = next(i for i, l in enumerate(lines) if l.startswith("| room | in closure | story constants |"))
    out: dict[str, list[int]] = {}
    for l in lines[hdr + 2:]:
        if not l.startswith("|"):
            break
        m = ROW_RE.match(l)
        if not m:
            raise SystemExit(f"sweep_untouched_plan: unparsed hints row: {l!r}")
        name, _closure, consts = m.group(1), m.group(2), m.group(3)
        out[name] = [] if consts.strip() == "none" else [int(c, 16) for c in consts.split()]
    return out


def class_members(line: str, universe: list[str]) -> list[str]:
    body = line.split(":", 2)[2] if line.count(":") >= 2 else line.split(":", 1)[1]
    members: list[str] = []
    uni = set(universe)
    for m in TOKEN_RE.finditer(body):
        a = int(m.group(1))
        if m.group(2):
            b = int(m.group(2))
            exp = sorted(r for r in universe if a <= room_num(r) <= b)
            if m.group(3) and len(exp) != int(m.group(3)):
                raise SystemExit(f"sweep_untouched_plan: range m{a:04d}i..m{b:04d}i expands to {len(exp)}, report says {m.group(3)}")
            members.extend(exp)
        else:
            name = f"m{a:04d}i"
            if name not in uni:
                raise SystemExit(f"sweep_untouched_plan: {name} is not in the 269-room set")
            members.append(name)
    return members


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        print(__doc__, file=sys.stderr)
        return 2
    lines = open(argv[0], encoding="utf-8").read().splitlines()
    hints = parse_hints(lines)
    universe = sorted(hints)
    if len(universe) != 269:
        raise SystemExit(f"sweep_untouched_plan: hints table has {len(universe)} rows, want 269")
    warp_line = next(l for l in lines if l.startswith("- warp-entered only"))
    never_line = next(l for l in lines if l.startswith("- never touched by any run past the Day-1 end"))
    warp = class_members(warp_line, universe)
    never = class_members(never_line, universe)
    if len(warp) != 83 or len(set(warp)) != 83:
        raise SystemExit(f"sweep_untouched_plan: warp-only class has {len(warp)} (distinct {len(set(warp))}), want 83")
    if len(never) != 186 or len(set(never)) != 186:
        raise SystemExit(f"sweep_untouched_plan: never-touched class has {len(never)} (distinct {len(set(never))}), want 186")
    if set(warp) & set(never) or (set(warp) | set(never)) != set(universe):
        raise SystemExit("sweep_untouched_plan: classes overlap or do not cover the 269 rooms")

    rows = []
    for cls, members in (("never", never), ("warp-only", warp)):
        for room in sorted(members):
            consts = hints[room]
            if consts:
                story, src = min(consts), "hint-min"
            elif room_num(room) <= 34:
                story, src = 0x08, "default-day1 (no hint)"
            elif room_num(room) < 275:
                story, src = 0x80, "default-day2 (no hint)"
            else:
                story, src = 0x2D6, "default-day6 (no hint)"
            disc = 2 if story >= DAY5_DISC2_STORY else 1
            rows.append({
                "room": room,
                "class": cls,
                "hints": [f"{c:X}" for c in consts],
                "story": f"{story:X}",
                "story_source": src,
                "disc": disc,
                "frames": FRAMES,
            })
    with open(argv[1], "x", encoding="utf-8") as f:
        json.dump({"source": argv[0], "classes": {"never": len(never), "warp-only": len(warp)},
                   "rooms": rows}, f, indent=1)
        f.write("\n")
    print(f"plan: never={len(never)} warp-only={len(warp)} universe={len(universe)} -> {argv[1]}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
