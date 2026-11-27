#!/usr/bin/env python3
"""Movement-speed showcase capture (pe_plat/enhance.h, docs/port/ENHANCEMENTS.md).

One deterministic headless run per segment (100 %, 150 %, 200 %, 200 % + Always
Run): Day-1 autopilot boot, pad hand-over, dev-menu warp to FIELD START, the
Options > Enhancements rows set through the dev menu (visible in the capture via
PE_SHOT_OVERLAY=1), then the same stretch: walk, run through the lobby door and
down the corridor.  Pass 1 probes each segment (Aya trace with VBlank stamps);
pass 2 re-runs it with the hold ended so every speed covers the same ground, and
dumps every emulated VBlank (PE_SHOT_VSYNC) plus the port's own audio
(PE_AUDIO_WAV, emulated time).  The game runs at native speed.

Writes <out>/seg_<name>/{v*.ppm, audio.wav, run.log, meta.json}; encode with
enh_showcase_encode.sh.
"""
import argparse
import json
import os
import re
import shutil
import subprocess
import sys

DISC_DEFAULT = "rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin"
RE_AYA = re.compile(r"AYA frame=(\d+) token=([0-9A-F]{8}) d1a0=([0-9A-F]+) pos=(-?\d+),(-?\d+) vbl=(\d+)")
UP, CIRCLE, CROSS = 0x10, 0x2000, 0x4000
LOBBY, CORRIDOR = 0xA8000148, 0xA80001C8
# name, menu presses on the speed row (from 100 %), always run, walk presents
SEGMENTS = [("100", 0, False, 60), ("150", 2, False, 40), ("200", 4, False, 30), ("200ar", 4, True, 0)]
CORRIDOR_RUN_100 = 120            # presents of corridor running at 100 % (~2700 units)


def keys_for(a, presses, always_run):
    keys, t = [], a.warp_at
    marks = {}
    def press(name, gap=2):
        nonlocal t
        keys.append(f"{t}:{name}")
        t += gap
    press("F8")
    for _ in range(5): press("DOWN")
    press("ENTER"); press("ENTER"); press("ENTER")          # Day 1 > FIELD START
    t += a.settle
    marks["menu"] = t
    if presses or always_run:
        press("DEV_MENU", 14)
        for _ in range(6): press("DOWN", 4)
        press("ENTER", 14); press("ENTER", 24)             # OPTIONS > ENHANCEMENTS
        for _ in range(presses): press("ENTER", 24)
        if always_run:
            press("DOWN", 14); press("ENTER", 30)
        t += 16
        for _ in range(3): press("BACK", 4)
    marks["closed"] = t
    return keys, marks


def run(a, seg, out, probe, run_len=None):
    name, presses, always_run, walk = seg
    keys, marks = keys_for(a, presses, always_run)
    rel = lambda f: f - a.hand_over
    taps = []
    t = a.warp_at + 60
    while t < marks["menu"] - 20:
        taps += [f"{rel(t)}:{0xFFFF & ~CROSS:04X}", f"{rel(t + 4)}:FFFF"]
        t += 40
    s0 = marks["closed"] + 20
    hold_end = s0 + (run_len if run_len else 400)
    if walk:
        taps += [f"{rel(s0)}:{0xFFFF & ~UP:04X}", f"{rel(s0 + walk)}:{0xFFFF & ~(UP | CIRCLE):04X}"]
    else:
        taps += [f"{rel(s0)}:{0xFFFF & ~(UP if always_run else UP | CIRCLE):04X}"]
    taps += [f"{rel(hold_end)}:FFFF"]
    end = hold_end + 45
    d = os.path.join(out, f"seg_{name}")
    shutil.rmtree(d, ignore_errors=True)
    os.makedirs(d)
    env = dict(os.environ, PE_CHEAT_KEYS=",".join(keys), PE_PAD_SCRIPT=",".join(taps),
               PE_AYA_TRACE="1", PE_ENH_CONFIG=os.path.join(d, "enh.conf"), PE_DEV_MENU="1",
               PE_SHOT_OVERLAY="1")
    for v in ("PE_MOVE_SPEED", "PE_ALWAYS_RUN", "PE_CHEATS", "DISPLAY", "WAYLAND_DISPLAY"):
        env.pop(v, None)
    if not probe:
        env["PE_SHOT_VSYNC"] = f"{d}:{a.first_vbl[name]}:1"
        env["PE_AUDIO_WAV"] = os.path.join(d, "audio.wav")
    cmd = [a.bin, "--headless", "--route-pad", "--hand-over-at", str(a.hand_over),
           "--max-frames", str(end), "--disc-image", a.disc]
    log = os.path.join(d, "run.log")
    with open(log, "w") as lf:
        rc = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=lf, env=env, timeout=3600).returncode
    tr = [(int(f), int(tk, 16), int(x), int(z), int(v)) for f, tk, _, x, z, v in
          RE_AYA.findall(open(log, errors="replace").read())]
    return rc, tr, marks, s0, hold_end, d


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--bin", required=True)
    ap.add_argument("--disc", default=os.environ.get("DISC", DISC_DEFAULT))
    ap.add_argument("--out", required=True)
    ap.add_argument("--hand-over", type=int, default=1100)
    ap.add_argument("--warp-at", type=int, default=1160)
    ap.add_argument("--settle", type=int, default=300)
    ap.add_argument("--only", default="")
    a = ap.parse_args()
    os.makedirs(a.out, exist_ok=True)
    segs = [s for s in SEGMENTS if not a.only or s[0] in a.only.split(",")]
    a.first_vbl = {}
    for seg in segs:
        name = seg[0]
        speed = 2.0 if name.startswith("200") else int(name) / 100
        rc, tr, marks, s0, _, d = run(a, seg, a.out, probe=True)
        cross = next((p[0] for p in tr if p[0] > s0 and p[1] == CORRIDOR), None)
        if rc != 0 or cross is None:
            sys.exit(f"{name}: probe failed rc={rc} cross={cross} (see {d}/run.log)")
        run_len = cross - s0 + round(CORRIDOR_RUN_100 / speed)
        vb = {p[0]: p[4] for p in tr}
        a.first_vbl[name] = vb.get(marks["menu"] - 30, min(vb.values()))
        rc, tr2, marks, s0, hold_end, d = run(a, seg, a.out, probe=False, run_len=run_len)
        vb2 = {p[0]: p[4] for p in tr2}
        stop = next((p for p in tr2 if p[0] >= hold_end), tr2[-1])
        meta = dict(name=name, rc=rc, first_vbl=a.first_vbl[name], menu_vbl=vb2.get(marks["menu"]),
                    closed_vbl=vb2.get(marks["closed"]), start_vbl=vb2.get(s0),
                    cross_vbl=vb2.get(cross), stop_vbl=vb2.get(hold_end), stop_pos=stop[2:4],
                    start_pos=next((p[2:4] for p in tr2 if p[0] >= s0), None),
                    probe_matches=[p for p in tr if p[0] <= cross] == [p for p in tr2 if p[0] <= cross])
        json.dump(meta, open(os.path.join(d, "meta.json"), "w"), indent=1)
        print(json.dumps(meta))
        if rc != 0 or not meta["probe_matches"]:
            sys.exit(f"{name}: capture run diverged from its probe or failed (rc={rc})")


if __name__ == "__main__":
    main()
