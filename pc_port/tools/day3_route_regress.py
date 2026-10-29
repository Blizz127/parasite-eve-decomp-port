#!/usr/bin/env python3
"""Day-3 route regression (dev harness): the Day 3 story stretches that the
game plays through its OWN room scripts, checked against ordered checkpoints.

Same method as day2_route_regress.py (shared harness): one dev warp
(PE_WARP_ROOM / PE_WARP_STORY) into a room whose script then drives the story
by itself; the pad alternates Cross and Circle; every later room change and
story write comes from the room scripts, not from the harness.

Most of Day 3 needs the player to walk or fight (warping m0092i at 0x140 or
m0360i at 0x140 just waits; m0123i at 0x190 is a fight the mashing pad loses),
so the gate covers the script-driven stretches only:

  day3_start  warp m0093i story 0x140: its script writes 0x148.
  day3_mid    warp m0108i story 0x188: its script writes 0x190.
  day3_end    warp m0352i story 0x1B0 (Day-4 hop): its room_transfer enters
              m0098i (Day 4 entry), whose script writes 0x1B8.
  day3_walk   warp m0093i story 0x140 (Day 3 start), then a recorded walk
              to story 0x1B8 in one run: the route pilot
              (pc_port/src/route_day3_pilot.h) replays the committed waypoint
              list day3_walk_waypoints.txt (room token, story, optional
              persist[0x2C] mask; world X,Z points, Cross holds, timeouts) and
              fights the random battles (aim shim + Cross). It walks the
              opening rooms (0x148 to 0x178), the precinct and the hospital (two flag examines), wins the
              hospital battles and the m0123i fight, and must reach story
              0x1B8. Three harness fixtures, because the warp keeps Day-1 stats:
              PE_ROUTE_MAXHP=300 (max HP in battle), cheat F3 (infinite
              ammo) and the 1-Hit Kills assist PE_ONE_HIT_KILLS=1. With the
              Day-1 pistol Aya hits the m0123i Kerberos heads (200/300/400 HP,
              the centre heals the others) for ~3 HP, so the fight cannot be
              won. The assist keeps retail's hit and only raises the final
              damage to the head's remaining HP, so retail's own death and
              victory path runs (docs/port/ENHANCEMENTS.md). The waypoints are
              the harness's own coordinates; no disc bytes. Runs by default
              since the whole walk passed (2026-10-09: story 0x1B0 at frame
              ~61700 from 0x140). After the fight it walks on through
              0x1B8: m0123i returns to m0044i at 0x1B0 with persist[0x2C]
              bit 0x400000 already set (by the m0044i Baker scene before the
              fight), so m0044i module 1 starts modules 0/4/2/7. Walking north
              into module 7's trigger polygon (x -96..1830, z 912..2541)
              posts mailbox payload 0xC to module 4, whose cutscene transfers
              to m0352i; m0352i enters m0098i, which writes 0x1B8 (~frame
              65000). A warp into m0044i at 0x1B0 starts with the bit clear and
              replays the Baker scene instead, so this stretch is only
              reachable by the continuous walk. The 1-Hit Kills assist is off
              by default; only this segment's env sets it.
A segment fails on a FATAL, CPU_BOUNDARY/REFUSED or [STOP] line, a story drop,
a checkpoint firing out of order, or a missed checkpoint.
"""
import argparse
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from day2_route_regress import run_segment  # noqa: E402

SEGMENTS = {
    "day3_start": dict(room="m0093i", story="140", frames=8000, checkpoints=[
        ("token", 0xA80091C8, "m0093i (story 0x140)"),
        ("story>=", 0x148, "m0093i script writes story 0x148"),
    ]),
    "day3_mid": dict(room="m0108i", story="188", frames=8000, checkpoints=[
        ("token", 0xA8020448, "m0108i (story 0x188)"),
        ("story>=", 0x190, "m0108i script writes story 0x190"),
    ]),
    "day3_end": dict(room="m0352i", story="1B0", frames=8000, checkpoints=[
        ("token", 0xA8065148, "m0352i Day-4 hop (story 0x1B0)"),
        ("token", 0xA8009448, "m0098i Day 4 entry, by m0352i's room_transfer"),
        ("story>=", 0x1B8, "m0098i script writes story 0x1B8"),
    ]),
}

WALK_FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "day3_walk_waypoints.txt")


def walk_waypoints() -> str:
    with open(WALK_FILE) as f:
        return "|".join(l.strip() for l in f if l.strip() and not l.startswith("#"))


SEGMENTS["day3_walk"] = dict(room="m0093i", story="140", frames=90000, checkpoints=[
    ("token", 0xA80091C8, "m0093i Day 3 start (story 0x140)"),
    ("story>=", 0x148, "m0093i script writes 0x148"),
    ("token", 0xA8009348, "m0096i"),
    ("token", 0xA80093C8, "m0097i"),
    ("story>=", 0x160, "story 0x160"),
    ("token", 0xA80290C8, "m0191i"),
    ("story>=", 0x170, "m0191i writes 0x170"),
    ("token", 0xA80434C8, "m0239i"),
    ("story>=", 0x178, "story 0x178"),
    ("token", 0xA80033C8, "m0037i precinct (map index 7)"),
    ("story>=", 0x180, "m0037i entry scene writes 0x180"),
    ("token", 0xA80201C8, "m0103i hospital entrance"),
    ("story>=", 0x188, "m0103i door writes 0x188"),
    ("token", 0xA80203C8, "m0107i (battle)"),
    ("story>=", 0x190, "m0108i scene writes 0x190"),
    ("token", 0xA80210C8, "m0111i (body examine)"),
    ("token", 0xA80094C8, "m0099i (object examine)"),
    ("token", 0xA80212C8, "m0115i vision"),
    ("token", 0xA8021348, "m0116i"),
    ("token", 0xA8004248, "m0044i scene"),
    ("token", 0xA8022148, "m0122i"),
    ("token", 0xA80221C8, "m0123i fight"),
    ("story>=", 0x1B0, "m0123i fight won, story 0x1B0"),
    ("token", 0xA8065148, "m0352i, by m0044i module 4 (bit 0x400000 set, story 0x1B0)"),
    ("token", 0xA8009448, "m0098i Day 4 entry, by m0352i's room_transfer"),
    ("story>=", 0x1B8, "m0098i script writes story 0x1B8"),
], env_fn=lambda: {"PE_ROUTE_WAYPOINTS": walk_waypoints(), "PE_CHEAT_KEYS": "1010:F3",
                   "PE_ROUTE_MAXHP": "300", "PE_ONE_HIT_KILLS": "1"})


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--bin", default="build/pcbuild-disc/parasite-eve-port")
    ap.add_argument("--segment", choices=sorted(SEGMENTS), action="append")
    ap.add_argument("--log-dir", default="build/lanes/day3")
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
    names = a.segment or [n for n, s in SEGMENTS.items() if not s.get("opt_in")]
    for n in names:
        if "env_fn" in SEGMENTS[n]:
            SEGMENTS[n]["env"] = SEGMENTS[n].pop("env_fn")()
        if n == "day3_walk":          # persist block per story step: day4_route_regress --persist
            SEGMENTS[n]["env"]["PE_D3_PERSIST_DUMP"] = os.path.abspath(os.path.join(a.log_dir, "persist"))
            os.makedirs(SEGMENTS[n]["env"]["PE_D3_PERSIST_DUMP"], exist_ok=True)
            SEGMENTS[n]["linger"] = 120       # the 0x1B8 block is written just after the story step
    ok = all([run_segment(n, SEGMENTS[n], a, disc, os.environ.get("DISC2"), prefix="day3")
              for n in names])
    print("DAY3_ROUTE=" + ("PASS" if ok else "FAIL"))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
