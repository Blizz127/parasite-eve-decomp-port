#!/usr/bin/env python3
"""Warp-enter each room of a sweep plan, one fresh process per room.

usage: sweep_untouched_rooms.py --bin BIN [--disc1 DISC1] [--disc2 DISC2] \
           --plan PLAN.json --out OUTDIR --lock LOCK [--class never|warp-only|all] \
           [--rooms m0001i,m0002i] [--room-timeout SEC] \
           [--lock-wait SEC] [--tries N] [--frames N] [--extra VAR=VAL ...]

The disc defaults are the repository-relative rom/image paths that the other
tools use. --lock names the run-slot lock file the host's scheduling uses; it
has no default.

COVERAGE EVIDENCE, ASSISTED. Every run is a developer warp (PE_DEV_MENU=1,
PE_WARP_ROOM/PE_WARP_STORY/PE_WARP_AT=1000, PE_WARP_DISC=2 for disc-2 rows)
after the --route-pad hand-over at frame 1000, with the same Cross mash as
laterdays_warp_sweep.sh. It shows whether a room loads and runs its script
natively and which boundary/stub/shim lines it prints. It is not play evidence.

Each run: env -i with a fresh HOME, XDG_DATA/CONFIG/STATE_HOME, TMPDIR and cwd
under OUTDIR/<room>-<story>-d<disc>/, run under
  flock -w LOCK_WAIT LOCK nice -n 19 timeout -k 10 ROOM_TIMEOUT
A run whose lock was never acquired (no start flag) is retried, up to --tries
times; if it still never starts, its manifest row says NOT RUN. Rows are
appended to OUTDIR/manifest.jsonl with the exit code, log path and sha256 of
run.log (from sha256sum), and the date -u stamps.
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys

DISC1_DEFAULT = "rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin"
DISC2_DEFAULT = "rom/image/Parasite Eve (USA) (Disc 2)/Parasite Eve (USA) (Disc 2).bin"
START_SCRIPT = 'touch "$1"; shift; exec "$@"'


def utc() -> str:
    return subprocess.run(["date", "-u", "+%Y-%m-%dT%H:%M:%SZ"], check=True,
                          capture_output=True, text=True).stdout.strip()


def pad_script() -> str:
    # The Cross mash of laterdays_warp_sweep.sh, verbatim: 400 steps, Cross on (BFFF)
    # and off (FFFF) every 20 frames (8000 frames, more than any run here).
    return ",".join(f"{i * 20}:{'BFFF' if i % 2 == 0 else 'FFFF'}" for i in range(400))


def run_row(row: dict, a: argparse.Namespace, out: str) -> dict:
    room, story, disc = row["room"], row["story"], int(row["disc"])
    frames = int(a.frames or row["frames"])
    tag = f"{room}-{story}-d{disc}"
    rd = os.path.join(out, tag)
    if os.path.exists(rd):
        raise SystemExit(f"sweep_untouched_rooms: {rd} exists (fresh dirs only)")
    for sub in ("cwd", "home/data", "home/config", "home/state", "tmp"):
        os.makedirs(os.path.join(rd, sub))
    env = [
        "PATH=/usr/bin:/bin",
        f"HOME={rd}/home",
        f"XDG_DATA_HOME={rd}/home/data",
        f"XDG_CONFIG_HOME={rd}/home/config",
        f"XDG_STATE_HOME={rd}/home/state",
        f"TMPDIR={rd}/tmp",
        f"PE_PAD_SCRIPT={pad_script()}",
        "PE_DEV_MENU=1",
        f"PE_WARP_ROOM={room}",
        f"PE_WARP_STORY={story}",
        "PE_WARP_AT=1000",
    ]
    if disc == 2:
        env.append("PE_WARP_DISC=2")
    env += a.extra
    game = [a.bin, "--headless", "--route-pad", "--hand-over-at", "1000",
            "--max-frames", str(frames), "--disc-image", a.disc1, "--disc2-image", a.disc2,
            "--screenshot", f"{rd}/end.ppm"]
    log = f"{rd}/run.log"
    started_flag = f"{rd}/started.flag"
    attempts = 0
    start_utc = utc()
    rc = None
    for attempts in range(1, a.tries + 1):
        cmd = ["flock", "-w", str(a.lock_wait), a.lock, "bash", "-c", START_SCRIPT, "_", started_flag,
               "nice", "-n", "19", "timeout", "-k", "10", str(a.room_timeout),
               "env", "-i", *env, *game]
        with open(log, "w") as lf:
            proc = subprocess.run(cmd, cwd=f"{rd}/cwd", stdout=lf, stderr=subprocess.STDOUT)
        if os.path.exists(started_flag):
            rc = proc.returncode
            break
        with open(log, "a") as lf:
            lf.write(f"[sweep] lock not acquired (attempt {attempts}, flock rc {proc.returncode}); retrying\n")
        rc = None
    end_utc = utc()
    started = rc is not None
    sha = ""
    if os.path.exists(log):
        sha = subprocess.run(["sha256sum", log], check=True, capture_output=True, text=True).stdout.split()[0]
    return {
        "room": room, "class": row.get("class", ""), "story": story, "disc": disc,
        "story_source": row.get("story_source", ""), "hints": row.get("hints", []),
        "frames_requested": frames, "run_started": started, "rc": rc, "attempts": attempts,
        "start_utc": start_utc, "end_utc": end_utc, "dir": rd, "log": log,
        "log_sha256": sha, "started_flag": os.path.exists(started_flag),
    }


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--bin", required=True)
    ap.add_argument("--disc1", default=DISC1_DEFAULT)
    ap.add_argument("--disc2", default=DISC2_DEFAULT)
    ap.add_argument("--plan", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--class", dest="klass", default="never", choices=("never", "warp-only", "all"))
    ap.add_argument("--rooms", default="")
    ap.add_argument("--lock", required=True)
    ap.add_argument("--room-timeout", type=int, default=420)
    ap.add_argument("--lock-wait", type=int, default=1800)
    ap.add_argument("--tries", type=int, default=6)
    ap.add_argument("--frames", type=int, default=0)
    ap.add_argument("--extra", nargs="*", default=[])
    a = ap.parse_args(argv)
    if os.path.exists(a.out):
        raise SystemExit(f"sweep_untouched_rooms: {a.out} exists (fresh dir required)")
    if not (os.access(a.bin, os.X_OK) and os.path.isfile(a.disc1) and os.path.isfile(a.disc2)):
        raise SystemExit("sweep_untouched_rooms: binary or disc missing")
    os.makedirs(a.out)
    plan = json.load(open(a.plan, encoding="utf-8"))
    rows = [r for r in plan["rooms"] if a.klass == "all" or r["class"] == a.klass]
    if a.rooms:
        want = set(a.rooms.split(","))
        rows = [r for r in rows if r["room"] in want]
        missing = want - {r["room"] for r in rows}
        if missing:
            raise SystemExit(f"sweep_untouched_rooms: not in plan: {sorted(missing)}")
    with open(os.path.join(a.out, "start.txt"), "w") as f:
        f.write(f"START {utc()} rows={len(rows)} bin={a.bin} sha_from=sha256sum-per-log\n")
    man = os.path.join(a.out, "manifest.jsonl")
    for row in rows:
        res = run_row(row, a, a.out)
        with open(man, "a", encoding="utf-8") as f:
            f.write(json.dumps(res) + "\n")
        print(f"{res['room']} story={res['story']} d{res['disc']} rc={res['rc']} "
              f"started={res['run_started']} attempts={res['attempts']} end={res['end_utc']}", flush=True)
    with open(os.path.join(a.out, "end.txt"), "w") as f:
        f.write(f"END {utc()}\n")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
