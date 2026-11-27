#!/usr/bin/env python3
"""Movement-speed sweep over the Day-1 warp rooms (pe_plat/enhance.h).

Each run boots with the Day-1 autopilot, hands the pad over, warps through the
dev menu (PE_DEV_MENU=1), settles (Cross taps dismiss entry text) and sets the
config in Options > Enhancements.  Aya's position, room token and battle bit are
logged every --every presents (PE_AYA_TRACE).  The boot, warp and settle are the
same in every config and are checked to trace identically.

Field rooms: one fresh run per direction (8: the 4 axes and 4 diagonals), a
Circle-held run long enough to reach a wall or a door (--hold presents at
200 %, twice that at 100 %).  Per direction, in the warp room, against 100 %:
  door      the first room change is to the same room, or there is none at
            both speeds (no door or transfer trigger skipped or added)
  walls     every settled sample lies inside the 100 % bounding box grown by
            --tol (a tunnelled wall would widen it); the fade before a room
            change is exempt (Aya keeps moving during it, twice as far at 2x)
  rest      without a door, she comes to rest within --tol of the 100 % point
Always Run (200r: d-pad only, 200 % + Always Run) is checked the same way on
the --ar-dirs directions.

Battle rooms (the battle bit is set when the holds start): battle movement is
not scaled, so the same unscaled 8-hold pattern must trace IDENTICALLY at 100 %
and 200 % (Circle held) and at 100 % and 200 % + Always Run (d-pad only), for
every sample while the battle runs.  This is the battle-timing check.

Prints one row per room and exits non-zero on any failure.
"""
import argparse
import concurrent.futures as cf
import os
import re
import subprocess
import sys

DISC_DEFAULT = "rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin"
# Dev-menu Day-1 warp list order (pc_port/platform/plat_cheats.c k_warps).
TOKENS = [0xA8000148, 0xA80002C8, 0xA80011C8, 0xA80021C8, 0xA8002348, 0xA80023C8,
          0xA8002448, 0xA80030C8, 0xA8003148, 0xA80031C8, 0xA8003248, 0xA80654C8,
          0xA8003348]
# Active-low pad masks: Up/Right/Down/Left (0x10/0x20/0x40/0x80), Circle 0x2000.
DIRS = [0x10, 0x20, 0x40, 0x80, 0x30, 0xC0, 0x90, 0x60]
CIRCLE, CROSS = 0x2000, 0x4000
RE_AYA = re.compile(r"AYA frame=(\d+) token=([0-9A-F]{8}) d1a0=([0-9A-F]+) pos=(-?\d+),(-?\d+)")
# name: (speed presses from 100 %, always run, Circle held, time-scaled 2x hold, re-warp per hold)
CONFIGS = {"100": (0, False, True, True, True), "200": (4, False, True, False, True),
           "200r": (4, True, False, False, True),
           "b100": (0, False, True, False, False), "b200": (4, False, True, False, False),
           "b100n": (0, False, False, False, False), "b200r": (4, True, False, False, False)}
FIELD, BATTLE = ("100", "200", "200r"), ("b100", "b200", "b100n", "b200r")


def run_one(a, idx, cfg, workdir, dirs=None):
    presses, always_run, circle, scaled, rewarp = CONFIGS[cfg]
    dirs = list(range(len(DIRS))) if dirs is None else dirs
    keys, taps = [], []
    rel = lambda f: f - a.hand_over          # PE_PAD_SCRIPT counts from the hand-over
    def warp(t):
        seq = ["F8"] + ["DOWN"] * 5 + ["ENTER", "ENTER"] + ["DOWN"] * idx + ["ENTER"]
        for i, k in enumerate(seq):
            keys.append(f"{t + 2 * i}:{k}")
        return t + 2 * len(seq)
    def cross(t0, t1):
        for t in range(t0, t1, 40):
            taps.extend([f"{rel(t)}:{0xFFFF & ~CROSS:04X}", f"{rel(t + 4)}:FFFF"])
    t = warp(a.warp_at)
    cross(t + 60, t + a.settle)
    t += a.settle
    seq = ["DEV_MENU"] + ["DOWN"] * 6 + ["ENTER", "ENTER"] + ["ENTER"] * presses \
        + ["DOWN"] + (["ENTER"] if always_run else []) + ["BACK"] * 3
    for i, k in enumerate(seq):
        keys.append(f"{t + 2 * i}:{k}")
    t += 2 * 20 + 20
    move0 = t
    hold = (a.hold if rewarp else a.battle_hold) * (2 if scaled else 1)
    segs = []
    for j in dirs:
        d = DIRS[j]
        if rewarp and segs:
            t = warp(t)
            cross(t + 90, t + a.rewarp_settle - 20)
            t += a.rewarp_settle
        m = d | CIRCLE if circle else d
        taps.extend([f"{rel(t)}:{0xFFFF & ~m:04X}", f"{rel(t + hold)}:FFFF"])
        segs.append((t, t + hold, t + hold + a.tail))
        t += hold + a.tail
    end = t + 10
    conf = os.path.join(workdir, f"enh_r{idx:02d}_{cfg}_{'-'.join(map(str, dirs))}.conf")
    if os.path.exists(conf):
        os.unlink(conf)
    env = dict(os.environ, PE_CHEAT_KEYS=",".join(keys), PE_PAD_SCRIPT=",".join(taps),
               PE_AYA_TRACE=str(a.every), PE_ENH_CONFIG=conf, PE_DEV_MENU="1")
    for v in ("PE_MOVE_SPEED", "PE_ALWAYS_RUN", "PE_CHEATS", "DISPLAY", "WAYLAND_DISPLAY"):
        env.pop(v, None)
    cmd = [a.bin, "--headless", "--route-pad", "--hand-over-at", str(a.hand_over),
           "--max-frames", str(end), "--disc-image", a.disc]
    tag = "" if len(dirs) == len(DIRS) else "_d" + "".join(map(str, dirs))
    log = os.path.join(workdir, f"r{idx:02d}_{cfg}{tag}.log")
    with open(log, "w") as lf:
        try:
            rc = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=lf, env=env,
                                timeout=a.timeout).returncode
        except subprocess.TimeoutExpired:
            rc = "timeout"
    text = open(log, errors="replace").read()
    tr = [(int(f), int(tk, 16), int(d, 16), int(x), int(z)) for f, tk, d, x, z in RE_AYA.findall(text)]
    want = ("MOVEMENT SPEED 200%" if presses else None, "ALWAYS RUN ON" if always_run else None)
    applied = all(w is None or w in text for w in want) and (presses or "MOVEMENT SPEED" not in text)
    return dict(rc=rc, warps=text.count("WARP:"), log=log, segs=segs, move0=move0,
                fatal=next((l for l in text.splitlines() if "FATAL" in l), ""),
                trace=tr, applied=applied, pre=[p for p in tr if a.warp_at < p[0] < move0])


def tokens_seq(trace):
    out = []
    for p in trace:
        if not out or out[-1] != p[1]:
            out.append(p[1])
    return out


def settled(trace, guard=24):
    """Samples not in the `guard` frames around a room-token change: the fade
    before a door (Aya keeps moving) and the token/position update skew."""
    ch = [q[0] for p, q in zip(trace, trace[1:]) if p[1] != q[1]]
    return [p for p in trace if all(not (c - guard <= p[0] <= c + 8) for c in ch)]


def at(trace, frame):
    best = None
    for p in trace:
        if p[0] <= frame:
            best = p
    return best


def window(r, j):
    s0, _, s2 = r["segs"][j]
    return [p for p in r["trace"] if s0 - 2 <= p[0] <= s2]


def live(r):
    return r["rc"] == 0 and not r["fatal"] and r["warps"] >= 1 and r["trace"] and r["applied"]


def first_room(r):
    """The hold's samples up to (and including) the first room change."""
    w = window(r, 0)
    out = []
    for p in w:
        out.append(p)
        if p[1] != w[0][1]:
            break
    return out


def judge_dir(a, base, r):
    """None if the 200 % run r matches the 100 % run base for one direction."""
    if r["pre"] != base["pre"]:
        return "diverged from 100% before the hold"
    w0, w1 = first_room(base), first_room(r)
    if not w0 or not w1:
        return "no samples"
    d0 = w0[-1][1] if w0[-1][1] != w0[0][1] else None
    d1 = w1[-1][1] if w1[-1][1] != w1[0][1] else None
    if d0 != d1:
        return f"first door {d1 and f'{d1:08X}'} != 100% {d0 and f'{d0:08X}'}"
    s0 = [p for p in settled(w0) if p[1] == w0[0][1] and not p[2] & 2]
    s1 = [p for p in settled(w1) if p[1] == w1[0][1] and not p[2] & 2]
    if s0:
        b = [min(p[3] for p in s0), max(p[3] for p in s0), min(p[4] for p in s0), max(p[4] for p in s0)]
        out = [p for p in s1 if not (b[0] - a.tol <= p[3] <= b[1] + a.tol and b[2] - a.tol <= p[4] <= b[3] + a.tol)]
        if out:
            return f"{out[0][3]},{out[0][4]} outside 100% box {b} (+{a.tol})"
    if d0 is None and s0 and s1 and max(abs(s0[-1][3] - s1[-1][3]), abs(s0[-1][4] - s1[-1][4])) > a.tol:
        return f"rest {s1[-1][3]},{s1[-1][4]} vs 100% {s0[-1][3]},{s0[-1][4]}"
    return None


def judge_field(a, runs):
    ok, notes = True, []
    for cfg in ("200", "200r"):
        bad, doors, travel, n = [], 0, 0, 0
        for j in range(len(DIRS)):
            if (cfg, j) not in runs:
                continue
            n += 1
            base, r = runs[("100", j)], runs[(cfg, j)]
            e = judge_dir(a, base, r)
            if e:
                bad.append(f"dir {j}: {e}")
            w = first_room(r)
            doors += bool(w and w[-1][1] != w[0][1])
            travel = max([travel] + [abs(p[3] - w[0][3]) + abs(p[4] - w[0][4]) for p in w])
        ok &= not bad
        notes.append(f"{cfg}: " + ("; ".join(bad[:2]) if bad else
                                   f"{n} dirs ok ({doors} through a door, max travel {travel})"))
    return ok, notes


def judge_battle(a, runs):
    ok, notes = True, []
    for x, y in (("b100", "b200"), ("b100n", "b200r")):
        bx = [p for p in runs[x]["trace"] if p[0] >= runs[x]["move0"]]
        stop = next((p[0] for p in bx if not p[2] & 2), None)
        bx = [p for p in bx if stop is None or p[0] < stop]
        by = [p for p in runs[y]["trace"] if p[0] >= runs[y]["move0"] and (stop is None or p[0] < stop)]
        moved = len({(p[3], p[4]) for p in bx})
        if bx != by:
            ok = False
            diff = next(((p, q) for p, q in zip(bx, by) if p != q), None)
            notes.append(f"{y} != {x} in battle: {diff}")
        else:
            notes.append(f"{y} == {x}: {len(bx)} battle samples identical, {moved} distinct positions")
    return ok, notes


def judge(a, idx, runs, battle):
    ok, notes = True, []
    for cfg, r in runs.items():
        if not live(r):
            ok = False
            notes.append(f"{cfg}: rc={r['rc']} warps={r['warps']} samples={len(r['trace'])} "
                         f"applied={r['applied']} {r['fatal'][:60]} ({r['log']})")
    if not ok:
        return False, notes
    k, n = (judge_battle if battle else judge_field)(a, runs)
    return ok and k, notes + n


def is_battle(r):
    return any(p[2] & 2 for p in r["trace"] if r["move0"] - 40 <= p[0] <= r["move0"] + 60)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--bin", default="build/pcbuild-port/parasite-eve-port")
    ap.add_argument("--disc", default=os.environ.get("DISC", DISC_DEFAULT))
    ap.add_argument("--targets", default="", help="comma list of warp indices (default all)")
    ap.add_argument("--hand-over", type=int, default=1100)
    ap.add_argument("--warp-at", type=int, default=1160)
    ap.add_argument("--settle", type=int, default=300)
    ap.add_argument("--rewarp-settle", type=int, default=240)
    ap.add_argument("--hold", type=int, default=240, help="field hold at 200 %% (100 %%: twice)")
    ap.add_argument("--battle-hold", type=int, default=120)
    ap.add_argument("--ar-dirs", default="0,5", help="directions also checked with Always Run")
    ap.add_argument("--tail", type=int, default=40)
    ap.add_argument("--every", type=int, default=2)
    ap.add_argument("--tol", type=int, default=96, help="position tolerance, world units")
    ap.add_argument("--jobs", type=int, default=3)
    ap.add_argument("--timeout", type=int, default=2400)
    ap.add_argument("--workdir", default="build/lanes/enh/sweep")
    a = ap.parse_args()
    os.makedirs(a.workdir, exist_ok=True)
    idxs = [int(x) for x in a.targets.split(",") if x] or list(range(len(TOKENS)))
    ar = [int(x) for x in a.ar_dirs.split(",") if x]
    with cf.ThreadPoolExecutor(a.jobs) as ex:
        field = {}
        for i in idxs:
            for j in range(len(DIRS)):
                for c in FIELD:
                    if c != "200r" or j in ar:
                        field[(i, c, j)] = ex.submit(run_one, a, i, c, a.workdir, [j])
        battle = {i: is_battle(field[(i, "100", 0)].result()) for i in idxs}
        bjobs = {(i, c): ex.submit(run_one, a, i, c, a.workdir) for i in idxs if battle[i] for c in BATTLE}
        bad = 0
        for i in idxs:
            if battle[i]:
                runs = {c: bjobs[(i, c)].result() for c in BATTLE}
            else:
                runs = {(c, j): f.result() for (ii, c, j), f in field.items() if ii == i}
            ok, notes = judge(a, i, runs, battle[i])
            bad += not ok
            print(f"{'PASS' if ok else 'FAIL'} warp {i:2d} {TOKENS[i]:08X} {'battle' if battle[i] else 'field '}  "
                  + " | ".join(notes), flush=True)
    print(f"move_speed_sweep: {len(idxs) - bad}/{len(idxs)} rooms pass")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
