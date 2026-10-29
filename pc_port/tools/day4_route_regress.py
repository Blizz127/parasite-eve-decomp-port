#!/usr/bin/env python3
"""Day 4 route regression (harness-only).

  day4_walk   warp m0098i story 0x1B8 (Day 4 entry) with the persist block
              that the day3_walk segment dumped at 0x1B8 (--persist; the
              day3_walk segment writes <log-dir>/persist/persist_1B8.bin), then
              the recorded walk day4_walk_waypoints.txt to story 0x220 in one
              run: the Chinatown/hospital rooms, the m0326i..m0332i
              battles, the m0129i/m0132i elevators, the m0152i cabinet, the
              m0156i/m0380i roof fight, the timed m0381i escape (runs, + suffix)
              -> m0157i, the precinct m0046i -> m0041i scene -> m0352i, which
              writes 0x220 (~frame 97600). Same fixtures as day3_walk
              (PE_ROUTE_MAXHP=300, cheat F3, PE_ONE_HIT_KILLS=1). The persist
              file is the harness's own runtime state from the same gate run;
              nothing is committed. Checkpoints: every first room entry and
              every story step of the 2026-10-09 through run, in order.
A segment fails on a FATAL, CPU_BOUNDARY/REFUSED or [STOP] line, a story drop,
a checkpoint firing out of order, or a missed checkpoint.
"""
import argparse
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from day2_route_regress import run_segment  # noqa: E402

WALK_FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "day4_walk_waypoints.txt")


def walk_waypoints() -> str:
    out = []
    with open(WALK_FILE) as f:
        for l in f:
            l = l.strip()
            if l and not l.startswith("#"):
                out.append(l.split("   (")[0].strip())
    return "|".join(out)


CHECKPOINTS = [
    ("token", 0xA8009448, "m0098i"),
    ("token", 0xA80094C8, "m0099i"),
    ("token", 0xA8020048, "m0100i"),
    ("token", 0xA8021348, "m0116i"),
    ("token", 0xA80212C8, "m0115i"),
    ("token", 0xA80220C8, "m0121i"),
    ("token", 0xA80213C8, "m0117i"),
    ("token", 0xA8000048, "m0000i"),
    ("story>=", 0x1C0, "story 0x1C0"),
    ("token", 0xA80034C8, "m0039i"),
    ("story>=", 0x1C8, "story 0x1C8"),
    ("token", 0xA80222C8, "m0125i"),
    ("story>=", 0x1CC, "story 0x1CC"),
    ("story>=", 0x1D0, "story 0x1D0"),
    ("token", 0xA8022348, "m0126i"),
    ("token", 0xA80223C8, "m0127i"),
    ("token", 0xA8022448, "m0128i"),
    ("story>=", 0x1D3, "story 0x1D3"),
    ("token", 0xA80230C8, "m0131i"),
    ("story>=", 0x1D6, "story 0x1D6"),
    ("token", 0xA8062048, "m0320i"),
    ("token", 0xA80620C8, "m0321i"),
    ("token", 0xA8023248, "m0134i"),
    ("token", 0xA8062148, "m0322i"),
    ("story>=", 0x1D9, "story 0x1D9"),
    ("story>=", 0x1DC, "story 0x1DC"),
    ("token", 0xA80621C8, "m0323i"),
    ("token", 0xA80634C8, "m0339i"),
    ("token", 0xA80622C8, "m0325i"),
    ("token", 0xA80623C8, "m0327i"),
    ("token", 0xA8062448, "m0328i"),
    ("token", 0xA8062348, "m0326i"),
    ("token", 0xA80624C8, "m0329i"),
    ("token", 0xA8063148, "m0332i"),
    ("token", 0xA8024248, "m0144i"),
    ("story>=", 0x1E8, "story 0x1E8"),
    ("token", 0xA8024148, "m0142i"),
    ("token", 0xA80224C8, "m0129i"),
    ("token", 0xA8023148, "m0132i"),
    ("token", 0xA8024348, "m0146i"),
    ("token", 0xA80243C8, "m0147i"),
    ("token", 0xA80663C8, "m0367i"),
    ("token", 0xA8023348, "m0136i"),
    ("story>=", 0x1EB, "story 0x1EB"),
    ("story>=", 0x1EC, "story 0x1EC"),
    ("token", 0xA8024448, "m0148i"),
    ("token", 0xA80244C8, "m0149i"),
    ("story>=", 0x1EE, "story 0x1EE"),
    ("token", 0xA8025048, "m0150i"),
    ("story>=", 0x1F0, "story 0x1F0"),
    ("token", 0xA8025148, "m0152i"),
    ("token", 0xA80251C8, "m0153i"),
    ("token", 0xA8025248, "m0154i"),
    ("story>=", 0x1F4, "story 0x1F4"),
    ("token", 0xA80470C8, "m0271i"),
    ("story>=", 0x1F6, "story 0x1F6"),
    ("token", 0xA80252C8, "m0155i"),
    ("story>=", 0x1F8, "story 0x1F8"),
    ("token", 0xA8025348, "m0156i"),
    ("token", 0xA8068048, "m0380i"),
    ("token", 0xA80680C8, "m0381i"),
    ("story>=", 0x1FC, "story 0x1FC"),
    ("token", 0xA80253C8, "m0157i"),
    ("token", 0xA8025448, "m0158i"),
    ("story>=", 0x200, "story 0x200"),
    ("token", 0xA80254C8, "m0159i"),
    ("token", 0xA8026048, "m0160i"),
    ("story>=", 0x208, "story 0x208"),
    ("token", 0xA80032C8, "m0035i"),
    ("token", 0xA8004348, "m0046i"),
    ("story>=", 0x210, "story 0x210"),
    ("story>=", 0x216, "story 0x216"),
    ("token", 0xA80041C8, "m0043i"),
    ("token", 0xA8004148, "m0042i"),
    ("token", 0xA80040C8, "m0041i"),
    ("token", 0xA8065148, "m0352i"),
    ("story>=", 0x218, "story 0x218"),
    ("story>=", 0x220, "story 0x220"),
]


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--bin", default="build/pcbuild-port/parasite-eve-port")
    ap.add_argument("--log-dir", default="build/day4-route-regress")
    ap.add_argument("--persist", required=True,
                    help="persist block at story 0x1B8 (day3_walk writes <log-dir>/persist/persist_1B8.bin)")
    ap.add_argument("--frames", type=int, default=130000)
    a = ap.parse_args()
    disc = os.environ.get("DISC")
    if not disc or not os.path.exists(disc):
        print("REGRESS FAIL day4_walk: set DISC to the Disc 1 image")
        return 1
    if not os.path.exists(a.persist):
        print(f"REGRESS FAIL day4_walk: no persist block {a.persist} (run day3_walk first)")
        return 1
    os.makedirs(a.log_dir, exist_ok=True)
    seg = dict(room="m0098i", story="1B8", frames=a.frames, checkpoints=CHECKPOINTS,
               env={"PE_ROUTE_WAYPOINTS": walk_waypoints(), "PE_CHEAT_KEYS": "1010:F3",
                    "PE_ROUTE_MAXHP": "300", "PE_ONE_HIT_KILLS": "1",
                    "PE_WARP_PERSIST_FILE": os.path.abspath(a.persist)})
    return 0 if run_segment("day4_walk", seg, a, disc, os.environ.get("DISC2"), prefix="day4") else 1


if __name__ == "__main__":
    sys.exit(main())
