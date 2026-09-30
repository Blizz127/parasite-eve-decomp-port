#!/usr/bin/env python3
"""F8 warp-menu liveness check (cheat console, pe_plat/cheats.h).

For each warp target: boot with the Day-1 autopilot until the first field room,
hand the pad over (idle), open the warp menu via PE_CHEAT_KEYS, pick target i,
then run a fixed number of presents and judge the destination room as live:

  entered  D_8009D280 == target token and D_8009D1C4 == D_8009D280 (room loop
           running in the target room, not stuck in a transfer)
  ticks    D_8009CDA4 (field-loop iterations) still advancing at the end
  vm       D_8009CE00 (field VM cursor) changed after entry
  shots    the last screenshots are not black (mean luma above a floor)

--new-game boots without the route autopilot instead: the title menu is driven
to New Game by a pad script (movies play) and the warp is taken from the
prologue (default present 3000, while FMV002 is decoding in A80830C8).

Prints one row per target and exits non-zero if any target is not live.
"""
import argparse
import os
import re
import shutil
import subprocess
import sys

DISC_DEFAULT = "rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin"
RE_TRACE = re.compile(r"\[CHEATS\] trace present=(\d+) token=([0-9A-F]{8}) cur=([0-9A-F]{8}) "
                      r"story=([0-9A-F]{8}) d1a0=([0-9A-F]{8}) ticks=(\d+) vm=([0-9A-F]{8})")
RE_WARPS = re.compile(r"WARP: (.+)")
# Title -> New Game (active-low masks): Start, Start, Down, Cross.
NEW_GAME_PAD = "1000:FFF7,1010:FFFF,1300:FFF7,1310:FFFF,1450:FFEF,1460:FFFF,1500:BFFF,1510:FFFF"


def ppm_luma(path):
    with open(path, "rb") as f:
        data = f.read()
    # P6 header: magic, width, height, maxval, then one whitespace byte.
    parts, pos = [], 0
    while len(parts) < 4:
        while data[pos:pos + 1].isspace():
            pos += 1
        end = pos
        while not data[end:end + 1].isspace():
            end += 1
        parts.append(data[pos:end])
        pos = end
    pix = data[pos + 1:]
    return sum(pix[::7]) / max(1, len(pix[::7]))


def run_one(a, idx, token, workdir):
    shots = os.path.join(workdir, f"t{idx:02d}")
    shutil.rmtree(shots, ignore_errors=True)
    os.makedirs(shots)
    k = a.warp_at
    keys = [f"{k}:F8"] + [f"{k + 2 + 2 * j}:DOWN" for j in range(idx)] + [f"{k + 4 + 2 * idx}:ENTER"]
    env = dict(os.environ, PE_CHEAT_KEYS=",".join(keys), PE_WARP_TRACE=str(a.trace_every))
    if a.new_game:
        env.update(PE_SHOT_DIR=shots, PE_SHOT_EVERY=str(a.shot_every))
    else:
        env.update(PE_ROUTE_SHOT_DIR=shots, PE_ROUTE_SHOT_EVERY=str(a.shot_every))
    if a.watchdog:
        env["PE_WATCHDOG_SEC"] = str(a.watchdog)
    taps = [NEW_GAME_PAD] if a.new_game else []
    if a.confirm_every:
        # Tap confirm after the warp so dialogue boxes advance; a script that
        # only waits on a text box is not a stuck room.
        t0 = k + 4 + 2 * idx + a.confirm_delay
        for t in range(t0, a.warp_at + a.run_after, a.confirm_every):
            taps += [f"{t}:{a.confirm_mask}", f"{t + 4}:FFFF"]
    if taps:
        env["PE_PAD_SCRIPT"] = ",".join(taps)
    if a.new_game:
        cmd = [a.bin, "--headless"]
    else:
        cmd = [a.bin, "--headless", "--route-pad", "--hand-over-at", str(a.hand_over)]
    cmd += ["--max-frames", str(a.warp_at + a.run_after), "--disc-image", a.disc]
    log = os.path.join(workdir, f"t{idx:02d}.log")
    with open(log, "w") as lf:
        try:
            rc = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=lf, env=env,
                                timeout=a.timeout).returncode
        except subprocess.TimeoutExpired:
            rc = "timeout"
    text = open(log, errors="replace").read()
    tr = [m.groups() for m in RE_TRACE.finditer(text)]
    tr = [(int(p), int(t, 16), int(c, 16), int(s, 16), int(d, 16), int(ti), int(v, 16))
          for p, t, c, s, d, ti, v in tr]
    after = [t for t in tr if t[0] > a.warp_at + 4 + 2 * idx]
    entered = [t for t in after if t[1] == token and t[2] == token]
    tail = after[-4:]
    ticks_adv = len(tail) >= 2 and tail[-1][5] > tail[0][5]
    vm_changed = len({t[6] for t in entered}) > 1
    vm_tail = len({t[6] for t in tail}) > 1
    pics = sorted(f for f in os.listdir(shots) if f.endswith(".ppm"))
    luma = [ppm_luma(os.path.join(shots, f)) for f in pics[-3:]]
    lit = bool(luma) and min(luma) > a.luma_floor
    warped = "WARP:" in text
    last = tail[-1] if tail else None
    live = rc == 0 and warped and bool(entered) and ticks_adv and lit and (tail and tail[-1][1] == token)
    fatal = next((l for l in text.splitlines() if "FATAL" in l), "")
    return dict(idx=idx, token=token, rc=rc, fatal=fatal, warped=warped, entered=bool(entered),
                ticks=ticks_adv, vm=vm_changed, vm_tail=vm_tail, luma=luma, live=bool(live), last=last, log=log)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--bin", default="build/pcbuild-port/parasite-eve-port")
    ap.add_argument("--disc", default=os.environ.get("DISC", DISC_DEFAULT))
    ap.add_argument("--targets", default="", help="comma list of menu indices (default all)")
    ap.add_argument("--tokens", required=True, help="comma list of target tokens, menu order")
    ap.add_argument("--hand-over", type=int, default=1100)
    ap.add_argument("--warp-at", type=int, default=1160)
    ap.add_argument("--run-after", type=int, default=1500)
    ap.add_argument("--trace-every", type=int, default=30)
    ap.add_argument("--shot-every", type=int, default=250)
    ap.add_argument("--luma-floor", type=float, default=4.0)
    ap.add_argument("--watchdog", type=int, default=0)
    ap.add_argument("--confirm-every", type=int, default=0, help="tap confirm every N presents after the warp")
    ap.add_argument("--confirm-delay", type=int, default=120)
    ap.add_argument("--confirm-mask", default="DFFF")
    ap.add_argument("--new-game", action="store_true", help="title -> New Game with movies, no route autopilot")
    ap.add_argument("--timeout", type=int, default=900)
    ap.add_argument("--workdir", default="build/lanes/warp/check")
    a = ap.parse_args()
    tokens = [int(t, 16) for t in a.tokens.split(",")]
    idxs = [int(i) for i in a.targets.split(",")] if a.targets else list(range(len(tokens)))
    os.makedirs(a.workdir, exist_ok=True)
    bad = 0
    for i in idxs:
        r = run_one(a, i, tokens[i], a.workdir)
        bad += not r["live"]
        last = r["last"]
        lst = (f"token={last[1]:08X} cur={last[2]:08X} story={last[3]:X} d1a0={last[4]:X} ticks={last[5]}"
               if last else "no trace")
        print(f"WARP {'LIVE' if r['live'] else 'DEAD'} [{i:2d}] {r['token']:08X} warped={int(r['warped'])} "
              f"entered={int(r['entered'])} ticks_adv={int(r['ticks'])} vm={int(r['vm'])} vm_tail={int(r['vm_tail'])} "
              f"luma={','.join(f'{x:.1f}' for x in r['luma'])} rc={r['rc']} last: {lst}", flush=True)
        if r["fatal"]:
            print(f"    {r['fatal'][:160]}", flush=True)
    print(f"WARP SUMMARY {len(idxs) - bad}/{len(idxs)} live")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
