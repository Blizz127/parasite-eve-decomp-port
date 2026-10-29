#!/usr/bin/env python3
"""Per-room executed-native table from port run logs (harness evidence only).

usage: laterdays_room_table.py [--min-story HEX] [--md OUT] [--json OUT] LABEL=LOG [LABEL=LOG ...]

Each log is the port's stderr (the day*_route_regress.py drivers write it as
<log-dir>/<prefix>-<segment>.log). It is split at its "[ROUTE] change" lines
(frame, token = D_8009D280, story = persist[74]); consecutive changes with the
same token are one room visit (story steps inside the room). Every marker line
is assigned to the visit it falls in:

  boundary    CPU_BOUNDARY            a guest call with no host code (REFUSED = stopped)
  stop        [STOP]                  a native-only stop
  shim        [DAY1_SHIM]             a substitution or skip instead of retail code
  stub        STUB: (not HOST_ADAPTED) a bootstrap stub (Bootstrap_ReturnVoid and kin)
  adapted     STUB:HOST_ADAPTED       a named host adaptation, listed but not counted as
                                      non-native: the route-pad skip-movie harness, the
                                      movie host stream, main's scratchpad stack handoff
                                      (func_8001220C_port.c: the overlay call runs natively
                                      without the PS1 stack switch)
  notported   not ported
  unresolved  unresolved
  fatal       FATAL
  stop_reason stop_reason=            the host's end-of-run line (frame-limit = clean end)

Also per visit: battles (BATTLE_EDGE d1a0&802 going 0 -> nonzero) and harness
aid lines (BATTLE_SHIM, M34_SHIM_*, [D3HP], [DNHP], [ROUTE_SHIM], [CHEATS], [ENH...):
route-pilot and battle-aid traces, which are harness input, not game code.

This binary has no MIPS interpreter and no CPU emulation (boundary policy
stop), so a visit with 0 boundary/stop/shim/stub/notported/unresolved/fatal
lines ran only host code on the paths that visit exercised. That is a claim
about the paths exercised, not about the whole room. Rows before the run's
dev warp ([WARP] env transfer) are the boot / New Game prefix.

Visits with story below --min-story (default 0x80, the Day-1 end) are listed
only in the totals. Room names come from the token's packed name (5 bits per
character, charset at EXE 0x800930B4; plat_cheats.c pe_plat_cheats_room_token).
No game bytes are read or printed.
"""
from __future__ import annotations

import argparse
import json
import re
import sys

CHARSET = "0123456789abcdefghiklmnoprstuvwy"
RE_CHANGE = re.compile(r"^\[ROUTE\] change frame=(\d+) token=([0-9A-Fa-f]{8}) story=([0-9A-Fa-f]{8})")
RE_FRAME = re.compile(r"\bframe=(\d+)")
RE_WARP = re.compile(r"^\[WARP\] env transfer [0-9A-F]{8} -> ([0-9A-F]{8}) story=([0-9A-F]{8}) frame=(\d+)")
RE_PC = re.compile(r"\bpc=(0x[0-9A-Fa-f]+|[0-9A-Fa-f]{8})")
RE_RA = re.compile(r"\bra=(\S+)")
RE_FUNC = re.compile(r"\bfunc_([0-9A-Fa-f]{8})")
RE_BEDGE = re.compile(r"^BATTLE_EDGE \d+ token=[0-9A-Fa-f]{8} d1a0&802=(\d+)")
AID_PREFIXES = ("BATTLE_SHIM", "M34_SHIM", "[D3HP]", "[DNHP]", "[ROUTE_SHIM]", "[CHEATS]", "[ENH")
MARKERS = [
    ("boundary", lambda l: "CPU_BOUNDARY" in l),
    ("stop", lambda l: "[STOP]" in l),
    ("shim", lambda l: "[DAY1_SHIM]" in l),
    ("stub", lambda l: "STUB:" in l and "STUB:HOST_ADAPTED" not in l),
    ("adapted", lambda l: "STUB:HOST_ADAPTED" in l),
    ("notported", lambda l: "not ported" in l),
    ("unresolved", lambda l: "unresolved" in l),
    ("fatal", lambda l: "FATAL" in l),
    ("stop_reason", lambda l: "stop_reason=" in l),
]
BAD = ("boundary", "stop", "shim", "stub", "notported", "unresolved", "fatal")


def room_name(token: int) -> str:
    if token == 0:
        return "-"
    name = "".join(CHARSET[(token >> s) & 0x1F] for s in (27, 22, 17, 12, 7, 2))
    return name if (token & 0x3) == 0 else f"{name}?"


def day_of(story: int) -> str:
    """Day by story, from the day hops the drivers and runs show: Day 2 from 0x80
    (m0351i hop), 3 from 0x140 (m0092i entry after the m0351i hop at 0x138), 4
    from 0x1B8 (m0098i after the m0352i hop at 0x1B0), 5 from 0x220 (world map
    after the m0352i hop at 0x218), 6 from 0x2D6 (m0425i after the m0352i hop at
    0x2D3, run f4 of the 2026-10-21 report). The hop room itself counts to the
    day it ends."""
    if story >= 0x1000:
        return "boot"
    for lo, d in ((0x2D6, "6"), (0x220, "5"), (0x1B8, "4"), (0x140, "3"), (0x80, "2")):
        if story >= lo:
            return d
    return "1"


def parse(label: str, path: str) -> dict:
    visits: list[dict] = []
    cur = None
    warp_frame = None
    last_frame = 0
    end = {"stop_reason": None, "last_line": ""}
    in_battle = False
    with open(path, errors="replace") as f:
        for n, raw in enumerate(f, 1):
            line = raw.rstrip("\n")
            m = RE_FRAME.search(line)
            if m:
                last_frame = max(last_frame, int(m.group(1)))
            w = RE_WARP.search(line)
            if w and warp_frame is None:
                warp_frame = int(w.group(3))
            b = RE_BEDGE.search(line)
            if b:
                now = b.group(1) != "0"
                if now and not in_battle and cur is not None:
                    cur["battles"] += 1
                in_battle = now
            if cur is not None and line.startswith(AID_PREFIXES):
                cur["aid_lines"] += 1
            c = RE_CHANGE.search(line)
            if c:
                fr, tok, st = int(c.group(1)), int(c.group(2), 16), int(c.group(3), 16)
                if cur is not None and cur["token"] == tok:
                    if st != cur["story_out"]:
                        cur["story_steps"].append(st)
                    cur["story_out"] = st
                    continue
                if cur is not None:
                    cur["frame_out"] = fr
                cur = {"label": label, "token": tok, "room": room_name(tok), "story_in": st,
                       "story_out": st, "story_steps": [], "frame_in": fr, "frame_out": None,
                       "line_in": n, "pre_warp": warp_frame is None and fr < 1000,
                       "counts": {k: 0 for k, _ in MARKERS}, "details": [], "battles": 0,
                       "aid_lines": 0}
                visits.append(cur)
                continue
            for k, pred in MARKERS:
                if pred(line):
                    if k == "stop_reason":
                        end["stop_reason"] = line.strip()[:200]
                    if cur is None:
                        continue
                    cur["counts"][k] += 1
                    if k == "adapted":
                        nm = re.search(r"STUB:HOST_ADAPTED\] (\S+)", line)
                        cur.setdefault("adapted", []).append(nm.group(1) if nm else line.strip()[:80])
                    if k in BAD and len(cur["details"]) < 12:
                        d = {"kind": k, "line": n, "text": line.strip()[:220]}
                        pc = RE_PC.search(line)
                        ra = RE_RA.search(line)
                        fn = RE_FUNC.search(line)
                        if pc:
                            d["pc"] = pc.group(1)
                        if ra:
                            d["ra"] = ra.group(1)
                        if fn:
                            d["func"] = "func_" + fn.group(1).upper()
                        cur["details"].append(d)
            if line.strip():
                end["last_line"] = line.strip()[:200]
    if cur is not None and cur["frame_out"] is None:
        cur["frame_out"] = last_frame
        cur["last"] = True
    for v in visits:
        v["frames"] = (v["frame_out"] or 0) - v["frame_in"]
        v["day"] = day_of(v["story_out"])
        v["clean"] = all(v["counts"][k] == 0 for k in BAD)
    return {"label": label, "path": path, "warp_frame": warp_frame, "last_frame": last_frame,
            "end": end, "visits": visits}


def fmt_story(v: dict) -> str:
    s = f"{v['story_in']:X}"
    if v["story_out"] != v["story_in"]:
        s += "->" + "->".join(f"{x:X}" for x in v["story_steps"])
    return s


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("logs", nargs="+", help="LABEL=PATH")
    ap.add_argument("--min-story", default="80")
    ap.add_argument("--md", default="")
    ap.add_argument("--json", default="")
    ap.add_argument("--aggregate", action="store_true",
                    help="one row per distinct room (all visits of all runs) instead of one per visit")
    a = ap.parse_args()
    min_story = int(a.min_story, 16)
    runs = []
    for spec in a.logs:
        if "=" not in spec:
            print(f"bad LABEL=PATH: {spec}", file=sys.stderr)
            return 2
        label, path = spec.split("=", 1)
        runs.append(parse(label, path))
    out = []
    out.append("| run | # | room | token | day | story (in->steps) | frame in | frames | battles | aid lines | markers | executed |")
    out.append("|---|--:|---|---|---|---|--:|--:|--:|--:|---|---|")
    tot = {"visits": 0, "clean": 0, "rooms": set(), "clean_rooms": set(), "bad_rooms": set()}
    stops = []
    for r in runs:
        for i, v in enumerate(r["visits"], 1):
            if v["pre_warp"] or v["story_in"] < min_story or v["story_in"] >= 0x1000:
                continue
            tot["visits"] += 1
            tot["rooms"].add(v["room"])
            marks = ", ".join(f"{k} {n}" for k, n in v["counts"].items()
                              if n and k not in ("stop_reason", "adapted")) or "0"
            if v.get("adapted"):
                marks += " (host-adapted: " + ", ".join(v["adapted"]) + ")"
            if v["clean"]:
                tot["clean"] += 1
                tot["clean_rooms"].add(v["room"])
                ex = "native (paths exercised)"
            else:
                tot["bad_rooms"].add(v["room"])
                ex = "NOT clean: " + "; ".join(
                    f"{d['kind']}" + (f" pc={d['pc']}" if "pc" in d else "") + (f" ra={d['ra']}" if "ra" in d else "")
                    + (f" {d['func']}" if "func" in d and "pc" not in d else "") for d in v["details"][:3])
                for d in v["details"]:
                    if d["kind"] in ("boundary", "stop", "fatal", "unresolved"):
                        stops.append((r["label"], v["room"], d))
            last = " (run end)" if v.get("last") else ""
            out.append(f"| {r['label']} | {i} | {v['room']} | {v['token']:08X} | {v['day']} | {fmt_story(v)} | "
                       f"{v['frame_in']} | {v['frames']}{last} | {v['battles']} | {v['aid_lines']} | {marks} | {ex} |")
    out.append("")
    out.append(f"visits (story >= 0x{min_story:X}, after the warp): {tot['visits']}; clean {tot['clean']}; "
               f"distinct rooms {len(tot['rooms'])}; rooms with any marker: {len(tot['bad_rooms'])} "
               f"({', '.join(sorted(tot['bad_rooms'])) or 'none'})")
    for r in runs:
        lv = r["visits"][-1] if r["visits"] else None
        out.append(f"- {r['label']}: warp frame {r['warp_frame']}, last frame {r['last_frame']}, "
                   f"last room {lv['room'] if lv else '-'} story {lv['story_out']:X} " if lv else
                   f"- {r['label']}: no [ROUTE] change lines ")
        out[-1] += f"end: {r['end']['stop_reason'] or 'no stop_reason line (harness ended the process)'}"
    if stops:
        out.append("")
        out.append("stops / boundaries:")
        for label, room, d in stops:
            out.append(f"- {label} {room} line {d['line']}: {d['text']}")
    if a.aggregate:
        rooms: dict[str, dict] = {}
        for r in runs:
            for v in r["visits"]:
                if v["pre_warp"] or v["story_in"] < min_story or v["story_in"] >= 0x1000:
                    continue
                g = rooms.setdefault(v["room"], {"token": v["token"], "smin": v["story_in"], "smax": v["story_out"],
                                                 "visits": 0, "frames": 0, "battles": 0, "bad": 0, "runs": [],
                                                 "first": (v["story_in"], r["label"], v["frame_in"])})
                g["smin"] = min(g["smin"], v["story_in"])
                g["smax"] = max(g["smax"], v["story_out"])
                g["visits"] += 1
                g["frames"] += v["frames"]
                g["battles"] += v["battles"]
                g["bad"] += sum(v["counts"][k] for k in BAD)
                if r["label"] not in g["runs"]:
                    g["runs"].append(r["label"])
        agg = ["| room | token | day | story range | visits | frames | battles | markers | runs | executed |",
               "|---|---|---|---|--:|--:|--:|--:|---|---|"]
        for name, g in sorted(rooms.items(), key=lambda kv: (kv[1]["first"][0], kv[1]["first"][1], kv[1]["first"][2])):
            d0, d1 = day_of(g["smin"]), day_of(g["smax"])
            agg.append(f"| {name} | {g['token']:08X} | {d0 if d0 == d1 else d0 + '-' + d1} | {g['smin']:X}..{g['smax']:X} | "
                       f"{g['visits']} | {g['frames']} | {g['battles']} | {g['bad']} | {', '.join(g['runs'])} | "
                       f"{'native (paths exercised)' if g['bad'] == 0 else 'NOT clean'} |")
        agg.append("")
        agg.append(f"distinct rooms {len(rooms)}; clean {sum(1 for g in rooms.values() if g['bad'] == 0)}; "
                   f"visits {sum(g['visits'] for g in rooms.values())}; battles {sum(g['battles'] for g in rooms.values())}")
        out = agg + [""] + out[-(len(runs) + (len(stops) + 2 if stops else 0)):]
    text = "\n".join(out)
    print(text)
    if a.md:
        with open(a.md, "w") as f:
            f.write(text + "\n")
    if a.json:
        with open(a.json, "w") as f:
            json.dump(runs, f, indent=1)
    return 0


if __name__ == "__main__":
    sys.exit(main())
