#!/usr/bin/env python3
"""Day-2 route regression (dev harness): segments of the Day 2 story that the
game plays through its OWN room exits, checked against ordered checkpoints.

There is no recorded Day 2 input (the Day 1 gate replays a recorded pad
sequence), so each segment starts with one dev warp (PE_WARP_ROOM /
PE_WARP_STORY, the field VM's own room_transfer) into a room whose script then
drives the story by itself: cutscenes, the end-of-day save screen and the
script-issued room transfers. The pad alternates Cross (advance text) and
Circle (cancel the end-of-day save screen without saving). Every later room
change comes from the room scripts, not from the harness.

Segments (token = D_8009D280, story = persist[74] @0x800A7918):
  day2_start  warp m0351i story 0x80 (Day-2 hop): save screen cancelled,
              Day 2 title card, m0042i (0x88), its script writes 0x90 and
              exits to m0041i.
  day2_end    warp m0090i story 0x128: m0136i (0x134), m0126i, m0125i,
              m0091i (0x136), m0351i (0x138, save screen cancelled), m0092i
              (Day 3 entry, 0x140).
A segment fails on a FATAL, CPU_BOUNDARY/REFUSED or [STOP] line, a story drop,
a checkpoint firing out of order, or a missed checkpoint.
"""
import argparse
import os
import re
import subprocess
import sys
import time

SEGMENTS = {
    "day2_start": dict(room="m0351i", story="80", frames=8000, checkpoints=[
        ("token", 0xA80650C8, "m0351i Day-2 hop (story 0x80)"),
        ("token", 0xA8004148, "m0042i entered by the hop's room_transfer"),
        ("story>=", 0x88, "story 0x88 written by m0351i"),
        ("story>=", 0x90, "m0042i script writes story 0x90"),
        ("token", 0xA80040C8, "m0041i entered by m0042i's room_transfer"),
    ]),
    "day2_end": dict(room="m0090i", story="128", frames=22000, checkpoints=[
        ("token", 0xA8009048, "m0090i (story 0x128)"),
        ("story>=", 0x134, "m0090i script writes story 0x134"),
        ("token", 0xA8023348, "m0136i"),
        ("token", 0xA8022348, "m0126i"),
        ("token", 0xA80222C8, "m0125i"),
        ("story>=", 0x136, "m0125i writes story 0x136"),
        ("token", 0xA80090C8, "m0091i"),
        ("story>=", 0x138, "m0091i writes story 0x138 (end of Day 2)"),
        ("token", 0xA80650C8, "m0351i Day-3 hop"),
        ("story>=", 0x140, "m0351i writes story 0x140"),
        ("token", 0xA8009148, "m0092i Day 3 entry"),
    ]),
}

RE_ROUTE = re.compile(r"\[ROUTE\] (?:change )?frame=(\d+) token=([0-9A-Fa-f]{8}) story=([0-9A-Fa-f]{8})")
RE_BAD = re.compile(r"CPU_BOUNDARY|FATAL|^\[STOP\]")


def pad_script() -> str:
    seq = ["BFFF", "FFFF", "DFFF", "FFFF"]       # Cross, release, Circle, release
    return ",".join(f"{i * 40}:{seq[i % 4]}" for i in range(1020))


def run_segment(name: str, seg: dict, a, disc: str, disc2: str | None, prefix: str = "day2") -> bool:
    log = os.path.join(a.log_dir, f"{prefix}-{name}.log")
    env = dict(os.environ, PE_PAD_SCRIPT=pad_script(), PE_DEV_MENU="1",
               PE_WARP_ROOM=seg["room"], PE_WARP_STORY=seg["story"], PE_WARP_AT="1000")
    env.update(seg.get("env", {}))               # per-segment harness switches (Day 3 walk)
    cmd = [a.bin, "--headless", "--route-pad", "--hand-over-at", "1000",
           "--max-frames", str(seg["frames"]), "--disc-image", disc]
    if disc2:
        cmd += ["--disc2-image", disc2]
    cps, idx, frame, token, story, peak, fail = seg["checkpoints"], 0, 0, 0, 0, 0, None
    done_at = None
    t0 = time.time()
    with open(log, "w") as lf:
        p = subprocess.Popen(cmd, env=env, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE,
                             text=True, errors="replace")
        for line in p.stderr:
            lf.write(line)
            if RE_BAD.search(line):
                fail = "stop: " + line.strip()[:200]
                break
            m = RE_ROUTE.search(line)
            if not m:
                continue
            frame, token, story = int(m.group(1)), int(m.group(2), 16), int(m.group(3), 16)
            if frame < 1000 or story >= 0x1000:
                continue                          # boot prefix before the warp
            if story < peak:
                fail = f"story fell 0x{peak:X} -> 0x{story:X}"
                break
            peak = story
            while idx < len(cps):
                k, v, d = cps[idx]
                if not ((k == "token" and token == v) or (k == "story>=" and story >= v)):
                    break
                print(f"REGRESS ok   {name}: {d:48s} frame={frame} token={token:08X} story={story:X}",
                      flush=True)
                idx += 1
            if idx == len(cps):
                done_at = done_at if done_at is not None else frame
                if frame - done_at >= seg.get("linger", 0):
                    break                         # linger: e.g. let the persist dump land
                continue
            for k, v, d in cps[idx + 1:]:
                if k == "token" and token == v:
                    fail = f"checkpoint '{d}' fired before '{cps[idx][2]}'"
                    break
            if fail:
                break
        if p.poll() is None:
            p.terminate()
            try:
                p.wait(timeout=20)
            except subprocess.TimeoutExpired:
                p.kill()
        p.wait()
    dt = time.time() - t0
    if idx == len(cps) and not fail:
        print(f"REGRESS PASS {name} frame={frame} token={token:08X} story={story:X} "
              f"elapsed={dt:.0f}s log={log}")
        return True
    print(f"REGRESS FAIL {name} at '{cps[idx][2] if idx < len(cps) else 'end'}' last frame={frame} "
          f"token={token:08X} story={story:X} reason={fail or 'run ended before checkpoint'} log={log}")
    return False


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--bin", default="build/pcbuild-disc/parasite-eve-port")
    ap.add_argument("--segment", choices=sorted(SEGMENTS), action="append")
    ap.add_argument("--log-dir", default="build/lanes/day2")
    ap.add_argument("--list", action="store_true")
    a = ap.parse_args()
    if a.list:
        for n, s in SEGMENTS.items():
            print(f"{n}: warp {s['room']} story 0x{s['story']}, {s['frames']} frames")
            for k, v, d in s["checkpoints"]:
                print(f"    {k:8s} {v:#010x}  {d}")
        return 0
    disc = os.environ.get("DISC")
    if not disc:
        print("REGRESS FAIL no_disc: set DISC to the Disc 1 image")
        return 2
    if not os.access(a.bin, os.X_OK):
        print(f"REGRESS FAIL no_binary: {a.bin}")
        return 2
    os.makedirs(a.log_dir, exist_ok=True)
    ok = all([run_segment(n, SEGMENTS[n], a, disc, os.environ.get("DISC2"))
              for n in (a.segment or list(SEGMENTS))])
    print("DAY2_ROUTE=" + ("PASS" if ok else "FAIL"))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
