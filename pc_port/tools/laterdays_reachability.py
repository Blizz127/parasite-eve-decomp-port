#!/usr/bin/env python3
"""Untested reachability: the room-transfer targets (field-VM op 0x31) of the
rooms that runs entered, minus the rooms they entered.

usage: laterdays_reachability.py --disc <Disc 1 .bin> [--min-story HEX] [--md OUT]
                                 TABLE.json [TABLE.json ...]

TABLE.json is laterdays_room_table.py --json output. A room counts as entered
when a visit after the run's dev warp has story >= --min-story (default 0x80,
the Day-1 end).

Static and condition-ignoring. Every op-0x31 command with an immediate token in
any module of a room's script is an edge, whatever story, flag or message
guards it. So the one-hop frontier and the closure are UPPER bounds of what a
player can reach from the entered rooms, not proof that a transfer is live at
the stories the runs had. World-map (M0000I, m0000i) destinations come from
the transition overlay's story selector, not from op 0x31, so they are not
edges here. Op-0x31 commands whose token is not immediate are counted per room
as "dynamic" and cannot be followed.

Reads the user's Disc 1 read-only (PE.IMG is byte-identical on Disc 2) and the
extracted EXE's field table; prints room names only (no bytes, no text).
"""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/research"))
sys.path.insert(0, str(ROOT / "pc_port/tools"))
from pe_pst0_scan import parse_field_table, extract_script_from_package, decode_script, decode_packed_name  # noqa: E402
from pe_btl14_m0005i_publish_oracle import read_form1  # noqa: E402

STORIES: dict[str, set] = {}

PE_IMG_LBA = 1013


STORY_SLOT = 0x4A   # persist[74], the story word (0x800A7918)


def story_consts(cmd: dict) -> list[int]:
    """Immediate story constants a command compares with or assigns: op 0x09
    (ALU/compare, story operand mode 2 at arg 2, immediate at arg 3) and op 0x0A
    (assign, story at arg 0, immediate at arg 1)."""
    a, m = cmd["args"], cmd["modes"]
    if cmd["opcode"] == 0x09 and len(a) >= 4 and a[2] == STORY_SLOT and m[2] == 2 and m[3] == 0:
        return [a[3]]
    if cmd["opcode"] == 0x0A and len(a) >= 2 and a[0] == STORY_SLOT and m[0] == 2 and m[1] == 0:
        return [a[1]]
    return []


def load_edges(disc: Path) -> tuple[dict, dict, set]:
    exe = (ROOT / "build/extracted/disc1/SLUS_006.62").read_bytes()
    edges: dict[str, set] = {}
    dynamic: dict[str, int] = {}
    scripted = set()
    global STORIES
    STORIES = {}
    for rec in parse_field_table(exe):
        c0 = rec["meta"] & 0xFF
        c1 = (rec["meta"] >> 8) & 0xFFF
        c2 = rec["meta"] >> 20
        if c0 + c1 + c2 == 0:
            continue
        try:
            package = read_form1(disc, PE_IMG_LBA + rec["start"], c0 + c1 + c2)
        except Exception:
            continue
        ext = extract_script_from_package(package, rec["meta"])
        if ext is None:
            continue
        name = rec["name"]
        scripted.add(name)
        script = decode_script(ext[1])
        for mod in script["modules"]:
            for cmd in mod["commands"]:
                for v in story_consts(cmd):
                    if 0 < v < 0x1000:
                        STORIES.setdefault(name, set()).add(v)
                if cmd["opcode"] != 0x31 or not cmd["args"]:
                    continue
                if cmd["modes"][0] == 0:
                    edges.setdefault(name, set()).add(decode_packed_name(cmd["args"][0]))
                else:
                    dynamic[name] = dynamic.get(name, 0) + 1
    return edges, dynamic, scripted


def entered(tables: list[str], min_story: int) -> dict[str, list[str]]:
    seen: dict[str, list[str]] = {}
    for t in tables:
        for run in json.load(open(t)):
            for v in run["visits"]:
                if v["pre_warp"] or v["story_in"] < min_story or v["story_in"] >= 0x1000:
                    continue
                seen.setdefault(v["room"], [])
                if run["label"] not in seen[v["room"]]:
                    seen[v["room"]].append(run["label"])
    return seen


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("tables", nargs="+")
    ap.add_argument("--disc", required=True)
    ap.add_argument("--min-story", default="80")
    ap.add_argument("--md", default="")
    ap.add_argument("--warp", action="append", default=[],
                    help="room table JSON of warp-sweep runs: rooms entered only there are listed as "
                         "warp-entered (entry evidence), apart from the walk-entered rooms")
    ap.add_argument("--story-hints", action="store_true",
                    help="list each not-entered scripted room with the story constants its script uses")
    a = ap.parse_args()
    edges, dynamic, scripted = load_edges(Path(a.disc))
    seen = entered(a.tables, int(a.min_story, 16))
    reached = set(seen)
    out = [f"rooms with a script: {len(scripted)}; rooms with op-0x31 edges: {len(edges)}; "
           f"edges: {sum(len(v) for v in edges.values())}; rooms with dynamic op-0x31 tokens: {len(dynamic)}",
           f"rooms entered after the Day-1 end (runs given): {len(reached)}"]
    frontier: dict[str, set] = {}
    for r in sorted(reached):
        for t in edges.get(r, ()):
            if t not in reached:
                frontier.setdefault(t, set()).add(r)
    out.append("")
    out.append(f"one-hop frontier (op-0x31 targets of entered rooms, not entered; condition-ignoring): {len(frontier)}")
    out.append("")
    out.append("| target | from entered room(s) |")
    out.append("|---|---|")
    for t in sorted(frontier):
        out.append(f"| {t} | {', '.join(sorted(frontier[t]))} |")
    closure, todo = set(), list(reached)
    while todo:
        r = todo.pop()
        for t in edges.get(r, ()):
            if t not in reached and t not in closure:
                closure.add(t)
                todo.append(t)
    out.append("")
    out.append(f"closure over op-0x31 edges from the entered rooms, not entered (upper bound): {len(closure)}")
    out.append(", ".join(sorted(closure)))
    never = sorted(scripted - reached)
    out.append("")
    out.append(f"scripted rooms no given run entered after the Day-1 end: {len(never)} "
               f"(in the closure: {len(set(never) & closure)}; not in it: {len(set(never) - closure)})")
    out.append(", ".join(never))
    if a.story_hints:
        out.append("")
        out.append("story hints for the not-entered rooms (immediate persist[74] constants the room's script "
                   "compares with or assigns; none = the script never tests the story word):")
        out.append("")
        out.append("| room | in closure | story constants |")
        out.append("|---|---|---|")
        for r in never:
            st = sorted(STORIES.get(r, ()))
            out.append(f"| {r} | {'yes' if r in closure else 'no'} | {' '.join(f'{v:X}' for v in st) or 'none'} |")
    if a.warp:
        warp = set(entered(a.warp, int(a.min_story, 16))) - reached
        untouched = scripted - reached - warp
        order = sorted(scripted, key=lambda n: int(n[1:5]))

        def ranges(members: set) -> str:
            out_r, run = [], []
            for n in order:
                if n in members:
                    run.append(n)
                    continue
                if run:
                    out_r.append(run[0] if len(run) == 1 else f"{run[0]}..{run[-1]} ({len(run)})")
                    run = []
            if run:
                out_r.append(run[0] if len(run) == 1 else f"{run[0]}..{run[-1]} ({len(run)})")
            return ", ".join(out_r)

        out.append("")
        out.append("rooms by evidence class (scripted rooms only; a range runs over consecutive scripted "
                   "rooms of the class, the count in brackets):")
        out.append(f"- walk-entered (a driver or a start leg ran the room's scripts): "
                   f"{len(reached & scripted)}: {ranges(reached & scripted)}")
        out.append(f"- warp-entered only (room-entry evidence from the sweeps): {len(warp & scripted)}: "
                   f"{ranges(warp & scripted)}")
        out.append(f"- never touched by any run past the Day-1 end: {len(untouched)}: {ranges(untouched)}")
    dyn = sorted(r for r in dynamic if r in reached)
    out.append("")
    out.append(f"entered rooms with dynamic (non-immediate) op-0x31 tokens: {', '.join(dyn) or 'none'}")
    text = "\n".join(out)
    print(text)
    if a.md:
        Path(a.md).write_text(text + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
