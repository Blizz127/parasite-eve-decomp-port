#!/usr/bin/env python3
"""Day 5 route measurement (harness-only, ASSISTED-DIAGNOSTIC: coverage
evidence of which room code runs natively, never play evidence).

  day5_walk   warp m0352i at story 0x218 with the persist block that the
              day4_walk segment dumped at 0x218 (--persist; run
              day4_route_regress.py with PE_D3_PERSIST_DUMP=<dir> to get
              <dir>/persist_218.bin). m0352i's own script writes 0x220 and
              opens the world map; the route pilot then replays
              day5_walk_waypoints.txt: Chinatown, the sewers, the subway, the
              bridge (Gate Key, story 0x258, the Disc 2 story boundary), the
              museum, Eve, to m0273i at 0x2D0 (the last waypoint entry).
              Fixtures as day3_walk/day4_walk: PE_ROUTE_MAXHP=300, cheat F3,
              PE_ONE_HIT_KILLS=1, plus the persist warp. The persist file does
              not carry the inventory (docs/evidence/day2-warp-probe/REPORT.md).

  --room/--story [--warp-disc 2] start the same walk at a later room (a
              start leg, e.g. m0192i at 0x258 with Disc 2 in the drive);
              only the later story checkpoints are kept.

  --override KEY=WAYPOINTS replaces one entry of the committed list for this
              run only (harness tuning; the committed file is unchanged).

  --with-day4 runs the same walk continuously from the Day 4 entry instead:
              warp m0098i at 0x1B8 with the day3_walk 0x1B8 persist block and
              the day4 + day5 waypoint lists in one run, so the Day 4
              inventory carries into Day 5.

Checkpoints are story steps only: this route revisits rooms (m0167i, m0168i,
m0343i ...), and a story step can never fire out of order. The run keeps going
--linger frames after the last checkpoint to see what follows the last waypoint.
Disc 2 is needed from 0x258 on: set DISC2 (passed as --disc2-image). Fails like
the other day gates on a FATAL, CPU_BOUNDARY/REFUSED or [STOP] line or a missed
checkpoint, and on a reset (story < 0x80 or the title token). Unlike the shared
day2 harness it lets the story word go down when a room script writes a lower
value (m0035i writes 0x282 after 0x29F before entering m0271i), and it passes
--screenshot into the log dir. Other markers ([DAY1_SHIM], STUB:, "not
ported") do not stop it; laterdays_room_table.py counts them.
"""
import argparse
import os
import subprocess
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from day2_route_regress import RE_BAD, RE_ROUTE, pad_script  # noqa: E402

TITLE_TOKEN = 0xA9400048          # the reset/title token (Game Over and soft reset land here)


def run_day5_segment(name: str, seg: dict, a, disc: str, disc2: str | None) -> bool:
    """day2_route_regress.run_segment with two Day-5 changes:
      * the story word may go DOWN when a room script writes a lower value
        (m0035i module 2 writes 0x282 after 0x29F before its m0271i transfer);
        only a reset (story < 0x80 or the title token) fails the run;
      * the port gets --screenshot <log-dir>/<prefix>-<segment>.ppm, so its
        shutdown frame never goes to the default /tmp/pe-port-black.ppm.
    Everything else is the same: dev warp at frame 1000, the Cross/Circle pad
    script, a FATAL / CPU_BOUNDARY / [STOP] line fails, ordered checkpoints,
    linger after the last one."""
    log = os.path.join(a.log_dir, f"day5-{name}.log")
    shot = os.path.join(a.log_dir, f"day5-{name}.ppm")
    env = dict(os.environ, PE_PAD_SCRIPT=pad_script(), PE_DEV_MENU="1",
               PE_WARP_ROOM=seg["room"], PE_WARP_STORY=seg["story"], PE_WARP_AT="1000")
    env.update(seg.get("env", {}))
    cmd = [a.bin, "--headless", "--route-pad", "--hand-over-at", "1000",
           "--max-frames", str(seg["frames"]), "--screenshot", shot, "--disc-image", disc]
    if disc2:
        cmd += ["--disc2-image", disc2]
    cps, idx, frame, token, story, fail, drops = seg["checkpoints"], 0, 0, 0, 0, None, []
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
            prev = story
            frame, token, story = int(m.group(1)), int(m.group(2), 16), int(m.group(3), 16)
            if frame < 1000 or story >= 0x1000:
                continue                          # boot prefix before the warp
            if token == TITLE_TOKEN or story < 0x80:
                fail = f"game reset/game over: token {token:08X} story 0x{prev:X} -> 0x{story:X}"
                break
            if story < prev:
                drops.append(f"0x{prev:X}->0x{story:X}@{frame}")
                print(f"REGRESS note {name}: story written down 0x{prev:X} -> 0x{story:X} by the room script "
                      f"frame={frame} token={token:08X}", flush=True)
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
              f"elapsed={dt:.0f}s drops={','.join(drops) or 'none'} log={log}")
        return True
    print(f"REGRESS FAIL {name} at '{cps[idx][2] if idx < len(cps) else 'end'}' last frame={frame} "
          f"token={token:08X} story={story:X} reason={fail or 'run ended before checkpoint'} "
          f"drops={','.join(drops) or 'none'} log={log}")
    return False

HERE = os.path.dirname(os.path.abspath(__file__))


def waypoints(name: str) -> list[str]:
    out = []
    with open(os.path.join(HERE, name)) as f:
        for line in f:
            line = line.strip()
            if line and not line.startswith("#"):
                out.append(line.split("   (")[0].strip())
    return out


DAY5_STORY = [0x220, 0x228, 0x238, 0x240, 0x248, 0x250, 0x254, 0x258, 0x260, 0x278, 0x2A4, 0x2D0]
DAY4_STORY = [0x1C0, 0x1D0, 0x1E8, 0x1F0, 0x200, 0x210, 0x218]


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--bin", default="build/pcbuild-gcc/parasite-eve-port")
    ap.add_argument("--log-dir", default="build/day5-route-regress")
    ap.add_argument("--persist", required=True,
                    help="persist block: 0x218 (day4_walk) or, with --with-day4, 0x1B8 (day3_walk)")
    ap.add_argument("--with-day4", action="store_true", help="start at the Day 4 entry (m0098i, 0x1B8)")
    ap.add_argument("--frames", type=int, default=300000)
    ap.add_argument("--linger", type=int, default=20000,
                    help="frames to keep running after the last checkpoint")
    ap.add_argument("--persist-dump", default="",
                    help="PE_D3_PERSIST_DUMP directory (per story step and per room)")
    ap.add_argument("--room", default="", help="start leg: warp room instead of m0352i (e.g. m0192i)")
    ap.add_argument("--story", default="", help="start leg: warp story (hex) with --room")
    ap.add_argument("--override", action="append", default=[],
                    help="replace the waypoint entry whose key (TOKEN@STORY[#VISIT/&MASK]) matches, "
                         "e.g. A8029048@254=x,z,r;...; an unmatched key is appended")
    ap.add_argument("--warp-disc", default="",
                    help="PE_WARP_DISC (2 = Disc 2 in the drive before the warp, for a start past 0x258)")
    a = ap.parse_args()
    disc = os.environ.get("DISC")
    if not disc or not os.path.exists(disc):
        print("REGRESS FAIL day5_walk: set DISC to the Disc 1 image")
        return 1
    if not os.environ.get("DISC2"):
        print("REGRESS FAIL day5_walk: set DISC2 (the route crosses the Disc 2 story boundary at 0x258)")
        return 1
    if not os.path.exists(a.persist):
        print(f"REGRESS FAIL day5_walk: no persist block {a.persist}")
        return 1
    os.makedirs(a.log_dir, exist_ok=True)
    wps = waypoints("day5_walk_waypoints.txt")
    stories = DAY5_STORY
    room, story = "m0352i", "218"
    if a.with_day4:
        wps = waypoints("day4_walk_waypoints.txt") + wps
        stories = DAY4_STORY + DAY5_STORY
        room, story = "m0098i", "1B8"
    if a.room:
        if a.with_day4 or not a.story:
            print("REGRESS FAIL day5_walk: --room needs --story and excludes --with-day4")
            return 1
        room, story = a.room, a.story
        stories = [s for s in stories if s > int(a.story, 16)]
    for ov in a.override:
        key = ov.split("=", 1)[0]
        hit = [i for i, e in enumerate(wps) if e.split("=", 1)[0] == key]
        if hit:
            for i in hit:
                wps[i] = ov
        else:
            wps.append(ov)
        print(f"day5_route_regress: override {key} ({'replaced' if hit else 'appended'})", flush=True)
    checkpoints = [("story>=", s, f"story 0x{s:X}") for s in stories]
    env = {"PE_ROUTE_WAYPOINTS": "|".join(wps), "PE_CHEAT_KEYS": "1010:F3",
           "PE_ROUTE_MAXHP": "300", "PE_ONE_HIT_KILLS": "1",
           "PE_WARP_PERSIST_FILE": os.path.abspath(a.persist)}
    if a.warp_disc:
        env["PE_WARP_DISC"] = a.warp_disc
    if a.persist_dump:
        os.makedirs(a.persist_dump, exist_ok=True)
        env["PE_D3_PERSIST_DUMP"] = os.path.abspath(a.persist_dump)
    seg = dict(room=room, story=story, frames=a.frames, checkpoints=checkpoints, env=env,
               linger=a.linger)
    name = "day5_walk_with_day4" if a.with_day4 else (f"day5_walk_from_{room}_{story}" if a.room else "day5_walk")
    print(f"day5_route_regress: {len(wps)} waypoint entries, warp {room} story 0x{story}, "
          f"{len(checkpoints)} story checkpoints, frames={a.frames} linger={a.linger}", flush=True)
    return 0 if run_day5_segment(name, seg, a, disc, os.environ.get("DISC2")) else 1


if __name__ == "__main__":
    sys.exit(main())
