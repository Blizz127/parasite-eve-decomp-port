#!/usr/bin/env python3
"""Frame coverage for headless port runs (player-experience record).

The port writes <dir>/p<present>.ppm every N presents when PE_SHOT_DIR and
PE_SHOT_EVERY are set (and PE_ROUTE_SHOTS=1 for --route-pad runs). This tool
hashes those frames and reports:
  - frames captured, first/last present, distinct images;
  - the longest run of consecutive identical captures, in presents (a frozen
    or stuck display shows up as a long run);
  - all-black captures;
  - with --baseline MANIFEST: per-present comparison against an earlier run.

It writes a manifest (one line per capture: present, sha256) with --write.
Usage:
  pe_frame_coverage.py SHOT_DIR [--write MANIFEST] [--baseline MANIFEST]
                       [--freeze-presents N]
Exit status: 0, or 1 when --baseline differs or a run of identical frames is
at least --freeze-presents long (when given).
"""
import argparse
import hashlib
import os
import re
import sys

RE_SHOT = re.compile(r"^p(\d+)\.ppm$")


def ppm_pixels(path):
    """The pixel bytes of a binary PPM (header stripped), so a header change
    alone does not count as a different image."""
    data = open(path, "rb").read()
    if not data.startswith(b"P6"):
        return data
    fields, pos = [], 2
    while len(fields) < 3:
        while data[pos:pos + 1].isspace():
            pos += 1
        if data[pos:pos + 1] == b"#":
            pos = data.index(b"\n", pos) + 1
            continue
        start = pos
        while not data[pos:pos + 1].isspace():
            pos += 1
        fields.append(int(data[start:pos]))
    return data[pos + 1:]


def scan(shot_dir):
    shots = []
    for name in os.listdir(shot_dir):
        m = RE_SHOT.match(name)
        if m:
            shots.append((int(m.group(1)), os.path.join(shot_dir, name)))
    shots.sort()
    out = []
    for present, path in shots:
        px = ppm_pixels(path)
        out.append((present, hashlib.sha256(px).hexdigest(), not any(px)))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("shot_dir")
    ap.add_argument("--write")
    ap.add_argument("--baseline")
    ap.add_argument("--freeze-presents", type=int)
    a = ap.parse_args()

    frames = scan(a.shot_dir)
    if not frames:
        print("FRAMES none captured in %s" % a.shot_dir)
        return 1
    distinct = len({h for _, h, _ in frames})
    black = sum(1 for _, _, b in frames if b)
    longest, run_start, best = 0, 0, (0, 0)
    for i in range(1, len(frames) + 1):
        if i == len(frames) or frames[i][1] != frames[run_start][1]:
            span = frames[i - 1][0] - frames[run_start][0]
            if span > longest:
                longest, best = span, (frames[run_start][0], frames[i - 1][0])
            run_start = i
    print("FRAMES captured=%d presents=%d..%d distinct=%d black=%d"
          % (len(frames), frames[0][0], frames[-1][0], distinct, black))
    print("FRAMES longest_identical_run=%d presents (p%06d..p%06d)" % (longest, best[0], best[1]))
    status = 0
    if a.freeze_presents is not None and longest >= a.freeze_presents:
        print("FRAMES FREEZE_SUSPECT run >= %d presents" % a.freeze_presents)
        status = 1
    if a.write:
        with open(a.write, "w") as f:
            for present, h, _ in frames:
                f.write("%d %s\n" % (present, h))
    if a.baseline:
        base = {}
        for line in open(a.baseline):
            p, h = line.split()
            base[int(p)] = h
        got = {p: h for p, h, _ in frames}
        common = sorted(set(base) & set(got))
        diff = [p for p in common if base[p] != got[p]]
        only_base = sorted(set(base) - set(got))
        only_got = sorted(set(got) - set(base))
        print("BASELINE common=%d differ=%d missing=%d extra=%d"
              % (len(common), len(diff), len(only_base), len(only_got)))
        if diff:
            print("BASELINE first_differing_present=%d" % diff[0])
        if diff or only_base or only_got:
            status = 1
    return status


if __name__ == "__main__":
    sys.exit(main())
