"""Ordinary keyboard input for the visible native port, with state readback.

Reads the JSON written by pe_live_gdb.py and sends X11 key events through
xdotool to the game window. Never writes guest memory.

Key map (host_window.c): Return/Z/X = Cross, C = Circle, V = Triangle,
S = Square, Q = L1, E = R1, 1 = L2, 3 = R2, Tab = Select, P = Start,
arrows = D-pad, F6 = speed toggle.

CLI:
  pe_live_drive.py --prefix P --title T state           print state
  pe_live_drive.py --prefix P --title T tap KEY [ms]    press/release
  pe_live_drive.py --prefix P --title T hold KEY ms     hold then release
  pe_live_drive.py --prefix P --title T shot OUT.png    save current frame
  pe_live_drive.py --prefix P vramshot OUT.png          save whole VRAM
  pe_live_drive.py --prefix P --title T walk X Z [tol]  walk Aya to X,Z
"""
import json
import os
import subprocess
import sys
import time

DISPLAY = os.environ.get("DISPLAY", ":10.0")


def sh(*args, check=True):
    return subprocess.run(list(args), check=check, capture_output=True,
                          text=True, env={**os.environ, "DISPLAY": DISPLAY})


def window_id(title):
    for _ in range(60):
        r = sh("xdotool", "search", "--name", title, check=False)
        ids = [l for l in r.stdout.split() if l.strip()]
        if ids:
            return ids[-1]
        time.sleep(0.5)
    raise SystemExit("window not found: %r" % title)


def read_state(prefix):
    for _ in range(50):
        try:
            with open(prefix + "-state.json") as f:
                return json.load(f)
        except (OSError, ValueError):
            time.sleep(0.05)
    raise SystemExit("no state at %s-state.json" % prefix)


def wait_state(prefix, pred, timeout, poll=0.1):
    end = time.time() + timeout
    last = None
    while time.time() < end:
        s = read_state(prefix)
        if s is not last and pred(s):
            return s
        last = s
        time.sleep(poll)
    return None


def key_down(win, key):
    sh("xdotool", "keydown", "--window", win, key)


def key_up(win, key):
    sh("xdotool", "keyup", "--window", win, key)


def tap(win, key, ms=90):
    key_down(win, key)
    time.sleep(ms / 1000.0)
    key_up(win, key)


def hold(win, key, ms):
    key_down(win, key)
    time.sleep(ms / 1000.0)
    key_up(win, key)


def save_png(prefix, out):
    """Convert the 320x240 RGB dump to PNG with a pure-Python PNG writer."""
    import struct
    import zlib
    with open(prefix + "-rgb.bin", "rb") as f:
        rgb = f.read()
    w, h = 320, 240
    raw = b"".join(b"\x00" + rgb[y * w * 3:(y + 1) * w * 3] for y in range(h))

    def chunk(t, d):
        c = struct.pack(">I", len(d)) + t + d
        return c + struct.pack(">I", zlib.crc32(t + d) & 0xFFFFFFFF)
    png = (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
           + chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b""))
    with open(out, "wb") as f:
        f.write(png)


def save_vram_png(prefix, out):
    """Convert the 1024x512 RGB555 VRAM dump to PNG."""
    import struct
    import zlib
    with open(prefix + "-vram.bin", "rb") as f:
        v = f.read()
    w, h = 1024, 512
    rows = []
    for y in range(h):
        row = bytearray(1 + w * 3)
        for x in range(w):
            p = v[(y * w + x) * 2] | (v[(y * w + x) * 2 + 1] << 8)
            row[1 + x * 3] = (p & 31) << 3
            row[2 + x * 3] = ((p >> 5) & 31) << 3
            row[3 + x * 3] = ((p >> 10) & 31) << 3
        rows.append(bytes(row))
    raw = b"".join(rows)

    def chunk(t, d):
        c = struct.pack(">I", len(d)) + t + d
        return c + struct.pack(">I", zlib.crc32(t + d) & 0xFFFFFFFF)
    png = (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
           + chunk(b"IDAT", zlib.compress(raw, 6)) + chunk(b"IEND", b""))
    with open(out, "wb") as f:
        f.write(png)


def aya_pos(s):
    a = s.get("aya")
    return (a["x"], a["z"]) if a else None


def walk_to(prefix, win, tx, tz, tol=50.0, timeout=60.0, step_ms=120,
            stop_when=None, stall_timeout=None):
    """Move Aya with D-pad taps toward (tx, tz) using position feedback.
    Screen-relative direction mapping is unknown per camera, so probe
    each axis: try a key, measure the delta, keep the key that helps."""
    end = time.time() + timeout
    keys = ["Up", "Down", "Left", "Right"]
    gain = {}
    closest = float("inf")
    last_progress = time.time()
    while time.time() < end:
        s = read_state(prefix)
        if stop_when is not None and stop_when(s):
            return False
        p = aya_pos(s)
        if p is None:
            time.sleep(0.2)
            continue
        dx, dz = tx - p[0], tz - p[1]
        if abs(dx) <= tol and abs(dz) <= tol:
            return True
        distance = dx * dx + dz * dz
        if distance < closest - 1.0:
            closest = distance
            last_progress = time.time()
        elif stall_timeout is not None and time.time() - last_progress > stall_timeout:
            return False
        best = None
        for k in keys:
            g = gain.get(k)
            if g is None:
                continue
            score = g[0] * dx + g[1] * dz
            if best is None or score > best[0]:
                best = (score, k)
        if best is None or best[0] <= 0:
            # (re)probe every key
            for k in keys:
                if stop_when is not None and stop_when(read_state(prefix)):
                    return False
                before = aya_pos(read_state(prefix))
                hold(win, k, step_ms)
                time.sleep(0.35)
                after = aya_pos(read_state(prefix))
                if before and after:
                    gain[k] = (after[0] - before[0], after[1] - before[1])
            continue
        before = aya_pos(read_state(prefix))
        hold(win, best[1], step_ms)
        time.sleep(0.3)
        after = aya_pos(read_state(prefix))
        if before and after:
            gain[best[1]] = (after[0] - before[0], after[1] - before[1])
    return False


def main():
    args = sys.argv[1:]
    prefix = "/tmp/pe-live"
    title = "Parasite Eve"
    while args and args[0].startswith("--"):
        if args[0] == "--prefix":
            prefix = args[1]
        elif args[0] == "--title":
            title = args[1]
        args = args[2:]
    cmd = args[0] if args else "state"
    if cmd == "state":
        s = read_state(prefix)
        s.pop("actors", None)
        print(json.dumps(s, indent=1))
        return
    if cmd == "shot":
        save_png(prefix, args[1])
        print("wrote", args[1])
        return
    if cmd == "vramshot":
        save_vram_png(prefix, args[1])
        print("wrote", args[1])
        return
    win = window_id(title)
    if cmd == "tap":
        tap(win, args[1], int(args[2]) if len(args) > 2 else 90)
    elif cmd == "hold":
        hold(win, args[1], int(args[2]))
    elif cmd == "walk":
        ok = walk_to(prefix, win, float(args[1]), float(args[2]),
                     float(args[3]) if len(args) > 3 else 50.0)
        print("arrived" if ok else "timeout", aya_pos(read_state(prefix)))
    else:
        raise SystemExit("unknown command " + cmd)


if __name__ == "__main__":
    main()
