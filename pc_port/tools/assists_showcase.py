#!/usr/bin/env python3
"""Assists showcase capture (pe_plat/enhance.h, docs/port/ENHANCEMENTS.md).

Deterministic headless Day-1 autopilot runs (--route-pad, native speed), each
with one assist set through its env override, dumping every emulated VBlank of
one window (PE_SHOT_VSYNC) plus the port's own audio (PE_AUDIO_WAV, emulated
time).  The windows come from the baseline Aya trace (PE_AYA_TRACE, which
also logs HP / EXP / BP): the first Eve fight (M0023I, Aya is hit three times,
HP 45 -> 42 -> 35 -> 28) and the rat fight (M0011I, one hit 28 -> 25, EXP +2,
BP +3).  vbl == 2 * frame - 2 on this route.

  eve_off / eve_god     the same Eve-fight window with God Mode off / on
  rat_off / rat_1hit    the rat fight with 1-Hit Kills off / on
  rat_exp3              the rat fight with EXP/BP x3 (results screen)
  menu                  the dev menu ASSISTS page opened in the field (overlay
                        burned in with PE_SHOT_OVERLAY=1): all rows OFF, then
                        each toggled once and back

Writes <out>/<name>/{v*.ppm, audio.wav, run.log, meta.json}; encode with
assists_showcase_encode.sh.
"""
import argparse
import json
import os
import re
import shutil
import subprocess
import sys

DISC_DEFAULT = "rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin"
RE_AYA = re.compile(r"AYA frame=(\d+) token=([0-9A-F]{8}) d1a0=([0-9A-F]+) pos=(-?\d+),(-?\d+) vbl=(\d+) hp=(-?\d+) exp=(\d+) bp=(\d+)")

EVE = (10600, 11350)          # frames: three hits land ~10825 / 11055 / 11280, battle ends 11590
RAT = (18880, 19430)          # frames: battle 18905..19370, results ~19285, exit 19400
MENU_AT = 9380                # M0023I field, before the scripted Eve fight starts at 10255


def menu_keys(t):
    keys, marks = [], {}
    def press(name, gap=12):
        nonlocal t
        keys.append(f"{t}:{name}")
        t += gap
    press("DEV_MENU", 30)
    for _ in range(7): press("DOWN", 8)          # root row 8: ASSISTS
    press("ENTER", 90); marks["page"] = t - 90    # ASSISTS page, all OFF
    press("ENTER", 50)                            # 1-HIT KILLS ON
    press("DOWN", 8); press("ENTER", 50)          # GOD MODE ON
    press("DOWN", 8); press("ENTER", 50)          # EXP/BP x2
    press("ENTER", 50)                            # x3
    press("ENTER", 40)                            # back to x1
    press("UP", 8); press("ENTER", 30)            # god off
    press("UP", 8); press("ENTER", 40)            # 1-hit off
    marks["allok"] = t
    press("BACK", 20); press("BACK", 30)
    marks["closed"] = t
    return keys, marks, t + 30


RUNS = {
    "eve_off":  ({}, EVE),
    "eve_god":  ({"PE_GOD_MODE": "1"}, EVE),
    "rat_off":  ({}, RAT),
    "rat_1hit": ({"PE_ONE_HIT_KILLS": "1"}, RAT),
    "rat_exp3": ({"PE_EXP_MULT": "3"}, RAT),
    "menu":     ({}, (MENU_AT - 20, None)),
}


def run(a, name):
    extra, (first, last) = RUNS[name]
    d = os.path.join(a.out, name)
    shutil.rmtree(d, ignore_errors=True)
    os.makedirs(d)
    env = dict(os.environ, PE_AYA_TRACE="5", PE_ENH_CONFIG=os.path.join(d, "enh.conf"),
               PE_AUDIO_WAV=os.path.join(d, "audio.wav"))
    for v in ("PE_MOVE_SPEED", "PE_ALWAYS_RUN", "PE_ONE_HIT_KILLS", "PE_GOD_MODE", "PE_EXP_MULT",
              "PE_DEV_MENU", "PE_CHEATS", "DISPLAY", "WAYLAND_DISPLAY"):
        env.pop(v, None)
    env.update(extra)
    marks = {}
    if name == "menu":
        keys, marks, last = menu_keys(MENU_AT)
        env.update(PE_DEV_MENU="1", PE_SHOT_OVERLAY="1", PE_CHEAT_KEYS=",".join(keys))
    first_vbl = 2 * first - 2
    env["PE_SHOT_VSYNC"] = f"{d}:{first_vbl}:1"
    cmd = [a.bin, "--headless", "--route-pad", "--max-frames", str(last), "--disc-image", a.disc]
    log = os.path.join(d, "run.log")
    with open(log, "w") as lf:
        rc = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=lf, env=env, timeout=3600).returncode
    tr = [dict(frame=int(f), token=int(tk, 16), battle=int(dd, 16) & 2, x=int(x), z=int(z), vbl=int(v),
               hp=int(hp), exp=int(e), bp=int(b)) for f, tk, dd, x, z, v, hp, e, b in
          RE_AYA.findall(open(log, errors="replace").read())]
    win = [t for t in tr if first <= t["frame"] <= last]
    meta = dict(name=name, rc=rc, env=extra, first_frame=first, last_frame=last, first_vbl=first_vbl,
                frames=len([f for f in os.listdir(d) if f.endswith(".ppm")]), marks=marks,
                hp=[t["hp"] for t in win], exp=[t["exp"] for t in win], bp=[t["bp"] for t in win],
                battle=[t["battle"] for t in win], vbl=[t["vbl"] for t in win], frame=[t["frame"] for t in win])
    json.dump(meta, open(os.path.join(d, "meta.json"), "w"))
    print(json.dumps({k: meta[k] for k in ("name", "rc", "first_vbl", "frames")}), flush=True)
    return rc


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--bin", required=True)
    ap.add_argument("--disc", default=os.environ.get("DISC", DISC_DEFAULT))
    ap.add_argument("--out", required=True)
    ap.add_argument("--only", default="")
    a = ap.parse_args()
    a.bin = os.path.abspath(a.bin); a.disc = os.path.abspath(a.disc); a.out = os.path.abspath(a.out)
    names = a.only.split(",") if a.only else list(RUNS)
    rc = 0
    for n in names:
        rc |= run(a, n)
    sys.exit(rc)


if __name__ == "__main__":
    main()
