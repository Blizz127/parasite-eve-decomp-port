"""Ordinary-input replay of Day 1 from New Game to the under-stage scene.

Drives the visible native port through pe_live_drive (xdotool keys) using
only state read back from pe_live_gdb.py snapshots. No guest RAM writes.

  python3 pc_port/tools/pe_live_replay.py --prefix local/live/sewersN \
      --title "Parasite Eve - Sewers run N" [--until PHASE]

Phases (in order): opening, name, courtyard, lobby, auditorium, aisle,
battle, stage_exit, backstage, understage, corridor, corridor_rat, mirror_room,
mirror_exit, theater_key_room, theater_key, diary_room, diary_key, rehearsal_room.
Each phase logs the frame it finished at.
"""
import json
import os
import struct
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pe_live_drive as d  # noqa: E402

RAM_BASE = 0x80000000


def ram_u32(prefix, addr):
    with open(prefix + "-ram.bin", "rb") as f:
        f.seek(addr - RAM_BASE)
        return struct.unpack("<I", f.read(4))[0]


class Replay:
    def __init__(self, prefix, title, log):
        self.prefix = prefix
        self.win = d.window_id(title)
        self.log = log
        self.events = []

    def note(self, phase, **kw):
        s = d.read_state(self.prefix)
        rec = {"phase": phase, "frame": s["frame"], **kw}
        self.events.append(rec)
        print(json.dumps(rec), file=self.log, flush=True)

    def state(self):
        return d.read_state(self.prefix)

    def aya(self, s=None):
        return (s or self.state()).get("aya")

    def in_control(self, s):
        a = s.get("aya")
        return bool(a) and a["flags"] == "0x00000008" and not (s["field_flags_B0CD8"] & 0x2000)

    def stopped(self):
        try:
            with open(self.prefix + "-exit.txt") as f:
                return f.read()
        except OSError:
            return None

    def tap_until(self, pred, timeout, key="Return", gap=1.2, alive=False):
        end = time.time() + timeout
        while time.time() < end:
            if self.stopped():
                return None
            s = self.state()
            if alive and (s.get("aya_hp",0)<=0 or
                          s.get("battle_mode_D28C") in (-1,0xFFFFFFFF)):
                self.note("defeat_detected",hp=s.get("aya_hp"),mode=s.get("battle_mode_D28C"))
                raise AssertionError("Aya was defeated; input stopped before advancing game over")
            if pred(s):
                return s
            d.tap(self.win, key, 90)
            time.sleep(gap)
        return None

    def wait_until(self, pred, timeout, poll=0.2):
        end = time.time() + timeout
        while time.time() < end:
            if self.stopped():
                return None
            s = self.state()
            if pred(s):
                return s
            time.sleep(poll)
        return None

    def walk_until(self, key, pred, timeout, step_ms=1500):
        end = time.time() + timeout
        while time.time() < end:
            if self.stopped():
                return None
            s = self.state()
            if pred(s):
                return s
            d.hold(self.win, key, step_ms)
            time.sleep(0.4)
        return None

    # ---- phases -------------------------------------------------------
    def opening(self):
        s = self.tap_until(lambda s: ram_u32(self.prefix, 0x8009D154) != 0, 120)
        assert s, "naming screen never opened"
        self.note("opening")

    def name(self):
        time.sleep(0.8)
        d.tap(self.win, "p", 90)          # Start: focus End
        time.sleep(0.8)
        d.tap(self.win, "Return", 90)     # confirm default name "Aya"
        s = self.wait_until(lambda s: ram_u32(self.prefix, 0x8009D154) == 0, 20)
        assert s, "name menu did not close"
        self.note("name")

    def courtyard(self):
        s = self.tap_until(lambda s: self.in_control(s) and self.aya(s)["z"] < -4000, 120)
        assert s, "no control in courtyard"
        self.note("courtyard", pos=[self.aya(s)["x"], self.aya(s)["z"]])

    def lobby(self):
        # walk Up until the map changes (Aya's Z jumps from ~-4600 to ~-176)
        s = self.walk_until("Up", lambda s: self.aya(s) and -1000 < self.aya(s)["z"] < 4000
                            and self.aya(s)["y"] < 0, 90)
        assert s, "never entered lobby"
        self.note("lobby", pos=[self.aya(s)["x"], self.aya(s)["z"]])

    def auditorium(self):
        # Up through the lobby until Aya vanishes (auditorium/performance scene)
        s = self.walk_until("Up", lambda s: self.aya(s) is None, 120, step_ms=2000)
        assert s, "never reached auditorium scene"
        self.note("auditorium")

    # Seat route proven by the INV22 replays (ACTIVE_HANDOFF): clear the seat
    # corner on the right before crossing to the side aisle and the stairs.
    AISLE = [(-1790.0, 3420.0), (-1790.0, 3100.0), (-2490.0, 3100.0),
             (-2490.0, 2500.0), (-1700.0, 1200.0), (-1400.0, 50.0),
             (-1100.0,500.0),(-600.0,500.0)]
    # The Eve encounter is a walk-in region (script op 77 polygon x -820..995,
    # z -202..2328 gated by 0x11 <= g74 < 0x28); the last waypoint is inside it.

    def aisle(self):
        # Retail gives control at the top of the aisle (X-2049, Z7468).
        s = self.tap_until(lambda s: self.in_control(s), 300)
        assert s, "no control in the auditorium"
        self.note("aisle_control", pos=[self.aya(s)["x"], self.aya(s)["z"]])
        def encounter_started(state):
            return bool(state["battle_flags_D1A0"] & 2 or
                        state.get("story_g74_A7918", 0) >= 0x28)
        for (x, z) in self.AISLE:
            if encounter_started(self.state()):
                break
            ok = d.walk_to(self.prefix, self.win, x, z, tol=60.0, timeout=90,
                           stop_when=encounter_started, stall_timeout=10)
            self.note("aisle_waypoint", target=[x, z], reached=ok,
                      pos=[self.aya()["x"], self.aya()["z"]] if self.aya() else None)
        self.note("aisle")

    def battle(self):
        s = self.tap_until(lambda s: s["battle_flags_D1A0"] & 2, 300,alive=True)
        assert s, "battle never started"
        self.note("battle_start")
        s = self.tap_until(lambda s: not (s["battle_flags_D1A0"] & 2)
                           and s["battle_mode_D28C"] == 12, 400, gap=0.8,alive=True)
        assert s, "battle never reached scripted exit"
        self.note("battle_exit")
        s = self.tap_until(lambda s: self.in_control(s) and self.aya(s)
                           and abs(self.aya(s)["x"] - 345) < 5, 120,alive=True)
        assert s, "no control after battle"
        self.note("battle", pos=[self.aya(s)["x"], self.aya(s)["z"]])

    def stage_exit(self):
        # Stage right wing: walk toward x=-2000,z=0 then keep going Right.
        left_stage = lambda s: bool(self.aya(s) and self.aya(s)["y"] > -500)
        ok = d.walk_to(self.prefix, self.win, -1900.0, 0.0, tol=150.0, timeout=90,
                       stop_when=left_stage)
        s = self.walk_until("Right", lambda s: self.aya(s) and self.aya(s)["y"] > -500, 60, step_ms=3000)
        assert s, "never left the stage (walk_to=%s)" % ok
        self.note("stage_exit", pos=[self.aya(s)["x"], self.aya(s)["z"]])

    def backstage(self):
        time.sleep(1.0)
        event_started = lambda s: bool(s["field_flags_B0CD8"] & 0x2000)
        ok = d.walk_to(self.prefix, self.win, -280.0, -2450.0, tol=120.0, timeout=120,
                       stop_when=event_started)
        s = self.walk_until("Up", event_started, 30, step_ms=800)
        assert s, "hole event never triggered (walk_to=%s)" % ok
        self.note("backstage", pos=[self.aya(s)["x"], self.aya(s)["z"]])

    def understage(self):
        s = self.tap_until(lambda s: self.aya(s) and abs(self.aya(s)["y"]) < 1
                           and abs(self.aya(s)["z"] - 125) < 5, 60)
        assert s, "never reached under-stage scene"
        self.note("understage_scene")
        # keep advancing dialogue until control returns or the port stops
        s = self.tap_until(lambda s: self.in_control(s), 240)
        self.note("understage", control=bool(s), stop=self.stopped())


    def fight(self, label, timeout=400):
        """Generic random battle: Cross taps until the battle flag clears."""
        s = self.tap_until(lambda s: not (s["battle_flags_D1A0"] & 2), timeout, gap=0.8,alive=True)
        self.note(label, won=bool(s), stop=self.stopped())
        return s

    def corridor(self):
        # Under-stage → Up through the exit door into the backstage corridor
        # (Aya lands at X293 Z~3900), then Up along the corridor. Random
        # encounters (rats) are fought with Cross taps as they come.
        s = self.walk_until("Up", lambda s: self.aya(s) and self.aya(s)["z"] > 3000, 60, step_ms=2000)
        assert s, "never entered the corridor"
        self.note("corridor_enter", pos=[self.aya(s)["x"], self.aya(s)["z"]])
        end = time.time() + 300
        while time.time() < end and not self.stopped():
            s = self.state()
            if s["battle_flags_D1A0"] & 2:
                self.note("corridor_battle_start", pos=[self.aya(s)["x"], self.aya(s)["z"]] if self.aya(s) else None)
                if not self.fight("corridor_battle"):
                    return
                s = self.wait_until(lambda s: self.in_control(s), 60)
                if not s:
                    break
                continue
            if self.aya(s) and self.aya(s)["z"] < -1400:
                break
            d.hold(self.win, "Up", 1500)
            time.sleep(0.4)
        self.note("corridor", pos=[self.aya()["x"], self.aya()["z"]] if self.aya() else None,
                  stop=self.stopped())

    def corridor_rat(self):
        # The first corridor rat is scripted: its trigger sets g74=0x40,
        # and the field return scene advances it to 0x48. Crossing the
        # trigger's Z coordinate alone does not prove this encounter ended.
        s = self.tap_until(lambda s: bool(s["battle_flags_D1A0"] & 2) or
                           s.get("story_g74_A7918", 0) >= 0x48, 60, gap=0.5,alive=True)
        assert s, "corridor rat did not start: %s" % self.stopped()
        if s["battle_flags_D1A0"] & 2:
            assert self.fight("corridor_rat_defeated"), self.stopped()
        s = self.tap_until(lambda s: self.in_control(s) and
                           s.get("story_g74_A7918", 0) >= 0x48, 90, gap=0.5,alive=True)
        assert s, "corridor rat did not return field control: %s" % self.stopped()
        self.note("corridor_rat", pos=[self.aya(s)["x"], self.aya(s)["z"]],
                  story=s["story_g74_A7918"])


    def mirror_room(self):
        # Door 6's automatic polygon is X532..669, Z-2018..-1900.
        # Align in the corridor before moving into the narrow doorway.
        ok = d.walk_to(self.prefix, self.win, -50, -1944, tol=30, timeout=60)
        assert ok, "could not line up with dressing-room door"
        s = self.walk_until("Left", lambda s: bool(s["field_flags_B0CD8"] & 0x2000),
                            15, step_ms=300)
        assert s, "dressing-room door did not trigger"
        self.note("mirror_door_trigger")
        s = self.tap_until(lambda s: self.in_control(s) and len(s["actors"]) >= 7, 90, gap=0.8)
        self.note("mirror_room", control=bool(s), stop=self.stopped())
        assert s, "dressing-room transition stopped: %s" % self.stopped()

    def mirror_exit(self):
        prior_script = self.aya()["script"]
        ok = d.walk_to(self.prefix, self.win, 0, -745, tol=30, timeout=45)
        assert ok, "could not line up with dressing-room exit"
        s = self.walk_until("Down", lambda s: bool(s["field_flags_B0CD8"] & 0x2000),
                            10, step_ms=250)
        assert s, "dressing-room exit did not trigger"
        self.note("mirror_exit_trigger")
        s = self.tap_until(lambda s: self.in_control(s) and
                          s["aya"]["script"] != prior_script and
                          s["dest_token_D280"] in (0, 0xA8001148), 90, gap=0.8)
        self.note("mirror_exit", control=bool(s), stop=self.stopped())
        assert s, "mirror-room exit stopped: %s" % self.stopped()
        d.save_png(self.prefix, self.prefix + "-mirror-exit.png")

    def door5(self):
        prior_script = self.aya()["script"]
        ok = d.walk_to(self.prefix, self.win, -50, -1944, tol=30, timeout=60)
        assert ok, "could not line up with door5"
        s = self.walk_until("Right", lambda s: bool(s["field_flags_B0CD8"] & 0x2000),
                            15, step_ms=300)
        assert s, "door5 did not trigger"
        self.note("door5_trigger")
        s = self.tap_until(lambda s: self.in_control(s) and
                          s["aya"]["script"] != prior_script and
                          s["dest_token_D280"] in (0, 0xA80013C8), 90, gap=0.8)
        self.note("door5", control=bool(s), stop=self.stopped())
        assert s, "door5 transition stopped: %s" % self.stopped()

    def theater_key_room(self):
        # Original door9 (M0020I) awards item200 and sets g24 bit0x20.
        # Door5's dressing-room event does not award the required key.
        battle = lambda s: bool(s["battle_flags_D1A0"] & 2)
        # Runs22/28: the centerline can stop atZ-2275 or-2695. Stay on
        # the right throughZ-3600 before rejoining the corridor center.
        for x,z in ((-50,-1940),(130,-2350),(130,-2600),(130,-3100),(130,-3600),(-50,-5390)):
            for attempt in range(6):
                ok=d.walk_to(self.prefix,self.win,x,z,tol=30,timeout=120,
                             stop_when=battle,stall_timeout=12)
                if battle(self.state()):
                    assert self.fight("key_route_battle"), self.stopped()
                    assert self.tap_until(self.in_control,90,gap=0.5), self.stopped()
                    continue
                self.note("key_room_waypoint",target=[x,z],reached=ok,
                          pos=[self.aya()["x"],self.aya()["z"]])
                assert ok, "could not reach key-room doorway"
                break
            else: raise AssertionError("too many interrupted key-room approaches")
        prior_script=self.aya()["script"]
        s=self.walk_until("Left",lambda s: bool(s["field_flags_B0CD8"] & 0x2000),
                          15,step_ms=250)
        assert s, "theater-key door did not trigger"
        self.note("theater_key_door_trigger")
        s=self.tap_until(lambda s: self.in_control(s) and s["aya"]["script"]!=prior_script and
                         s["dest_token_D280"] in (0,0xA8002048),90,gap=0.8)
        self.note("theater_key_room",control=bool(s),stop=self.stopped())
        assert s, "theater-key room stopped: %s" % self.stopped()

    def theater_key(self):
        # Run21: approach around the chair's right edge. Contact starts the
        # examination before the final waypoint; Cross then searches the body.
        story = lambda: ram_u32(self.prefix, 0x800A7850)
        busy = lambda s: bool(s["field_flags_B0CD8"] & 0x2000)
        if story() & 0x20:
            self.note("theater_key", already_collected=True)
            return
        for x,z in ((585,180),(460,160),(395,185)):
            ok=d.walk_to(self.prefix,self.win,x,z,tol=12,timeout=20,
                         stop_when=busy,stall_timeout=5)
            if busy(self.state()):
                break
            assert ok, "could not approach the body"
        s=self.tap_until(lambda s: bool(story()&0x200) and not busy(s),40,gap=0.8)
        assert s, "body examination did not finish: %s" % self.stopped()
        self.note("body_examined",g24=hex(story()))
        s=self.tap_until(lambda s: bool(s["field_flags_B0CD8"]&0x1000),40,gap=0.8)
        assert s, "theater-key popup did not open: %s" % self.stopped()
        d.save_png(self.prefix,self.prefix+"-key-popup.png")
        self.note("theater_key_popup",g24=hex(story()))
        s=self.tap_until(lambda s: bool(story()&0x20) and
                         not(s["field_flags_B0CD8"]&0x3000),40,gap=0.8)
        assert s, "theater-key pickup did not return control: %s" % self.stopped()
        self.note("theater_key",g24=hex(story()))

    def diary_room(self):
        busy=lambda s:bool(s["field_flags_B0CD8"]&0x2000)
        for x,z in ((585,180),(730,0)):
            ok=d.walk_to(self.prefix,self.win,x,z,tol=15,timeout=30,
                         stop_when=busy,stall_timeout=6)
            if busy(self.state()):break
            assert ok,"could not leave theater-key room"
        s=self.tap_until(lambda s:self.in_control(s) and
                         s["dest_token_D280"]==0xA8001148,60,gap=0.8)
        assert s,self.stopped()
        self.note("theater_key_exit")
        # Leave the doorway before walk_to probes all four directions.
        d.hold(self.win,"Right",600)
        self.corridor_waypoints(((-50,-5390),(-50,-3572)))
        assert self.walk_until("Left",busy,20,step_ms=250),self.stopped()
        s=self.tap_until(lambda s:self.in_control(s) and
                         s["dest_token_D280"]==0xA8001448,90,gap=0.8)
        self.note("diary_room",control=bool(s),stop=self.stopped())
        assert s,self.stopped()

    def corridor_waypoints(self,points):
        battle=lambda s:bool(s["battle_flags_D1A0"]&2)
        for x,z in points:
            for attempt in range(5):
                ok=d.walk_to(self.prefix,self.win,x,z,tol=25,timeout=90,
                             stop_when=battle,stall_timeout=10)
                if battle(self.state()):
                    assert self.fight("corridor_route_battle"),self.stopped()
                    assert self.tap_until(self.in_control,60,gap=0.8),self.stopped()
                    continue
                assert ok,"could not reach corridor waypoint"
                break
            else:raise AssertionError("too many interrupted corridor approaches")

    def diary_key(self):
        assert self.tap_until(self.in_control,40,gap=0.8),self.stopped()
        if self.state()["dest_token_D280"] != 0xA8001448:
            busy=lambda s:bool(s["field_flags_B0CD8"]&0x2000)
            self.corridor_waypoints(((-50,-3572),))
            assert self.walk_until("Left",busy,20,step_ms=250),self.stopped()
            s=self.tap_until(lambda s:self.in_control(s) and
                             s["dest_token_D280"]==0xA8001448,90,gap=0.8)
            assert s,"could not re-enter diary room: %s" % self.stopped()
        for x,z in ((300,-300),(0,0),(-181,223)):
            a=self.aya()
            if a and abs(a["x"]-x)<=20 and abs(a["z"]-z)<=20:
                continue
            assert d.walk_to(self.prefix,self.win,x,z,tol=20,timeout=45,
                             stall_timeout=12),"could not approach diary"
        d.hold(self.win,"Up",120)  # face the diary at original heading0x800
        s=self.tap_until(lambda s:bool(s["field_flags_B0CD8"]&0x1000),150,gap=1)
        assert s,"Rehearse Key popup did not open: %s" % self.stopped()
        self.note("rehearse_key_popup",g24=hex(ram_u32(self.prefix,0x800A7850)))
        d.save_png(self.prefix,self.prefix+"-rehearse-key.png")
        s=self.tap_until(lambda s:bool(ram_u32(self.prefix,0x800A7850)&0x80000) and
                         not(s["field_flags_B0CD8"]&0x3000),40,gap=0.8)
        assert s,"Rehearse Key pickup did not finish: %s" % self.stopped()
        self.note("diary_key")

    def rehearsal_room(self):
        busy=lambda s:bool(s["field_flags_B0CD8"]&0x2000)
        # Run24/31: the table blocks a south line from (300,200). Go around
        # its right side, then to the east door.
        for x,z in ((500,180),(550,-50),(750,-400),(1000,-400),(1200,-400),(1400,-200)):
            a=self.aya()
            if a and abs(a["x"]-x)<=25 and abs(a["z"]-z)<=25:
                continue
            ok=d.walk_to(self.prefix,self.win,x,z,tol=25,timeout=45,
                         stop_when=busy,stall_timeout=12)
            if busy(self.state()):break
            if not ok:
                d.hold(self.win,"Right",1500)
                if busy(self.state()):break
                continue
        else:
            d.hold(self.win,"Right",2000)
        s=self.tap_until(lambda s:self.in_control(s) and
                         s["dest_token_D280"]==0xA8001148,60,gap=0.8)
        assert s,self.stopped()
        self.note("diary_room_exit")
        d.hold(self.win,"Right",600)
        self.corridor_waypoints(((-50,-3572),(-4,-5900)))
        assert self.walk_until("Up",busy,20,step_ms=200),self.stopped()
        self.note("rehearsal_door_unlocked")
        s=self.tap_until(lambda s:self.in_control(s) and
                         s["dest_token_D280"]==0xA80614C8,120,gap=0.8)
        assert s,self.stopped()
        self.note("rehearsal_room")
        d.save_png(self.prefix,self.prefix+"-rehearsal-room.png")

PHASES = ["opening", "name", "courtyard", "lobby", "auditorium", "aisle", "battle",
          "stage_exit", "backstage", "understage", "corridor", "corridor_rat", "mirror_room",
          "mirror_exit", "theater_key_room", "theater_key", "diary_room", "diary_key",
          "rehearsal_room"]


def main():
    args = sys.argv[1:]
    prefix, title, until, start = "local/live/sewers2", "Parasite Eve - Sewers run 2", None, None
    while args:
        if args[0] == "--prefix":
            prefix = args[1]
        elif args[0] == "--title":
            title = args[1]
        elif args[0] == "--until":
            until = args[1]
        elif args[0] == "--from":
            start = args[1]
        args = args[2:]
    with open(prefix + "-replay.log", "a") as log:
        r = Replay(prefix, title, log)
        for ph in PHASES[PHASES.index(start) if start else 0:]:
            getattr(r, ph)()
            if ph == until:
                break
        d.save_png(prefix, prefix + "-final.png")


if __name__ == "__main__":
    main()
