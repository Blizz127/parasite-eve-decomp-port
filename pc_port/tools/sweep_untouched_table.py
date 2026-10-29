#!/usr/bin/env python3
"""Per-room table for a sweep_untouched_rooms.py output directory.

usage: sweep_untouched_table.py OUTDIR [--md OUT.md] [--json OUT.json]

Coverage evidence (developer warp, entry only). For every manifest row:
  - the WHOLE run.log is scanned, with no story filter and no "after the first
    visit" filter, so lines before the warp count too (raw counts). Each row also
    records the frame of the first marker line, for triage;
  - entered   = the armed room's token appears in a "[ROUTE] change" line at or
                after the warp frame ("[WARP] env transfer ... frame=N");
  - frames in room = from that change to the next change (or the last frame);
  - next rooms = the room tokens the game changed to after the entry;
  - rc        = the flock/timeout/env exit code (124/137 = timeout);
  - a row with no "[HOST] stop_reason=" line, a timeout, or a lock that never
    came up is an ANOMALY, not CLEAN.
Markers (raw line counts; one line may count under two markers, e.g. a
"[DAY1_SHIM] ... not ported" line):
  boundary        "CPU_BOUNDARY" (the boundary message, all forms)
  stop            "[STOP]" (game_port.c: "[STOP] reason=...")
  stop_reason     "stop_reason=" with a value other than frame-limit
  day1_shim       "[DAY1_SHIM]"
  stub_bootstrap  "STUB:BOOTSTRAP" ("[STUB:BOOTSTRAP_RET] <symbol>")
  ret_void        "Bootstrap_ReturnVoid"
  not_ported      "not ported"
  unresolved      "unresolved"
  fatal           "FATAL"
  stub_other      "[STUB:" lines other than HOST_ADAPTED and BOOTSTRAP
HOST_ADAPTED stub lines are not counted; their names are listed. The
"[NATIVE] boundary policy=stop" banner is not a marker. Error-like lines outside
those markers are grouped by shape (digits and hex normalised).
"""
from __future__ import annotations

import argparse
import json
import os
import re
import sys
from collections import Counter

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from laterdays_room_table import room_name  # noqa: E402

RE_ARMED = re.compile(r"^\[WARP\] armed room=(\S+) token=([0-9A-Fa-f]{8}) story=([0-9A-Fa-f]+) at>=(\d+)")
RE_WARP = re.compile(r"^\[WARP\] env transfer ([0-9A-Fa-f]{8}) -> ([0-9A-Fa-f]{8}) story=([0-9A-Fa-f]{8}) frame=(\d+)")
RE_CHANGE = re.compile(r"^\[ROUTE\] change frame=(\d+) token=([0-9A-Fa-f]{8}) story=([0-9A-Fa-f]{8})")
RE_FRAME = re.compile(r"\bframe=(\d+)")
RE_STOPR = re.compile(r"stop_reason=(\S+)")
RE_BEDGE = re.compile(r"^BATTLE_EDGE \d+ token=[0-9A-Fa-f]{8} d1a0&802=(\d+)")
RE_ERRLIKE = re.compile(r"(?i)error|abort|assert|panic|exception|segfault|unsupported|refused|out of range|"
                        r"invalid|not implemented|unimplemented|undefined|cannot|could not|failed|warning|missing")
RE_SHAPE = re.compile(r"0x[0-9A-Fa-f]+|\b[0-9A-Fa-f]{6,}\b|\d+")
# End-of-run "=== Stub Summary ===" block (stub_registry.c, Stub_PrintSummary).
RE_SUMMARY = re.compile(r"^(implemented|host-adapted|bootstrap-return|unsupported|"
                        r"invoked bootstrap stubs|uninvoked stubs):\s+(\d+)$|^=== Stub Summary ===$")
MARK_DEFS = [
    ("boundary", lambda l: "CPU_BOUNDARY" in l),
    ("stop", lambda l: "[STOP]" in l),
    ("day1_shim", lambda l: "[DAY1_SHIM]" in l),
    ("stub_bootstrap", lambda l: "STUB:BOOTSTRAP" in l),
    ("ret_void", lambda l: "Bootstrap_ReturnVoid" in l),
    ("not_ported", lambda l: "not ported" in l),
    ("unresolved", lambda l: "unresolved" in l),
    ("fatal", lambda l: "FATAL" in l),
    ("stub_other", lambda l: l.startswith("[STUB:") and "HOST_ADAPTED" not in l and "BOOTSTRAP" not in l),
]
BAD = ("boundary", "stop", "stop_reason", "day1_shim", "stub_bootstrap", "ret_void",
       "not_ported", "unresolved", "fatal", "stub_other")
MAX_DETAIL = 3


def scan(path: str) -> dict:
    counts = Counter()
    detail: dict[str, list] = {k: [] for k in BAD}
    host_adapted: Counter = Counter()
    errlike: Counter = Counter()
    stop_values: list[str] = []
    armed = warp = None
    warp_from = None
    changes: list[tuple[int, str, str]] = []
    last_frame = 0
    cur_frame = 0
    battles = 0
    in_battle = False
    banner = 0
    summary: dict = {}
    mounted = False
    with open(path, errors="replace") as f:
        for n, raw in enumerate(f, 1):
            line = raw.rstrip("\n")
            m = RE_FRAME.search(line)
            if m:
                cur_frame = int(m.group(1))
                last_frame = max(last_frame, cur_frame)
            if line.startswith("[NATIVE] boundary policy=stop"):
                banner += 1
            if "disc 2 in the drive (mounted)" in line:
                mounted = True
            a = RE_ARMED.search(line)
            if a and armed is None:
                armed = (a.group(1), a.group(2).upper(), a.group(3).upper(), int(a.group(4)))
            w = RE_WARP.search(line)
            if w and warp is None:
                warp = int(w.group(4))
                warp_from = w.group(2).upper()
            c = RE_CHANGE.search(line)
            if c:
                changes.append((int(c.group(1)), c.group(2).upper(), c.group(3).upper()))
            b = RE_BEDGE.search(line)
            if b:
                now = b.group(1) != "0"
                if now and not in_battle:
                    battles += 1
                in_battle = now
            if line.startswith("[HOST] stop_reason=") or RE_STOPR.search(line):
                for v in RE_STOPR.findall(line):
                    stop_values.append(v)
                    if v != "frame-limit":
                        counts["stop_reason"] += 1
                        if len(detail["stop_reason"]) < MAX_DETAIL:
                            detail["stop_reason"].append((n, cur_frame, line.strip()[:200]))
            hit = False
            for key, pred in MARK_DEFS:
                if pred(line):
                    hit = True
                    counts[key] += 1
                    if len(detail[key]) < MAX_DETAIL:
                        detail[key].append((n, cur_frame, line.strip()[:220]))
            sm = RE_SUMMARY.match(line)
            if sm:
                if sm.group(1) == "invoked bootstrap stubs":
                    summary["invoked_bootstrap"] = int(sm.group(2))
                elif sm.group(1) == "unsupported":
                    summary["unsupported"] = int(sm.group(2))
                elif sm.group(1):
                    summary[sm.group(1)] = int(sm.group(2))
                continue
            if line.startswith("[STUB:HOST_ADAPTED]"):
                nm = re.match(r"\[STUB:HOST_ADAPTED\] (\S+)", line)
                host_adapted[nm.group(1) if nm else line[:60]] += 1
            if not hit and not line.startswith("[STUB:HOST_ADAPTED]") and RE_ERRLIKE.search(line):
                errlike[RE_SHAPE.sub("N", line.strip())[:140]] += 1
    entered = None
    frames_in = None
    next_rooms: list[str] = []
    noop = False
    if armed and warp is not None:
        tok = armed[1]
        idx = next((i for i, (fr, t, _s) in enumerate(changes) if t == tok and fr >= warp), None)
        if idx is None and warp_from == tok:
            # The armed room is the room the game already stood in (a Day-1 route room): the
            # warp is a no-op. Its entry is the route's own change to that token, before the warp.
            idx = max((i for i, (fr, t, _s) in enumerate(changes) if t == tok and fr < warp), default=None)
            noop = idx is not None
        if idx is not None:
            entered = changes[idx][0]
            frame_out = next((fr for fr, _t, _s in changes[idx + 1:]), None)
            frame_out = frame_out if frame_out is not None else last_frame
            frames_in = frame_out - entered
            seen = []
            for fr, t, s in changes[idx + 1:]:
                nm = room_name(int(t, 16))
                tag = f"{nm} ({int(s, 16):X})"
                if tag not in seen:
                    seen.append(tag)
            next_rooms = seen
    return {
        "counts": {k: counts.get(k, 0) for k in BAD},
        "detail": detail,
        "host_adapted": dict(host_adapted),
        "errlike": dict(errlike),
        "stop_values": sorted(set(stop_values)),
        "armed": armed,
        "warp_frame": warp,
        "entered_frame": entered,
        "noop_warp": noop,
        "frames_in_room": frames_in,
        "next_rooms": next_rooms,
        "battle_entries": battles,
        "last_frame": last_frame,
        "banner_lines": banner,
        "summary": summary,
        "disc2_mounted": mounted,
    }


def status(row: dict, s: dict) -> tuple[str, list[str]]:
    why: list[str] = []
    aided = bool(row.get("driver_line"))  # day5 driver legs: its own REGRESS line decides, not rc
    if not row["run_started"]:
        why.append("NOT RUN: lock never acquired")
    elif aided:
        if "reason=stop:" in row["driver_line"]:
            why.append("driver stop: " + row["driver_line"][row["driver_line"].index("reason="):][:160])
        elif "game reset" in row["driver_line"]:
            why.append("note: game reset/game over (game response, not a native stop)")
    elif row["rc"] in (124, 137):
        why.append(f"ANOMALY: timeout rc={row['rc']}")
    elif row["rc"] != 0:
        why.append(f"ANOMALY: rc={row['rc']}")
    if row["run_started"] and not aided and not s["stop_values"]:
        why.append("ANOMALY: no stop_reason line")
    if s["armed"] is None or s["warp_frame"] is None:
        why.append("warp not transferred")
    elif s["entered_frame"] is None:
        why.append("target not entered")
    for k in BAD:
        if s["counts"][k]:
            why.append(f"{k}={s['counts'][k]}")
    if row["disc"] == 2 and not s.get("disc2_mounted"):
        why.append("disc 2 mounted line missing")
    if s.get("summary", {}).get("invoked_bootstrap", 0):
        why.append(f"stub summary: invoked bootstrap stubs={s['summary']['invoked_bootstrap']}")
    if s.get("summary", {}).get("unsupported", 0):
        why.append(f"stub summary: unsupported={s['summary']['unsupported']}")
    if not why:
        return "CLEAN", why
    if any(w.startswith(("NOT RUN", "ANOMALY", "warp not", "target not")) for w in why):
        return "ANOMALY/FLAG", why
    if all(w.startswith("note:") for w in why):
        return "CLEAN", why
    return "FLAG", why


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("outdir")
    ap.add_argument("--md", default="")
    ap.add_argument("--json", default="")
    a = ap.parse_args(argv)
    man = os.path.join(a.outdir, "manifest.jsonl")
    rows = [json.loads(l) for l in open(man, encoding="utf-8")] if os.path.exists(man) else []
    out_rows = []
    totals: Counter = Counter()
    host_names: Counter = Counter()
    errshapes: Counter = Counter()
    status_counts: Counter = Counter()
    for row in rows:
        if row["run_started"] and os.path.exists(row["log"]):
            s = scan(row["log"])
        else:
            s = {"counts": {k: 0 for k in BAD}, "detail": {k: [] for k in BAD}, "host_adapted": {},
                 "errlike": {}, "stop_values": [], "armed": None, "warp_frame": None,
                 "entered_frame": None, "noop_warp": False, "frames_in_room": None, "next_rooms": [], "battle_entries": 0,
                 "last_frame": 0, "banner_lines": 0, "summary": {}}
        st, why = status(row, s)
        status_counts[st] += 1
        for k in BAD:
            totals[k] += s["counts"][k]
        host_names.update(s["host_adapted"])
        errshapes.update(s["errlike"])
        out_rows.append({
            "room": row["room"], "class": row["class"], "story": row["story"], "disc": row["disc"],
            "story_source": row["story_source"], "hints": row["hints"], "rc": row["rc"],
            "run_started": row["run_started"], "attempts": row["attempts"],
            "frames_requested": row["frames_requested"], "last_frame": s["last_frame"],
            "entered": s["entered_frame"] is not None, "entered_frame": s["entered_frame"],
            "noop_warp": s["noop_warp"],
            "frames_in_room": s["frames_in_room"], "next_rooms": s["next_rooms"],
            "battle_entries": s["battle_entries"], "stop_values": s["stop_values"],
            "counts": s["counts"], "host_adapted": s["host_adapted"], "errlike": s["errlike"],
            "details": s["detail"], "status": st, "why": why, "banner_lines": s["banner_lines"],
            "summary": s["summary"], "disc2_mounted": s.get("disc2_mounted", False),
            "driver_line": row.get("driver_line", ""),
            "log": row["log"], "log_sha256": row["log_sha256"], "start_utc": row["start_utc"],
            "end_utc": row["end_utc"],
        })

    def cell(v):
        return "-" if v in (None, "") else str(v)

    lines = ["| room | class | story | disc | rc | entered | frames in room | next rooms (story) | battles | "
             "stop_reason | boundary | [STOP] | DAY1_SHIM | STUB:BOOTSTRAP | Bootstrap_ReturnVoid | not ported | "
             "unresolved | FATAL | stub other | HOST_ADAPTED (names) | other error-like | last frame | status |",
             "|---|---|---|--:|--:|---|--:|---|--:|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|--:|--:|---|"]
    for r in out_rows:
        c = r["counts"]
        nxt = ", ".join(r["next_rooms"][:4]) + (" ..." if len(r["next_rooms"]) > 4 else "") if r["next_rooms"] else "-"
        ha = ", ".join(sorted(r["host_adapted"])) if r["host_adapted"] else "-"
        lines.append(
            f"| {r['room']} | {r['class']} | {r['story']} | {r['disc']} | {cell(r['rc'])} | "
            f"{('yes (no-op warp: route entry at frame ' + str(r['entered_frame']) + ')') if r['noop_warp'] else ('yes' if r['entered'] else 'no')} | "
            f"{cell(r['frames_in_room'])} | {nxt} | {r['battle_entries']} | "
            f"{', '.join(r['stop_values']) or '-'} | {c['boundary']} | {c['stop']} | {c['day1_shim']} | "
            f"{c['stub_bootstrap']} | {c['ret_void']} | {c['not_ported']} | {c['unresolved']} | {c['fatal']} | "
            f"{c['stub_other']} | {ha} | {sum(r['errlike'].values())} | {r['last_frame']} | "
            f"{r['status']}{' (' + '; '.join(r['why']) + ')' if r['status'] != 'CLEAN' else ''} |")
    summary = [
        f"rows: {len(out_rows)}; status: " + ", ".join(f"{k}={v}" for k, v in sorted(status_counts.items())),
        "marker totals (raw lines over all rows): " + ", ".join(f"{k}={totals[k]}" for k in BAD),
        f"entered: {sum(1 for r in out_rows if r['entered'])} of {len(out_rows)}",
        "HOST_ADAPTED stub names (not counted as markers): " + (", ".join(f"{k} x{v}" for k, v in sorted(host_names.items())) or "none"),
        "other error-like shapes: " + (" | ".join(f"{k} x{v}" for k, v in errshapes.most_common(12)) or "none"),
    ]
    text = "\n".join(summary) + "\n\n" + "\n".join(lines) + "\n"
    if a.md:
        with open(a.md, "w", encoding="utf-8") as f:
            f.write(text)
    if a.json:
        with open(a.json, "w", encoding="utf-8") as f:
            json.dump({"rows": out_rows, "totals": dict(totals), "status": dict(status_counts),
                       "host_adapted": dict(host_names), "errlike": dict(errshapes)}, f, indent=1)
    sys.stdout.write(text)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
