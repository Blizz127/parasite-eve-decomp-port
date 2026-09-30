#!/usr/bin/env python3
"""Day-1 route regression checker (driven by day1_route_regress.sh).

Runs the headless --route-pad autopilot from cold boot, streams its stderr
and checks an ordered list of checkpoints. It exits non-zero on the first
checkpoint that is missed (the run ended or a later checkpoint fired first),
or when the game resets (the story value drops back after progress, which
means game over/New Game). On failure it prints the last frame/token/story seen.

Checkpoints, in route order (token = D_8009D280, story = persist[74] @0x800A7918):
"""
import argparse
import os
import re
import subprocess
import sys
import time

DISC_DEFAULT = "rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin"

# (name, kind, arg, description)
#   kind 'story>=': story value reaches arg
#   kind 'token'  : room token equals arg
#   kind 'line'   : a log line contains arg
CHECKPOINTS = [
    ("field_start",    "story>=", 0x09, "New Game reaches the first field room"),
    ("eve1_room",      "token",   0xA80021C8, "M0023I stage (first Eve fight)"),
    ("eve1_victory",   "story>=", 0x5E, "M0023I fight done, story 0x5E"),
    ("sewer_entry",    "story>=", 0x68, "M0026I sewer entry (story 0x68)"),
    ("sewer1_victory", "line",    "route: sewer victory room=1", "M0027I first sewer victory"),
    ("m0028i",         "token",   0xA8002448, "M0028I second hallway"),
    ("sewer2_victory", "line",    "route: sewer victory room=2", "M0028I sewer victory"),
    ("m0031i",         "token",   0xA80030C8, "M0031I"),
    ("m0032i",         "token",   0xA8003148, "M0032I"),
    ("m0033i",         "token",   0xA80031C8, "M0033I"),
    ("m34_entry",      "token",   0xA8003248, "M0034I alligator arena (story 0x6C)"),
    ("m34_tail_broken","line",    "M34_TAIL_BROKEN", "M34 type-4 tail HP <= 1,000,000"),
    ("m34_phase2",     "token",   0xA8064448, "M34 phase 2 sewer tunnel (after tail break)"),
    ("m34_exit",       "story>=", 0x70, "M34 script exit (story 0x70)"),
    ("m0359i",         "token",   0xA80654C8, "M0359I post-boss room"),
    ("day1_exit_78",   "story>=", 0x78, "M0359I -> M0036I (story 0x78)"),
    ("m0036i",         "token",   0xA8003348, "M0036I Day-1 exit room"),
    ("day1_end_80",    "story>=", 0x80, "M0036I Day-1 completion marker persist[74]=0x80"),
]
NAMES = [c[0] for c in CHECKPOINTS]

RE_ROUTE = re.compile(r"\[ROUTE\] (?:change )?frame=(\d+) token=([0-9A-Fa-f]{8}) story=([0-9A-Fa-f]{8})")
RE_FRAME = re.compile(r"^(?:\S+ )?(\d{3,})\b")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--bin", default="build/pcbuild-port/parasite-eve-port")
    ap.add_argument("--until", default="m34_entry", choices=NAMES)
    ap.add_argument("--max-frames", type=int, default=90000)
    ap.add_argument("--log")
    ap.add_argument("--screenshot")
    ap.add_argument("--list", action="store_true")
    ap.add_argument("--skip-normal-start-check", action="store_true")
    a = ap.parse_args()
    if a.list:
        for n, k, v, d in CHECKPOINTS:
            print(f"{n:16s} {k:8s} {v if isinstance(v, str) else hex(v):32s} {d}")
        return 0
    disc = os.environ.get("DISC", DISC_DEFAULT)
    if not os.access(a.bin, os.X_OK):
        print(f"REGRESS FAIL no_binary: {a.bin} missing or not executable")
        return 2
    log = a.log or f"build/lanes/day1/regress-{a.until}.log"
    shot = a.screenshot or re.sub(r"\.log$", "", log) + ".ppm"

    # 0. Autopilot aids must be disabled on a normal start.
    if not a.skip_normal_start_check:
        r = subprocess.run([a.bin, "--headless", "--max-frames", "30", "--disc-image", disc],
                           stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True,
                           errors="replace", timeout=600)
        if "[ROUTE_SHIM] disabled (normal start" not in r.stderr or "ROUTE_SHIM] M34" in r.stderr:
            print("REGRESS FAIL normal_start_shim_off: normal start did not report route aids disabled")
            return 3
        print("REGRESS ok   normal_start_shim_off")

    targets = CHECKPOINTS[: NAMES.index(a.until) + 1]
    cmd = [a.bin, "--headless", "--route-pad", "--max-frames", str(a.max_frames),
           "--screenshot", shot, "--disc-image", disc]
    print("REGRESS cmd:", " ".join(repr(c) if " " in c else c for c in cmd))
    t0 = time.time()
    idx = 0
    frame, token, story, peak = 0, 0, 0, 0
    fail = None
    with open(log, "w") as lf:
        p = subprocess.Popen(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE,
                             text=True, errors="replace")
        for line in p.stderr:
            lf.write(line)
            lf.flush()
            m = RE_ROUTE.search(line)
            if m:
                frame, token, story = int(m.group(1)), int(m.group(2), 16), int(m.group(3), 16)
                if story < 0x1000:
                    if peak >= 0x10 and story < peak and story <= 0x0A:
                        fail = f"game reset/game over: story fell 0x{peak:X} -> 0x{story:X}"
                        break
                    peak = max(peak, story)
            # A checkpoint may be satisfied by any later one firing, so test all
            # remaining in order and advance through consecutive hits only.
            while idx < len(targets):
                n, k, v, d = targets[idx]
                hit = ((k == "story>=" and m and story < 0x1000 and story >= v) or
                       (k == "token" and m and token == v) or
                       (k == "line" and v in line))
                if not hit:
                    break
                if k == "line":
                    fm = re.search(r"frame=(\d+)", line) or re.match(r"\S+ (\d+)", line)
                    if fm:
                        frame = int(fm.group(1))
                print(f"REGRESS ok   {n:16s} frame={frame} token={token:08X} story={story:X}  ({d})",
                      flush=True)
                idx += 1
            if idx == len(targets):
                break
            # A later token/story checkpoint firing first means this one was skipped.
            for j in range(idx + 1, len(targets)):
                n, k, v, d = targets[j]
                if (k == "token" and m and token == v) or (k == "line" and v in line):
                    fail = f"checkpoint {n} fired before {targets[idx][0]}"
                    break
            if fail:
                break
        if p.poll() is None:
            p.terminate()
            try:
                p.wait(timeout=20)
            except subprocess.TimeoutExpired:
                p.kill()
        rc = p.wait()
    dt = time.time() - t0
    if idx == len(targets) and not fail:
        print(f"REGRESS PASS until={a.until} frame={frame} token={token:08X} story={story:X} "
              f"elapsed={dt:.0f}s log={log}")
        return 0
    n, k, v, d = targets[idx]
    print(f"REGRESS FAIL {n} ({d}) last frame={frame} token={token:08X} story={story:X} "
          f"peak_story={peak:X} port_rc={rc} reason={fail or 'run ended before checkpoint'} "
          f"elapsed={dt:.0f}s log={log} screenshot={shot}")
    return 1


if __name__ == "__main__":
    sys.exit(main())
