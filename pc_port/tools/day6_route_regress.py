#!/usr/bin/env python3
"""Day 6 -> ending route measurement (harness-only, ASSISTED-DIAGNOSTIC: coverage
evidence of which room code runs natively on the paths a run exercises, never
play evidence).

  default     warp m0425i at story 0x2D6 with Disc 2 in the drive (--warp-disc 2)
              and a 0x2D6 persist block (--persist; e.g. the block run f4 of
              docs/evidence/laterdays-native-20261021 dumped on entering m0275i).
              m0425i's own script sets door 0x1A9 and transfers to m0275i; the
              route pilot then replays day6_walk_waypoints.txt (or --waypoints
              PATH) through the m0275i/m0276i walks, the Day-6 battles (pilot
              battle aid), the pursuer chase m0275i .. m0287i (run, '+'), and
              m0288i .. m0050i, whose script plays movie 0x2E, writes story 0x2F4
              and transfers to the credits pseudo-room m8888i (token AA108448).
  --room/--story start a later leg (a dev warp into that room at that story);
              only the later checkpoints are kept.
  --until     what ends the run as a pass:
                credits      (default) the transfer to AA108448; the run stops
                             there (the credits loop and m0431i are another lane);
                ending       AA108448, then m0431i (A80830C8) entered after it,
                             then the clear-data save ("[MEMCARD] ... saved") or
                             the title token A9400048; the post-credits story 0
                             inside m0431i is not a reset;
                story>=HEX   a story step (a leg run, e.g. story>=2E2 for the chase);
                token=HEX    a room token (a leg run).
  --override KEY=WAYPOINTS replaces the waypoint entry with that key (TOKEN@STORY
              [&MASK][#VISIT]) for this run only; an unmatched key is appended.

Outcomes (one line "DAY6 RESULT=<outcome> ..." and <log-dir>/day6-<name>.json):
  ENDING-REACHED  --until credits/ending reached;
  TARGET-REACHED  --until story>=/token= reached;
  GAME_OVER       m0438i (A8083448) entered (the pursuer catch or a lost fight);
  RESET           the title token, or story < 0x80, outside the post-ending path;
  STALL           no room/story change for --stall frames (not counted inside the
                  credits), the frame limit, the wall-clock limit, or the process
                  ending before the target;
  NATIVE-STOP     a CPU_BOUNDARY / FATAL / [STOP] line, "[VM] unported script
                  opcode", a credits boundary name, or a stop_reason other than
                  frame-limit / host-quit.
Non-native markers that do not end the run ([DAY1_SHIM], STUB: other than
STUB:HOST_ADAPTED, Bootstrap_ReturnVoid, "not ported", "unresolved") are counted
per room; any of them makes the exit code non-zero. laterdays_room_table.py gives
the full per-room table from the log. Lines before the dev warp ("[WARP] env
transfer") are the boot / New Game prefix: their room changes are ignored, their
markers are counted.

Aids (all existing, disclosed): the dev warp (PE_DEV_MENU, PE_WARP_ROOM/STORY/AT),
the persist warp (PE_WARP_PERSIST_FILE), PE_WARP_DISC, --route-pad (HOST_ADAPTED
skip-movie / skip-opening-menu), the route pilot and its battle aid, PE_ROUTE_MAXHP
300, PE_ONE_HIT_KILLS=1, cheat F3 (infinite ammo, PE_CHEAT_KEYS=1010:F3).

The child gets PE_MEMCARD1=<log-dir>/memcard1.mcd unless PE_MEMCARD1 is set, and
PE_MEMCARD2 is always removed from its environment. The wall clock is enforced
with a watchdog (--wall), since PE_ROUTE_PLAY_MOVIES holds the route clock.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import signal
import subprocess
import sys
import threading
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from day2_route_regress import RE_BAD, RE_ROUTE, pad_script  # noqa: E402

HERE = os.path.dirname(os.path.abspath(__file__))

TITLE_TOKEN = 0xA9400048       # m5000i: the title (Game Over and reset land here too)
GAMEOVER_TOKEN = 0xA8083448    # m0438i: Game Over room (pursuer catch, death)
CREDITS_TOKEN = 0xAA108448     # m8888i: main's credits arm (not a room)
M0431I_TOKEN = 0xA80830C8      # m0431i: post-credits room (clear bonus, clear-data save)
WARP_FRAME = 1000              # PE_WARP_AT / --hand-over-at

# Day-6 story steps in game order (m0426i 0x2D8 .. m0050i 0x2F4); the per-room
# writes are in docs/evidence/day6-ending-native-20261023/REPORT.md.
DAY6_STORY = [0x2D8, 0x2DA, 0x2DB, 0x2DC, 0x2DD, 0x2DE, 0x2DF, 0x2E1, 0x2E2, 0x2E4,
              0x2E6, 0x2E8, 0x2EA, 0x2EC, 0x2EE, 0x2F0, 0x2F2, 0x2F4]

CHARSET = "0123456789abcdefghiklmnoprstuvwy"
RE_WARP = re.compile(r"^\[WARP\] env transfer [0-9A-Fa-f]{8} -> ([0-9A-Fa-f]{8}) story=([0-9A-Fa-f]{8})")
RE_MEMCARD_SAVED = re.compile(r"^\[MEMCARD\] .*\bsaved\b")
RE_STOP_REASON = re.compile(r"\[HOST\] stop_reason=(\S+)")
HARD_MARKERS = (
    ("boundary", lambda l: "CPU_BOUNDARY" in l),
    ("fatal", lambda l: "FATAL" in l),
    ("stop", lambda l: l.startswith("[STOP]")),
    ("vm-unported", lambda l: "[VM] unported script opcode" in l),
    ("credits-boundary", lambda l: any(n in l for n in ("credits_xa_file_absent", "credits_cd_attach",
                                                          "credits_cd_xa_service"))),
    ("bad", lambda l: RE_BAD.search(l) is not None),     # day2's stop set, kept in sync
)
SOFT_MARKERS = (
    ("shim", lambda l: "[DAY1_SHIM]" in l),
    ("stub", lambda l: ("STUB:" in l and "STUB:HOST_ADAPTED" not in l) or "Bootstrap_ReturnVoid" in l),
    ("notported", lambda l: "not ported" in l),
    ("unresolved", lambda l: "unresolved" in l),
)
SOFT_KINDS = tuple(k for k, _ in SOFT_MARKERS)
CLEAN_STOP_REASONS = ("frame-limit", "host-quit")
PASS_OUTCOMES = ("ENDING-REACHED", "TARGET-REACHED")


def room_name(token: int) -> str:
    if token == 0:
        return "-"
    name = "".join(CHARSET[(token >> s) & 0x1F] for s in (27, 22, 17, 12, 7, 2))
    return name if (token & 0x3) == 0 else f"{name}?"


def room_token(name: str) -> int:
    """'m0425i' -> A80822C8 (inverse of room_name); a hex token passes through."""
    if len(name) == 6 and all(c in CHARSET for c in name):
        v = 0
        for i, ch in enumerate(name):
            v |= CHARSET.index(ch) << (27 - 5 * i)
        return v
    return int(name, 16)


def parse_until(spec: str) -> tuple[str, int]:
    """'credits' | 'ending' | 'story>=HEX' | 'token=HEX' -> (kind, value)."""
    if spec in ("credits", "ending"):
        return spec, 0
    if spec.startswith("story>="):
        return "story>=", int(spec[7:], 16)
    if spec.startswith("token="):
        return "token", int(spec[6:], 16)
    raise ValueError(f"bad --until {spec!r} (credits | ending | story>=HEX | token=HEX)")


def build_checkpoints(start_story: int, until: tuple[str, int]) -> list[tuple[str, int, str]]:
    """Ordered checkpoints after the warp: the Day-6 story steps above the start
    story (up to the target for story>=; none for a token target, whose story
    is not known in advance), then the target's own steps."""
    kind, val = until
    cps = []
    for s in DAY6_STORY:
        if kind == "token" or s <= start_story or (kind == "story>=" and s > val):
            continue
        cps.append(("story>=", s, f"story 0x{s:X}"))
    if kind == "story>=" and not any(c[1] == val for c in cps):
        cps.append(("story>=", val, f"story 0x{val:X}"))
    if kind in ("credits", "ending"):
        cps.append(("token", CREDITS_TOKEN, "credits token AA108448 (m8888i, main's credits arm)"))
    if kind == "ending":
        cps.append(("token", M0431I_TOKEN, "m0431i A80830C8 after the credits"))
        cps.append(("save|title", 0, "clear-data save or the title A9400048"))
    if kind == "token":
        cps.append(("token", val, f"token {val:08X} ({room_name(val)})"))
    return cps


class Classifier:
    """Line-fed state machine over the port's stderr. feed() returns True when
    the run should end (outcome decided and any linger spent)."""

    def __init__(self, start_story: int, until: str = "credits", stall: int = 8000,
                 linger: int = 0, warp_token: int | None = None, warp_frame: int = WARP_FRAME):
        self.until = parse_until(until)
        self.start_story = start_story
        self.checkpoints = build_checkpoints(start_story, self.until)
        self.idx = 0
        self.hits: list[dict] = []
        self.stall, self.linger = stall, linger
        self.warp_token, self.warp_frame = warp_token, warp_frame
        self.started = False
        self.frame = 0
        self.token = 0
        self.story = 0
        self.last_progress = 0
        self.visits: list[dict] = []
        self.drops: list[str] = []
        self.markers: dict[str, int] = {}
        self.marker_lines: list[dict] = []
        self.outcome: str | None = None
        self.reason = ""
        self.decided_at: int | None = None
        self.stop_reason: str | None = None
        self.credits_at: int | None = None
        self.m0431i_at: int | None = None
        self.save_at: int | None = None
        self.title_after_ending_at: int | None = None
        self.gameover_at: int | None = None
        self.tail_markers: dict[str, int] = {}
        self.lines = 0

    def _decide(self, outcome: str, reason: str) -> None:
        if self.outcome is None:
            self.outcome, self.reason, self.decided_at = outcome, reason, self.frame

    def _post_ending(self) -> bool:
        return self.credits_at is not None and self.m0431i_at is not None

    def _marker(self, kind: str, line: str, hard: bool) -> None:
        self.markers[kind] = self.markers.get(kind, 0) + 1
        if len(self.marker_lines) < 40:
            self.marker_lines.append({"kind": kind, "hard": hard, "frame": self.frame,
                                      "room": room_name(self.token) if self.started else "(boot prefix)",
                                      "line_no": self.lines, "text": line.strip()[:240]})
        if self.started and self.visits:
            v = self.visits[-1]["markers"]
            v[kind] = v.get(kind, 0) + 1

    def _advance(self) -> None:
        while self.idx < len(self.checkpoints):
            kind, val, desc = self.checkpoints[self.idx]
            if kind == "token":
                ok = self.token == val
            elif kind == "story>=":
                ok = self.story >= val
            else:   # "save|title"
                ok = self.save_at is not None or self.title_after_ending_at is not None
            if not ok:
                break
            self.hits.append({"checkpoint": desc, "frame": self.frame, "token": f"{self.token:08X}",
                              "story": f"{self.story:X}"})
            self.idx += 1
        if self.idx == len(self.checkpoints) and self.outcome is None:
            if self.until[0] == "ending":
                end = " (title reached)" if self.title_after_ending_at is not None else " (clear-data save written)"
                self._decide("ENDING-REACHED", self.checkpoints[-1][2] + end)
            elif self.until[0] == "credits":
                self._decide("ENDING-REACHED", self.checkpoints[-1][2])
            else:
                self._decide("TARGET-REACHED", self.checkpoints[-1][2])

    def _change(self, frame: int, token: int, story: int) -> None:
        prev_story, prev_token = self.story, self.token
        self.last_progress = frame
        if token != prev_token:
            self.visits.append({"room": room_name(token), "token": f"{token:08X}", "frame_in": frame,
                                "story_in": f"{story:X}", "story_steps": [], "markers": {}})
        elif self.visits:
            self.visits[-1]["story_steps"].append(f"{story:X}")
        self.token, self.story = token, story
        post_ending = token == M0431I_TOKEN and self._post_ending()
        if token == CREDITS_TOKEN and self.credits_at is None:
            self.credits_at = frame
        if token == M0431I_TOKEN and self.credits_at is not None and self.m0431i_at is None:
            self.m0431i_at = frame
            post_ending = True
        if self.outcome is None and prev_token and story < prev_story and not post_ending:
            self.drops.append(f"0x{prev_story:X}->0x{story:X}@{frame}")
        if token == GAMEOVER_TOKEN:
            if self.gameover_at is None:
                self.gameover_at = frame
            self._decide("GAME_OVER", f"m0438i (A8083448) entered from {room_name(prev_token)} "
                                      f"story 0x{prev_story:X}")
        elif token == TITLE_TOKEN:
            if self._post_ending():
                if self.title_after_ending_at is None:
                    self.title_after_ending_at = frame
            else:
                self._decide("RESET", f"title token A9400048 from {room_name(prev_token)} story 0x{prev_story:X}")
        elif story < 0x80 and not post_ending:
            self._decide("RESET", f"story 0x{prev_story:X} -> 0x{story:X} in {room_name(token)}")
        if self.outcome is None or self.outcome in PASS_OUTCOMES:
            self._advance()

    def feed(self, line: str) -> bool:
        self.lines += 1
        for kind, pred in HARD_MARKERS:
            if pred(line):
                self._marker(kind, line, True)
                self._decide("NATIVE-STOP", f"{kind}: {line.strip()[:200]}")
                return True
        for kind, pred in SOFT_MARKERS:
            if pred(line):
                self._marker(kind, line, False)
        sr = RE_STOP_REASON.search(line)
        if sr:
            self.stop_reason = sr.group(1)
            if self.stop_reason not in CLEAN_STOP_REASONS:
                self._marker("stop_reason", line, True)
                self._decide("NATIVE-STOP", f"stop_reason={self.stop_reason}")
                return True
        if RE_WARP.search(line):
            self.started = True
        if RE_MEMCARD_SAVED.search(line) and self.m0431i_at is not None and self.save_at is None:
            self.save_at = self.frame
            if self.outcome is None:
                self._advance()
        m = RE_ROUTE.search(line)
        if not m:
            return self._done()
        frame, token, story = int(m.group(1)), int(m.group(2), 16), int(m.group(3), 16)
        if not self.started:
            if frame >= self.warp_frame and self.warp_token is not None and token == self.warp_token \
                    and story < 0x1000:
                self.started = True                # no [WARP] line seen: the warp room itself
            else:
                return False                       # boot / New Game prefix before the warp
        self.frame = max(self.frame, frame)
        if not self.last_progress:
            self.last_progress = frame
        if token != self.token or story != self.story:
            self._change(frame, token, story)
        elif (self.outcome is None and self.stall and self.token != CREDITS_TOKEN
              and frame - self.last_progress >= self.stall):
            self._decide("STALL", f"no room/story change for {frame - self.last_progress} frames in "
                                  f"{room_name(self.token)} story 0x{self.story:X}")
        return self._done()

    def tail(self, line: str) -> None:
        """A line the port printed after the outcome was decided (shutdown output):
        its stop_reason and markers are recorded; the outcome never changes."""
        self.lines += 1
        sr = RE_STOP_REASON.search(line)
        if sr:
            self.stop_reason = sr.group(1)
        for kind, pred in HARD_MARKERS + SOFT_MARKERS:
            if pred(line):
                self.tail_markers[kind] = self.tail_markers.get(kind, 0) + 1
                break
        if sr and sr.group(1) not in CLEAN_STOP_REASONS:
            self.tail_markers["stop_reason"] = self.tail_markers.get("stop_reason", 0) + 1

    def _done(self) -> bool:
        if self.outcome is None or self.decided_at is None:
            return False
        if self.outcome not in PASS_OUTCOMES:
            return True
        return self.frame - self.decided_at >= self.linger

    def finish(self, why: str = "") -> dict:
        """Close the run (process ended or was ended) and return the summary."""
        if self.outcome is None:
            nxt = self.checkpoints[self.idx][2] if self.idx < len(self.checkpoints) else "end"
            self._decide("STALL", f"{why or 'process ended'} before '{nxt}' (last {room_name(self.token)} "
                                  f"story 0x{self.story:X}, stop_reason={self.stop_reason or '-'})")
        kind, val = self.until
        return {
            "outcome": self.outcome, "reason": self.reason,
            "until": kind + (f"{val:X}" if kind in ("story>=", "token") else ""),
            "start_story": f"{self.start_story:X}", "frame": self.frame, "decided_at": self.decided_at,
            "token": f"{self.token:08X}", "room": room_name(self.token), "story": f"{self.story:X}",
            "checkpoints": [c[2] for c in self.checkpoints], "hits": self.hits, "visits": self.visits,
            "drops": self.drops, "markers": self.markers, "tail_markers": self.tail_markers,
            "soft_markers": sum(n for k, n in self.markers.items() if k in SOFT_KINDS),
            "marker_lines": self.marker_lines, "stop_reason": self.stop_reason,
            "credits_at": self.credits_at, "m0431i_at": self.m0431i_at, "save_at": self.save_at,
            "title_after_ending_at": self.title_after_ending_at, "gameover_at": self.gameover_at,
        }


def passed(summary: dict) -> bool:
    clean_process = ("rc" not in summary or
                     (summary["rc"] == 0 and not summary.get("wall_expired")
                      and not summary.get("runner_signalled")
                      and summary.get("stop_reason") in CLEAN_STOP_REASONS))
    return (summary["outcome"] in PASS_OUTCOMES and not summary["markers"]
            and not summary["tail_markers"] and clean_process)


def load_waypoints(path: str) -> list[str]:
    out = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if line and not line.startswith("#"):
                out.append(line.split("   (")[0].strip())
    return out


def apply_overrides(wps: list[str], overrides: list[str]) -> list[str]:
    wps = list(wps)
    for ov in overrides:
        key = ov.split("=", 1)[0]
        hit = [i for i, e in enumerate(wps) if e.split("=", 1)[0] == key]
        for i in hit:
            wps[i] = ov
        if not hit:
            wps.append(ov)
        print(f"day6_route_regress: override {key} ({'replaced' if hit else 'appended'})", flush=True)
    return wps


def sha256(path: str) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for b in iter(lambda: f.read(1 << 20), b""):
            h.update(b)
    return h.hexdigest()


def run(a, disc: str, disc2: str, wps: list[str]) -> int:
    os.makedirs(a.log_dir, exist_ok=True)
    name = a.name or f"from_{a.room}_{a.story}"
    log = os.path.join(a.log_dir, f"day6-{name}.log")
    shot = a.screenshot or os.path.join(a.log_dir, "end.ppm")
    env = dict(os.environ)
    env.pop("PE_MEMCARD2", None)                  # never the owner's second card
    env.setdefault("PE_MEMCARD1", os.path.join(os.path.abspath(a.log_dir), "memcard1.mcd"))
    env.update(PE_PAD_SCRIPT=pad_script(), PE_DEV_MENU="1", PE_WARP_ROOM=a.room, PE_WARP_STORY=a.story,
               PE_WARP_AT=str(WARP_FRAME), PE_ROUTE_WAYPOINTS="|".join(wps), PE_CHEAT_KEYS="1010:F3",
               PE_ROUTE_MAXHP="300", PE_ONE_HIT_KILLS="1", PE_WARP_PERSIST_FILE=os.path.abspath(a.persist))
    if a.warp_disc:
        env["PE_WARP_DISC"] = a.warp_disc
    if a.persist_dump:
        os.makedirs(a.persist_dump, exist_ok=True)
        env["PE_D3_PERSIST_DUMP"] = os.path.abspath(a.persist_dump)
    if a.floor_dump:
        os.makedirs(a.floor_dump, exist_ok=True)
        env["PE_D3_FLOOR_DUMP"] = os.path.abspath(a.floor_dump)
    for kv in a.env:
        k, _, v = kv.partition("=")
        if k == "PE_MEMCARD2":
            print("DAY6 RESULT=USAGE refusing PE_MEMCARD2", flush=True)
            return 2
        env[k] = v
    cmd = [a.bin, "--headless", "--route-pad", "--hand-over-at", str(WARP_FRAME),
           "--max-frames", str(a.frames), "--screenshot", shot, "--disc-image", disc, "--disc2-image", disc2]
    cl = Classifier(int(a.story, 16), a.until, a.stall, a.linger, warp_token=room_token(a.room))
    print(f"day6_route_regress: {len(wps)} waypoint entries, warp {a.room} story 0x{a.story}, until={a.until}, "
          f"{len(cl.checkpoints)} checkpoints, frames={a.frames} linger={a.linger} stall={a.stall} "
          f"wall={a.wall}s", flush=True)
    t0 = time.time()
    expired, finished, signalled = threading.Event(), threading.Event(), threading.Event()
    with open(log, "w", buffering=1) as lf:
        p = subprocess.Popen(cmd, env=env, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE,
                             text=True, errors="replace")

        def watchdog() -> None:
            if not finished.wait(a.wall):
                expired.set()
                p.terminate()

        def on_signal(signum, _frame) -> None:   # an outer timeout/kill: never orphan the port
            signalled.set()
            if p.poll() is None:
                p.terminate()

        signal.signal(signal.SIGTERM, on_signal)
        signal.signal(signal.SIGINT, on_signal)
        threading.Thread(target=watchdog, daemon=True).start()
        ended = False
        for line in p.stderr:
            lf.write(line)
            seen = cl.idx
            ended = cl.feed(line)
            for h in cl.hits[seen:]:
                print(f"DAY6 ok   {h['checkpoint']:52s} frame={h['frame']} token={h['token']} story={h['story']}",
                      flush=True)
            if ended:
                break
        finished.set()
        if p.poll() is None:
            p.terminate()
        killer = threading.Timer(30.0, lambda: p.poll() is None and p.kill())
        killer.start()
        # Drain the port's shutdown lines ([SCREENSHOT], [HOST] stop_reason=, final room token)
        # into the log. After a decided outcome they are recorded but never change it.
        for line in p.stderr:
            lf.write(line)
            if ended:
                cl.tail(line)
            else:
                ended = cl.feed(line) or ended
        p.wait()
        killer.cancel()
    dt = time.time() - t0
    why = (f"wall-clock limit {a.wall}s" if expired.is_set() else
           "runner signalled (outer timeout/kill)" if signalled.is_set() else f"process exit rc={p.returncode}")
    s = cl.finish(why)
    s.update({"name": name, "log": os.path.abspath(log), "log_sha256": sha256(log), "wall_s": round(dt, 1),
              "rc": p.returncode, "wall_expired": expired.is_set(),
              "runner_signalled": signalled.is_set(), "bin": os.path.abspath(a.bin),
              "bin_sha256": sha256(a.bin),
              "warp": {"room": a.room, "story": a.story, "disc": a.warp_disc,
                       "persist": os.path.abspath(a.persist), "persist_sha256": sha256(a.persist)},
              "waypoints": os.path.abspath(a.waypoints), "waypoint_entries": len(wps),
              "overrides": a.override, "extra_env": a.env, "frames_limit": a.frames,
              "screenshot": os.path.abspath(shot)})
    s["accepted"] = passed(s)
    with open(os.path.join(a.log_dir, f"day6-{name}.json"), "w") as f:
        json.dump(s, f, indent=1)
    mk = ", ".join(f"{k} {n}" for k, n in sorted(s["markers"].items())) or "0"
    print(f"DAY6 RESULT={s['outcome']} {name}: {s['reason']} | accepted={int(s['accepted'])} frame={s['frame']} last={s['room']} "
          f"story={s['story']} hits={len(s['hits'])}/{len(s['checkpoints'])} markers={mk} "
          f"drops={','.join(s['drops']) or 'none'} wall={dt:.0f}s log={log} log_sha256={s['log_sha256']}",
          flush=True)
    return 0 if s["accepted"] else 1


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--bin", default="build/pcbuild-gcc/parasite-eve-port")
    ap.add_argument("--log-dir", default="build/day6-route-regress")
    ap.add_argument("--name", default="", help="run label (log day6-<name>.log); default from_<room>_<story>")
    ap.add_argument("--persist", required=True, help="persist block for the warp (PE_WARP_PERSIST_FILE)")
    ap.add_argument("--room", default="m0425i", help="warp room (default m0425i, the Day-6 entry)")
    ap.add_argument("--story", default="2D6", help="warp story, hex (default 2D6)")
    ap.add_argument("--warp-disc", default="2", help="PE_WARP_DISC (default 2: Disc 2 in the drive)")
    ap.add_argument("--waypoints", default=os.path.join(HERE, "day6_walk_waypoints.txt"))
    ap.add_argument("--override", action="append", default=[],
                    help="KEY=WAYPOINTS: replace the entry with that key, or append it")
    ap.add_argument("--until", default="credits", help="credits | ending | story>=HEX | token=HEX")
    ap.add_argument("--frames", type=int, default=60000, help="--max-frames (presents)")
    ap.add_argument("--linger", type=int, default=0, help="frames to keep running after a pass")
    ap.add_argument("--stall", type=int, default=8000,
                    help="STALL after this many frames with no room/story change (0 = off)")
    ap.add_argument("--wall", type=int, default=1800, help="wall-clock limit in seconds")
    ap.add_argument("--persist-dump", default="", help="PE_D3_PERSIST_DUMP directory")
    ap.add_argument("--floor-dump", default="", help="PE_D3_FLOOR_DUMP directory")
    ap.add_argument("--screenshot", default="", help="--screenshot path (default <log-dir>/end.ppm)")
    ap.add_argument("--env", action="append", default=[], help="extra KEY=VALUE for the port (harness probes)")
    a = ap.parse_args()
    try:
        parse_until(a.until)
        room_token(a.room)
    except ValueError as e:
        print(f"DAY6 RESULT=USAGE {e}")
        return 2
    disc, disc2 = os.environ.get("DISC"), os.environ.get("DISC2")
    if not disc or not os.path.exists(disc) or not disc2 or not os.path.exists(disc2):
        print("DAY6 RESULT=USAGE set DISC (Disc 1 image) and DISC2 (Disc 2 image)")
        return 2
    if not os.path.exists(a.persist):
        print(f"DAY6 RESULT=USAGE no persist block {a.persist}")
        return 2
    wps = apply_overrides(load_waypoints(a.waypoints), a.override)
    return run(a, disc, disc2, wps)


if __name__ == "__main__":
    sys.exit(main())
