#!/usr/bin/env python3
"""Offline test of the day6_route_regress.py classifier (no game run): synthetic
port stderr lines ([ROUTE], [WARP], [MEMCARD], [HOST], marker lines) are fed to
Classifier and the outcome is checked.

usage: python3 -I -B pc_port/tools/test_day6_route_regress.py [-v]
"""
import os
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import day6_route_regress as d6  # noqa: E402

M0425I, M0275I, M0276I, M0426I = 0xA80822C8, 0xA80472C8, 0xA8047348, 0xA8082348
M0430I, M0287I, M0288I, M0050I = 0xA8083048, 0xA80483C8, 0xA8048448, 0xA8005048

BOOT = [   # New Game prefix before the warp: title, m0431i at story 0, the prologue
    "[ROUTE] change frame=1 token=00000000 story=00000000",
    "[ROUTE] change frame=2 token=A9400048 story=00000000",
    "[STUB:HOST_ADAPTED] func_801909B4_skip_movie_new_game (first invocation)",
    "[ROUTE] change frame=6 token=A80830C8 story=00000000",
    "[ROUTE] change frame=13 token=A8001048 story=00000001",
    "[ROUTE] change frame=881 token=A8000148 story=00000008",
    "[ROUTE] change frame=963 token=A8000148 story=00000009",
    "[WARP] env transfer A8000148 -> A80822C8 story=000002D6 frame=1000",
]


def ch(frame, token, story):
    return f"[ROUTE] change frame={frame} token={token:08X} story={story:08X}"


def tick(frame, token, story):
    return f"[ROUTE] frame={frame} token={token:08X} story={story:08X} victories=0"


def feed(cl, lines):
    """Feed lines; return the index of the line that ended the run, or None."""
    for i, line in enumerate(lines):
        if cl.feed(line):
            return i
    return None


def full_route():
    """m0425i 0x2D6 .. m0050i 0x2F4 -> AA108448, with the m0373i/m0049i ping-pong."""
    rooms = [(1000, M0425I, 0x2D6), (1592, M0275I, 0x2D6), (2300, M0276I, 0x2D6), (2900, M0426I, 0x2D6),
             (3400, M0426I, 0x2D8), (3600, 0xA80823C8, 0x2D8), (4800, 0xA80823C8, 0x2DA),
             (5000, 0xA8082448, 0x2DA), (6000, 0xA8082448, 0x2DB), (6010, 0xA8065048, 0x2DB),
             (7000, 0xA8065048, 0x2DC), (7010, 0xA80644C8, 0x2DC), (7300, 0xA80824C8, 0x2DC),
             (8000, 0xA80824C8, 0x2DD), (8010, M0430I, 0x2DD), (9000, M0430I, 0x2DE),
             (9100, 0xA8047148, 0x2DE), (9500, 0xA8047148, 0x2DF), (9600, M0430I, 0x2DF),
             (10200, M0430I, 0x2E1), (10300, M0426I, 0x2E1), (10420, M0275I, 0x2E1),
             (10700, M0276I, 0x2E1), (10900, 0xA8047448, 0x2E1), (11100, 0xA8048048, 0x2E1),
             (11400, M0287I, 0x2E1), (11700, M0287I, 0x2E2), (11710, M0288I, 0x2E2),
             (12000, M0288I, 0x2E4), (14000, M0288I, 0x2E6), (14060, 0xA8000348, 0x2E6),
             (14500, 0xA8000348, 0x2E8), (14680, 0xA80022C8, 0x2E8), (15000, 0xA80671C8, 0x2E8),
             (16000, 0xA80671C8, 0x2EA), (16010, 0xA80044C8, 0x2EA), (17000, 0xA80044C8, 0x2EC),
             (17010, 0xA80671C8, 0x2EC), (17100, 0xA80671C8, 0x2EE), (17110, 0xA80044C8, 0x2EE),
             (17800, 0xA80044C8, 0x2F0), (17810, 0xA80671C8, 0x2F0), (18100, 0xA80671C8, 0x2F2),
             (18110, M0050I, 0x2F2), (19000, M0050I, 0x2F4), (19010, d6.CREDITS_TOKEN, 0x2F4)]
    return [ch(f, t, s) for f, t, s in rooms]


class ClassifierTest(unittest.TestCase):
    def test_target_followed_by_unclean_process_is_not_accepted(self):
        cl = d6.Classifier(0x2D6, "credits", warp_token=M0425I)
        feed(cl, BOOT + full_route())
        summary = cl.finish()
        summary.update(rc=0, wall_expired=False, runner_signalled=False, stop_reason="host-quit")
        self.assertTrue(d6.passed(summary))
        for change in ({"rc": -11}, {"rc": 1}, {"wall_expired": True},
                       {"runner_signalled": True}, {"stop_reason": ""},
                       {"stop_reason": "unresolved-boundary"}):
            with self.subTest(change=change):
                self.assertFalse(d6.passed(dict(summary, **change)))

    def test_tokens(self):
        self.assertEqual(d6.room_token("m0425i"), M0425I)
        self.assertEqual(d6.room_name(M0425I), "m0425i")
        self.assertEqual(d6.room_name(d6.CREDITS_TOKEN), "m8888i")
        self.assertEqual(d6.room_name(d6.GAMEOVER_TOKEN), "m0438i")
        self.assertEqual(d6.room_name(d6.M0431I_TOKEN), "m0431i")
        self.assertEqual(d6.room_name(d6.TITLE_TOKEN), "m5000i")
        self.assertEqual(d6.room_token("AA108448"), d6.CREDITS_TOKEN)

    def test_ending_pass_to_credits(self):
        cl = d6.Classifier(0x2D6, "credits", warp_token=M0425I)
        lines = BOOT + full_route()
        end = feed(cl, lines)
        self.assertEqual(end, len(lines) - 1, "run ends on the AA108448 change (linger 0)")
        s = cl.finish()
        self.assertEqual(s["outcome"], "ENDING-REACHED")
        self.assertEqual(len(s["hits"]), len(s["checkpoints"]))
        self.assertEqual(len(s["checkpoints"]), len(d6.DAY6_STORY) + 1)
        self.assertEqual(s["hits"][-1]["token"], "AA108448")
        self.assertEqual(s["drops"], [])
        self.assertTrue(d6.passed(s))
        self.assertEqual(s["visits"][0]["room"], "m0425i", "boot prefix rooms are not visits")

    def test_boot_prefix_title_is_not_a_reset(self):
        cl = d6.Classifier(0x2D6, "credits", warp_token=M0425I)
        self.assertIsNone(feed(cl, BOOT + [ch(1000, M0425I, 0x2D6)]))
        self.assertIsNone(cl.outcome)

    def test_warp_room_starts_without_warp_line(self):
        cl = d6.Classifier(0x2D6, "credits", warp_token=M0425I)
        feed(cl, BOOT[:-1] + [ch(1000, M0425I, 0x2D6), ch(1600, M0275I, 0x2D6)])
        self.assertEqual([v["room"] for v in cl.visits], ["m0425i", "m0275i"])

    def test_game_over_in_chase(self):
        cl = d6.Classifier(0x2E1, "credits", warp_token=M0426I)
        lines = ["[WARP] env transfer A8000148 -> A8082348 story=000002E1 frame=1000",
                 ch(1000, M0426I, 0x2E1), ch(1121, M0275I, 0x2E1), tick(1500, M0275I, 0x2E1),
                 ch(1623, d6.GAMEOVER_TOKEN, 0x2E1), ch(1825, d6.TITLE_TOKEN, 0)]
        end = feed(cl, lines)
        self.assertEqual(end, 4, "the run ends on the m0438i entry")
        s = cl.finish()
        self.assertEqual(s["outcome"], "GAME_OVER")
        self.assertIn("m0275i", s["reason"])
        self.assertFalse(d6.passed(s))

    def test_shutdown_tail_is_recorded_not_reclassified(self):
        cl = d6.Classifier(0x2E1, "credits", warp_token=M0426I)
        feed(cl, ["[WARP] env transfer A8000148 -> A8082348 story=000002E1 frame=1000",
                  ch(1000, M0426I, 0x2E1), ch(1623, d6.GAMEOVER_TOKEN, 0x2E1)])
        for line in ("[SCREENSHOT] /x/end.ppm (320x240)", "[HOST] stop_reason=host-quit",
                     "[HOST] final room token=A8083448 story persist[74]=000002E1"):
            cl.tail(line)
        s = cl.finish()
        self.assertEqual((s["outcome"], s["stop_reason"], s["tail_markers"]), ("GAME_OVER", "host-quit", {}))
        cl = d6.Classifier(0x2F2, "credits", warp_token=M0050I)
        feed(cl, ["[WARP] env transfer A8000148 -> A8005048 story=000002F2 frame=1000",
                  ch(1000, M0050I, 0x2F2), ch(1900, M0050I, 0x2F4), ch(1910, d6.CREDITS_TOKEN, 0x2F4)])
        cl.tail("[STOP] reason=unresolved-boundary")
        s = cl.finish()
        self.assertEqual(s["outcome"], "ENDING-REACHED")
        self.assertEqual(s["tail_markers"], {"stop": 1})
        self.assertFalse(d6.passed(s), "a marker in the shutdown tail still fails the run")

    def test_reset_title_without_game_over(self):
        cl = d6.Classifier(0x2DB, "credits", warp_token=0xA8082448)
        lines = ["[WARP] env transfer A8000148 -> A8082448 story=000002DB frame=1000",
                 ch(1000, 0xA8082448, 0x2DB), ch(1700, d6.TITLE_TOKEN, 0)]
        self.assertEqual(feed(cl, lines), 2)
        self.assertEqual(cl.finish()["outcome"], "RESET")

    def test_story_below_0x80_is_a_reset(self):
        cl = d6.Classifier(0x2DB, "credits", warp_token=0xA8082448)
        lines = ["[WARP] env transfer A8000148 -> A8082448 story=000002DB frame=1000",
                 ch(1000, 0xA8082448, 0x2DB), ch(1700, 0xA8082448, 0x10)]
        self.assertEqual(feed(cl, lines), 2)
        self.assertEqual(cl.finish()["outcome"], "RESET")

    def test_story_drop_is_logged_not_failed(self):
        cl = d6.Classifier(0x2DD, "story>=2DF", warp_token=M0430I)
        lines = ["[WARP] env transfer A8000148 -> A8083048 story=000002DD frame=1000",
                 ch(1000, M0430I, 0x2DD), ch(1100, M0430I, 0x2DE), ch(1200, M0430I, 0x2DD),
                 ch(1300, M0430I, 0x2DF)]
        self.assertEqual(feed(cl, lines), 4)
        s = cl.finish()
        self.assertEqual(s["outcome"], "TARGET-REACHED")
        self.assertEqual(s["drops"], ["0x2DE->0x2DD@1200"])
        self.assertEqual(s["checkpoints"], ["story 0x2DE", "story 0x2DF"])

    def test_native_stop_markers(self):
        for bad in ("[CPU_BOUNDARY] REFUSED func_80069B08 ra=0x80012345",
                    "[STOP] reason=unresolved-boundary",
                    "FATAL: guest trap",
                    "[VM] unported script opcode fn=0x80012345 at pc=0x801A0000 actor=0x800F0000",
                    "[CREDITS] boundary credits_xa_file_absent"):
            cl = d6.Classifier(0x2D6, "credits", warp_token=M0425I)
            end = feed(cl, BOOT + [ch(1000, M0425I, 0x2D6), bad, ch(1600, M0275I, 0x2D6)])
            self.assertEqual(end, len(BOOT) + 1, bad)
            s = cl.finish()
            self.assertEqual(s["outcome"], "NATIVE-STOP", bad)
            self.assertEqual(s["marker_lines"][0]["room"], "m0425i")

    def test_stop_reason(self):
        cl = d6.Classifier(0x2D6, "credits", warp_token=M0425I)
        self.assertIsNotNone(feed(cl, BOOT + [ch(1000, M0425I, 0x2D6), "[HOST] stop_reason=guest-trap"]))
        self.assertEqual(cl.finish()["outcome"], "NATIVE-STOP")
        cl = d6.Classifier(0x2D6, "credits", warp_token=M0425I)
        self.assertIsNone(feed(cl, BOOT + [ch(1000, M0425I, 0x2D6), "[HOST] stop_reason=frame-limit",
                                           "[HOST] final room token=A80822C8 story persist[74]=000002D6"]))
        s = cl.finish("process exit rc=0")
        self.assertEqual(s["outcome"], "STALL")
        self.assertIn("before 'story 0x2D8'", s["reason"])
        self.assertIn("stop_reason=frame-limit", s["reason"])

    def test_stall_without_progress(self):
        cl = d6.Classifier(0x2D6, "credits", stall=3000, warp_token=M0425I)
        lines = BOOT + [ch(1000, M0425I, 0x2D6), ch(1600, M0275I, 0x2D6)] + \
            [tick(f, M0275I, 0x2D6) for f in range(2000, 5001, 500)]
        end = feed(cl, lines)
        self.assertEqual(end, len(lines) - 1, "STALL at frame 4600+ (>= 3000 after 1600)")
        s = cl.finish()
        self.assertEqual(s["outcome"], "STALL")
        self.assertIn("m0275i", s["reason"])

    def test_no_stall_inside_credits(self):
        cl = d6.Classifier(0x2F4, "ending", stall=3000, warp_token=d6.CREDITS_TOKEN)
        lines = ["[WARP] env transfer A8000148 -> AA108448 story=000002F4 frame=1000",
                 ch(1000, d6.CREDITS_TOKEN, 0x2F4)] + \
            [tick(f, d6.CREDITS_TOKEN, 0x2F4) for f in range(1500, 36001, 500)]
        self.assertIsNone(feed(cl, lines))
        self.assertIsNone(cl.outcome)

    def test_until_ending_save_and_title(self):
        head = ["[WARP] env transfer A8000148 -> AA108448 story=000002F4 frame=1000",
                ch(1000, d6.CREDITS_TOKEN, 0x2F4), ch(36201, d6.M0431I_TOKEN, 0x2F4),
                ch(36443, d6.M0431I_TOKEN, 0)]
        save = ["[MEMCARD] created formatted card image /x/memcard1.mcd",
                "[MEMCARD] port 0: created BASLUS-006620000001A (1 block(s) from 1)",
                "[MEMCARD] port 0: closed file at block 1 after writing to 8192/8192, saved /x/memcard1.mcd"]
        cl = d6.Classifier(0x2F4, "ending", warp_token=d6.CREDITS_TOKEN)
        self.assertEqual(feed(cl, head + save), len(head) + 2, "story 0 in m0431i is not a reset")
        s = cl.finish()
        self.assertEqual(s["outcome"], "ENDING-REACHED")
        self.assertIn("clear-data save", s["reason"])
        self.assertEqual(s["drops"], [])
        cl = d6.Classifier(0x2F4, "ending", warp_token=d6.CREDITS_TOKEN)
        lines = head + [ch(37000, d6.M0431I_TOKEN, 0x7D0), ch(37010, d6.TITLE_TOKEN, 0)]
        self.assertEqual(feed(cl, lines), len(lines) - 1)
        s = cl.finish()
        self.assertEqual(s["outcome"], "ENDING-REACHED")
        self.assertIn("title reached", s["reason"])
        cl = d6.Classifier(0x2F4, "ending", warp_token=d6.CREDITS_TOKEN)   # RUN A: no exit after story 0
        lines = head + [tick(f, d6.M0431I_TOKEN, 0) for f in range(36500, 45001, 500)]
        self.assertEqual(feed(cl, lines), lines.index(tick(44500, d6.M0431I_TOKEN, 0)))
        s = cl.finish("process exit rc=0")
        self.assertEqual(s["outcome"], "STALL")
        self.assertIn("m0431i story 0x0", s["reason"])

    def test_title_after_m0431i_without_credits_is_a_reset(self):
        cl = d6.Classifier(0x2F4, "ending", warp_token=d6.M0431I_TOKEN)
        lines = ["[WARP] env transfer A8000148 -> A80830C8 story=000002F4 frame=1000",
                 ch(1000, d6.M0431I_TOKEN, 0x2F4), ch(1362, d6.M0431I_TOKEN, 0)]
        self.assertEqual(feed(cl, lines), 2)
        self.assertEqual(cl.finish()["outcome"], "RESET")

    def test_soft_markers_are_counted_per_room(self):
        cl = d6.Classifier(0x2D6, "credits", warp_token=M0425I)
        lines = BOOT[:4] + ["[STUB:BOOTSTRAP_RET] boot-prefix stub"] + BOOT[4:]
        route = full_route()
        lines += route[:3] + ["[DAY1_SHIM] substituted func_80012345", "not ported: func_80054321"] + route[3:]
        feed(cl, lines)
        s = cl.finish()
        self.assertEqual(s["outcome"], "ENDING-REACHED")
        self.assertEqual(s["markers"], {"stub": 1, "shim": 1, "notported": 1})
        self.assertEqual(s["soft_markers"], 3)
        self.assertFalse(d6.passed(s))
        self.assertEqual(s["marker_lines"][0]["room"], "(boot prefix)")
        self.assertEqual(s["visits"][2]["room"], "m0276i")
        self.assertEqual(s["visits"][2]["markers"], {"shim": 1, "notported": 1})

    def test_host_adapted_is_not_a_marker(self):
        cl = d6.Classifier(0x2D6, "credits", warp_token=M0425I)
        feed(cl, BOOT + ["[STUB:HOST_ADAPTED] func_80014E30_skip_movie (first invocation)"] + full_route())
        s = cl.finish()
        self.assertEqual(s["markers"], {})
        self.assertTrue(d6.passed(s))

    def test_linger_after_pass(self):
        cl = d6.Classifier(0x2F2, "credits", linger=1000, warp_token=M0050I)
        lines = ["[WARP] env transfer A8000148 -> A8005048 story=000002F2 frame=1000",
                 ch(1000, M0050I, 0x2F2), ch(1900, M0050I, 0x2F4), ch(1910, d6.CREDITS_TOKEN, 0x2F4),
                 tick(2000, d6.CREDITS_TOKEN, 0x2F4), tick(2500, d6.CREDITS_TOKEN, 0x2F4),
                 tick(3000, d6.CREDITS_TOKEN, 0x2F4)]
        self.assertEqual(feed(cl, lines), 6)
        self.assertEqual(cl.finish()["outcome"], "ENDING-REACHED")

    def test_target_token_and_story(self):
        cl = d6.Classifier(0x2E1, "token=A8048448", warp_token=M0426I)
        lines = ["[WARP] env transfer A8000148 -> A8082348 story=000002E1 frame=1000",
                 ch(1000, M0426I, 0x2E1), ch(1121, M0275I, 0x2E1), ch(2000, M0287I, 0x2E1),
                 ch(2300, M0287I, 0x2E2), ch(2310, M0288I, 0x2E2)]
        self.assertEqual(feed(cl, lines), 5)
        s = cl.finish()
        self.assertEqual(s["outcome"], "TARGET-REACHED")
        self.assertEqual(s["checkpoints"], ["token A8048448 (m0288i)"])
        self.assertEqual(d6.parse_until("story>=2E2"), ("story>=", 0x2E2))
        with self.assertRaises(ValueError):
            d6.parse_until("title")

    def test_waypoint_file_and_overrides(self):
        with tempfile.TemporaryDirectory() as td:
            p = os.path.join(td, "w.txt")
            with open(p, "w") as f:
                f.write("# comment\n\nA80472C8@2D6=1600,-2150,100   (m0275i: the exit quad)\n"
                        "A8047348@2D6=515,-835,80\n")
            wps = d6.load_waypoints(p)
        self.assertEqual(wps, ["A80472C8@2D6=1600,-2150,100", "A8047348@2D6=515,-835,80"])
        out = d6.apply_overrides(wps, ["A80472C8@2D6=1,2,3", "A8047448@2E1=4,5,6+"])
        self.assertEqual(out, ["A80472C8@2D6=1,2,3", "A8047348@2D6=515,-835,80", "A8047448@2E1=4,5,6+"])


if __name__ == "__main__":
    unittest.main()
