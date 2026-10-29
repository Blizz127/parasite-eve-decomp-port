#!/usr/bin/env python3
"""Aided warp-entry legs for the Day-6 never-touched rooms (sweep 2026-10-21).

usage: sweep_untouched_aided.py --bin BIN --plan PLAN.json --persist PERSIST.bin --out OUTDIR
           --lock LOCK [--rooms m0291i,...] [--frames 6000] [--linger 5000]
           [--room-timeout SEC] [--lock-wait SEC] [--tries N]

COVERAGE EVIDENCE, AIDED, ENTRY ONLY. Each room is a dev warp into the room at
story 0x2D6 with the 0x2D6 persist block that the lane's Day-6 f4 leg dumped
(sha256 0526356d...), run by day5_route_regress.py with its fixtures
(PE_ROUTE_MAXHP=300, PE_CHEAT_KEYS=1010:F3, PE_ONE_HIT_KILLS=1, pilot battle aid).
The day5 waypoints are keyed by room token and story, so no waypoint matches a
Chrysler or m0382i..m0424i room: these legs warp in and idle, they do not walk.
The driver stops the port on a FATAL / CPU_BOUNDARY / [STOP] line and on a game
reset; its own REGRESS line is kept in the manifest.

Env for every run: env -i with fresh HOME, XDG_DATA/CONFIG/STATE_HOME, TMPDIR
and cwd under OUTDIR/<room>-2D6-d2/, DISC and DISC2 set. Run under
flock -w LOCK_WAIT LOCK, nice -n 19, timeout -k 10 ROOM_TIMEOUT.
Rows go to OUTDIR/manifest.jsonl in the same format as sweep_untouched_rooms.py,
plus driver_line (the last REGRESS line of the driver's stdout).
"""
from __future__ import annotations

import argparse
import glob
import json
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from sweep_untouched_rooms import DISC1_DEFAULT, DISC2_DEFAULT, utc  # noqa: E402

STORY = "2D6"
DRIVER = os.path.join(HERE, "day5_route_regress.py")


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--bin", required=True)
    ap.add_argument("--disc1", default=DISC1_DEFAULT)
    ap.add_argument("--disc2", default=DISC2_DEFAULT)
    ap.add_argument("--plan", required=True)
    ap.add_argument("--persist", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--rooms", default="")
    ap.add_argument("--frames", type=int, default=6000)
    ap.add_argument("--linger", type=int, default=5000)
    ap.add_argument("--lock", required=True)
    ap.add_argument("--room-timeout", type=int, default=900)
    ap.add_argument("--lock-wait", type=int, default=1800)
    ap.add_argument("--tries", type=int, default=6)
    a = ap.parse_args(argv)
    if os.path.exists(a.out):
        raise SystemExit(f"sweep_untouched_aided: {a.out} exists (fresh dir required)")
    if not os.path.isfile(a.persist):
        raise SystemExit("sweep_untouched_aided: persist block missing")
    os.makedirs(a.out)
    plan = json.load(open(a.plan, encoding="utf-8"))
    rows = [r for r in plan["rooms"] if r["class"] == "never" and r["story"] == STORY]
    if a.rooms:
        want = set(a.rooms.split(","))
        rows = [r for r in rows if r["room"] in want]
        missing = want - {r["room"] for r in rows}
        if missing:
            raise SystemExit(f"sweep_untouched_aided: not a never-touched 2D6 room: {sorted(missing)}")
    man = os.path.join(a.out, "manifest.jsonl")
    for r in rows:
        room = r["room"]
        tag = f"{room}-{STORY}-d2"
        rd = os.path.join(a.out, tag)
        for sub in ("cwd", "home/data", "home/config", "home/state", "tmp", "logs"):
            os.makedirs(os.path.join(rd, sub))
        env = ["PATH=/usr/bin:/bin", f"HOME={rd}/home", f"XDG_DATA_HOME={rd}/home/data",
               f"XDG_CONFIG_HOME={rd}/home/config", f"XDG_STATE_HOME={rd}/home/state",
               f"TMPDIR={rd}/tmp", f"DISC={a.disc1}", f"DISC2={a.disc2}"]
        game = ["/usr/bin/python3", "-B", DRIVER, "--bin", a.bin, "--log-dir", f"{rd}/logs",
                "--persist", a.persist, "--room", room, "--story", STORY, "--warp-disc", "2",
                "--frames", str(a.frames), "--linger", str(a.linger)]
        out_txt = os.path.join(rd, "driver.out")
        started_flag = os.path.join(rd, "started.flag")
        start_utc = utc()
        rc = None
        attempts = 0
        for attempts in range(1, a.tries + 1):
            cmd = ["flock", "-w", str(a.lock_wait), a.lock, "bash", "-c",
                   'touch "$1"; shift; exec "$@"', "_", started_flag,
                   "nice", "-n", "19", "timeout", "-k", "10", str(a.room_timeout),
                   "env", "-i", *env, *game]
            with open(out_txt, "w") as f:
                proc = subprocess.run(cmd, cwd=f"{rd}/cwd", stdout=f, stderr=subprocess.STDOUT)
            if os.path.exists(started_flag):
                rc = proc.returncode
                break
            rc = None
        end_utc = utc()
        logs = sorted(glob.glob(os.path.join(rd, "logs", "day5-*.log")))
        log = logs[0] if logs else out_txt
        sha = subprocess.run(["sha256sum", log], check=True, capture_output=True, text=True).stdout.split()[0] \
            if os.path.exists(log) else ""
        regress = [l.strip() for l in open(out_txt, errors="replace") if l.startswith("REGRESS")]
        row = {
            "room": room, "class": r["class"], "story": STORY, "disc": 2,
            "story_source": "aided: 2D6 persist block", "hints": r.get("hints", []),
            "frames_requested": a.frames, "run_started": rc is not None, "rc": rc,
            "attempts": attempts, "start_utc": start_utc, "end_utc": end_utc, "dir": rd,
            "log": log, "log_sha256": sha, "started_flag": rc is not None,
            "driver_line": regress[-1] if regress else "",
        }
        with open(man, "a", encoding="utf-8") as f:
            f.write(json.dumps(row) + "\n")
        print(f"{room} rc={rc} started={row['run_started']} attempts={attempts} "
              f"end={end_utc} :: {row['driver_line'][:160]}", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
