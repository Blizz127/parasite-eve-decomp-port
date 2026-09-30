"""Read-only live inspection of the visible native port under gdb.

Usage (from the repo root, window on DISPLAY :10.0):

  PE_LIVE_PREFIX=/path/prefix PE_LIVE_EVERY=15 \
  gdb -q -batch -x pc_port/tools/pe_live_gdb.py --args \
      pc_port/build/parasite-eve-port --windowed --skip-movie \
      --disc-image "rom/image/.../Parasite Eve (USA) (Disc 1).bin" \
      --scale 2 --window-title "Parasite Eve - Live"

Every PE_LIVE_EVERY presented frames the script writes, atomically:
  <prefix>-state.json   frame counter, key battle/field globals, actor list
  <prefix>-ram.bin      the whole 2 MiB guest RAM
  <prefix>-rgb.bin      the presented 320x240 RGB framebuffer
  <prefix>-vram.bin     1024x512 RGB555 VRAM (PE_LIVE_VRAM=0 disables)
It never writes guest memory. The breakpoint handler returns False so the
game never pauses for the debugger. On process exit the final stop reason
is appended to <prefix>-exit.txt.
"""
import json
import os
import struct
import time

import gdb

PREFIX = os.environ.get("PE_LIVE_PREFIX", "/tmp/pe-live")
EVERY = int(os.environ.get("PE_LIVE_EVERY", "15"))
RAM_BASE = 0x80000000
RAM_SIZE = 0x200000
FB_W, FB_H = 320, 240

WATCH_U32 = {
    "battle_flags_D1A0": "D_8009D1A0",   # host scalar
    "dest_token_D280": "D_8009D280",     # host scalar, guest copy is stale
}
GUEST_U32 = {
    "battle_mode_D28C": 0x8009D28C,
    "battle_pad_edge_D1F4": 0x8009D1F4,
    "actor_head_D20C": 0x8009D20C,
    "camera_target_D254": 0x8009D254,
    "field_flags_B0CD8": 0x800B0CD8,
    "story_g74_A7918": 0x800A7918,
    "m18_B6AC8": 0x800B6AC8,
}
GUEST_U8 = {
    "battle_menu_mode_D1F0": 0x8009D1F0,
    "battle_D244": 0x8009D244,
}


def _atomic_write(path, data):
    tmp = path + ".tmp"
    with open(tmp, "wb") as f:
        f.write(data)
    os.replace(tmp, path)


class PresentBP(gdb.Breakpoint):
    def __init__(self):
        super().__init__("PE_Port_InvokePresentHook", internal=True)
        self.frames = 0
        self.last = 0.0

    def stop(self):
        self.frames += 1
        if self.frames == 1 and os.environ.get("PE_LIVE_WATCH"):
            try:
                for a in os.environ["PE_LIVE_WATCH"].split(","):
                    WatchBP(self, int(a, 16))
            except Exception as exc:
                with open(PREFIX + "-error.txt", "a") as f:
                    f.write("arm watch: %r\n" % exc)
        if self.frames % EVERY:
            return False
        try:
            self.snapshot()
        except Exception as exc:  # never let inspection stop the game
            with open(PREFIX + "-error.txt", "a") as f:
                f.write("frame %d: %r\n" % (self.frames, exc))
        return False

    def snapshot(self):
        inf = gdb.selected_inferior()
        ram_ptr = int(gdb.parse_and_eval("g_pe_ram"))
        ram = bytes(inf.read_memory(ram_ptr, RAM_SIZE))
        fb_ptr = int(gdb.parse_and_eval("&'host_framebuffer.c'::fb[0]"))
        rgb = bytes(inf.read_memory(fb_ptr, FB_W * FB_H * 3))
        vram = None
        if os.environ.get("PE_LIVE_VRAM", "1") != "0":
            vram_ptr = int(gdb.parse_and_eval("&g_gpu.vram[0]"))
            vram = bytes(inf.read_memory(vram_ptr, 1024 * 512 * 2))

        def u32(a):
            o = a - RAM_BASE
            return struct.unpack_from("<I", ram, o)[0]

        def s32(a):
            o = a - RAM_BASE
            return struct.unpack_from("<i", ram, o)[0]

        def u8(a):
            return ram[a - RAM_BASE]

        def s16(a):
            return struct.unpack_from("<h", ram, a - RAM_BASE)[0]

        state = {"frame": self.frames, "time": time.time()}
        for k, sym in WATCH_U32.items():
            state[k] = int(gdb.parse_and_eval(sym))
        for k, a in GUEST_U32.items():
            state[k] = u32(a)
        for k, a in GUEST_U8.items():
            state[k] = u8(a)
        # 334AC reads packed halfwords, not words: +C current HP,
        # +1C maximum HP, +10 AT in Aya's battle record at B8A20.
        state["aya_hp"] = s16(0x800B8A2C)
        state["aya_max_hp"] = s16(0x800B8A3C)
        state["aya_at"] = s16(0x800B8A30)
        # Actor list: head D_8009D20C, next +4, update identity +0x190,
        # 16.16 world position +0x28/+0x2C/+0x30, flags +0x98.
        actors = []
        a = u32(0x8009D20C)
        n = 0
        while a and RAM_BASE <= a < RAM_BASE + RAM_SIZE - 0x240 and n < 64:
            actors.append({
                "addr": "0x%08X" % a,
                "fn": "0x%08X" % u32(a + 0x190),
                "type": u8(a + 0x0C),
                "id": u8(a + 0x0D),
                "script": "0x%08X" % u32(a + 0x9C),
                "flags": "0x%08X" % u32(a + 0x98),
                "x": s32(a + 0x28) / 65536.0,
                "y": s32(a + 0x2C) / 65536.0,
                "z": s32(a + 0x30) / 65536.0,
            })
            a = u32(a + 4)
            n += 1
        state["actors"] = actors
        aya = [x for x in actors if x["fn"] == "0x80035C84"]
        state["aya"] = aya[0] if aya else None
        # Message records at D_800BCEA8: four 56-byte slots, byte 0 != 0 in
        # use, s16 id at +0x10 (func_800375E0_port.c REC_STRIDE).
        msgs = []
        for i in range(4):
            r = 0x800BCEA8 + i * 56
            if u8(r):
                msgs.append({"rec": "0x%08X" % r, "flags": u8(r),
                             "id": struct.unpack_from("<h", ram, r + 0x10 - RAM_BASE)[0],
                             "w1": "0x%08X" % u32(r + 4)})
        state["messages"] = msgs
        _atomic_write(PREFIX + "-ram.bin", ram)
        _atomic_write(PREFIX + "-rgb.bin", rgb)
        if vram is not None:
            _atomic_write(PREFIX + "-vram.bin", vram)
        _atomic_write(PREFIX + "-state.json",
                      json.dumps(state, indent=1).encode())
        with open(PREFIX + "-timeline.jsonl", "a") as tl:
            a = state.get("aya")
            tl.write(json.dumps({"frame": self.frames, "g74": state.get("story_g74_A7918"),
                                 "m18": state.get("m18_B6AC8"), "D280": "0x%08X" % state.get("dest_token_D280", 0),
                                 "mode": state.get("battle_mode_D28C"), "D1A0": state.get("battle_flags_D1A0"),
                                 "aya": [round(a["x"]), round(a["y"]), round(a["z"])] if a else None,
                                 "actors": len(actors)}) + "\n")


class StopBP(gdb.Breakpoint):
    """Snapshot at the moment a stop is requested (RAM still intact)."""
    def __init__(self, present):
        super().__init__("PE_Port_RequestStop", internal=True)
        self.present = present

    def stop(self):
        global PREFIX
        saved = PREFIX
        PREFIX = saved + "-stop"
        try:
            with open(saved + "-stop-backtrace.txt", "a") as f:
                f.write(gdb.execute("bt 16", to_string=True))
            self.present.snapshot()
        except Exception as exc:
            with open(saved + "-error.txt", "a") as f:
                f.write("stop snapshot: %r\n" % exc)
        finally:
            PREFIX = saved
        return False


class WatchBP(gdb.Breakpoint):
    """Optional hardware watchpoints on guest words (PE_LIVE_WATCH=0x8009D2E8[,0x...]).
    Logs frame, old/new value and the top native frames to <prefix>-watch.log
    and continues; read-only."""
    def __init__(self, present, guest_addr):
        ram_ptr = int(gdb.parse_and_eval("g_pe_ram"))
        expr = "*(unsigned int *)0x%x" % (ram_ptr + (guest_addr - RAM_BASE))
        super().__init__(expr, gdb.BP_WATCHPOINT, gdb.WP_WRITE, internal=True)
        self.present = present
        self.addr = guest_addr
        self.expr = expr

    def stop(self):
        try:
            val = int(gdb.parse_and_eval(self.expr))
            frames = []
            f = gdb.newest_frame()
            for _ in range(5):
                if f is None:
                    break
                frames.append(f.name() or "?")
                f = f.older()
            with open(PREFIX + "-watch.log", "a") as out:
                out.write("frame %d %s=0x%08X via %s\n" % (
                    self.present.frames, "0x%08X" % self.addr, val, " < ".join(frames)))
        except Exception as exc:
            with open(PREFIX + "-error.txt", "a") as out:
                out.write("watch: %r\n" % exc)
        return False


class TaskTraceBP(gdb.Breakpoint):
    """PE_LIVE_TRACE_TASK=0x8009D33C: log each VM visit of that task record
    (frame, PC, flags, delay) to <prefix>-task.log. Read-only."""
    def __init__(self, present, task):
        super().__init__("func_80017018", internal=True)
        self.present = present
        self.task_addr = task

    def stop(self):
        try:
            inf = gdb.selected_inferior()
            ram_ptr = int(gdb.parse_and_eval("g_pe_ram"))
            def gu32(a):
                return struct.unpack("<I", bytes(inf.read_memory(ram_ptr + (a - RAM_BASE), 4)))[0]
            cur = gu32(0x8009D300)
            if cur == self.task_addr:
                with open(PREFIX + "-task.log", "a") as out:
                    out.write("frame %d task pc=0x%08X flags=0x%08X delay=%d actor=0x%08X D1A0=0x%X\n" % (
                        self.present.frames, gu32(self.task_addr), gu32(self.task_addr + 8),
                        gu32(self.task_addr + 0x10), gu32(0x8009D2F0),
                        int(gdb.parse_and_eval("D_8009D1A0"))))
        except Exception as exc:
            with open(PREFIX + "-error.txt", "a") as out:
                out.write("tasktrace: %r\n" % exc)
        return False


class CallLogBP(gdb.Breakpoint):
    """PE_LIVE_CALLS=fn1,fn2: log each call of these native functions with
    their argument values (needs debug info) to <prefix>-calls.log."""
    def __init__(self, present, name):
        super().__init__(name, internal=True)
        self.present = present
        self.fname = name

    def stop(self):
        try:
            fr = gdb.selected_frame()
            args = []
            blk = fr.block()
            for sym in blk:
                if sym.is_argument:
                    try:
                        args.append("%s=0x%X" % (sym.name, int(sym.value(fr)) & 0xFFFFFFFF))
                    except Exception:
                        args.append("%s=?" % sym.name)
            with open(PREFIX + "-calls.log", "a") as out:
                out.write("frame %d %s(%s)\n" % (self.present.frames, self.fname, ", ".join(args)))
        except Exception as exc:
            with open(PREFIX + "-error.txt", "a") as out:
                out.write("calllog: %r\n" % exc)
        return False


def on_exit(event):
    code = getattr(event, "exit_code", None)
    with open(PREFIX + "-exit.txt", "a") as f:
        f.write("exit code %r at %s\n" % (code, time.ctime()))


def on_signal(event):
    if not isinstance(event, gdb.SignalEvent):
        return
    global PREFIX
    saved = PREFIX
    PREFIX = saved + "-crash"
    try:
        with open(PREFIX + "-backtrace.txt", "a") as f:
            f.write(event.stop_signal + "\n" + gdb.execute("bt 24", to_string=True))
        _present.snapshot()
    except Exception as exc:
        with open(saved + "-error.txt", "a") as f:
            f.write("signal snapshot: %r\n" % exc)
    finally:
        PREFIX = saved


gdb.execute("set pagination off")
gdb.execute("set confirm off")
gdb.execute("set breakpoint pending on")
gdb.execute("set print thread-events off")
_present = PresentBP()
StopBP(_present)
for _n in [x for x in os.environ.get("PE_LIVE_CALLS", "").split(",") if x]:
    CallLogBP(_present, _n)
if os.environ.get("PE_LIVE_TRACE_TASK"):
    TaskTraceBP(_present, int(os.environ["PE_LIVE_TRACE_TASK"], 16))
gdb.events.exited.connect(on_exit)
gdb.events.stop.connect(on_signal)
gdb.execute("run")
