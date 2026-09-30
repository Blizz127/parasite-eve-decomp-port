#!/usr/bin/env python3
"""Enumerate the retail map-exit selector's persist[74] -> room mapping.

The transition overlay loaded by the M0000I sentinel (base 0x8018EFF0) contains
the exit selector at 0x80192030. This runs the *original* MIPS selector under
the project's MIPS interpreter for each story/progress value and records the
room token it selects (0x8009D280), so the Day-1 exit and the Day-2 terminal
handoff can be read straight off the retail code path rather than inferred.

Inputs are the story word (persist[74] = 0x800A7918), the flags word
(0x800A77FC) and the exit-menu selection (0x8019CA68). Static harness only:
one selector invocation per case; no frame loop, no async handlers.
"""
from __future__ import annotations

import hashlib
import json
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "pc_port/tools"))
from pe_battle_hud_oracle import execute  # noqa: E402
from pe_btl14_m0005i_publish_oracle import (  # noqa: E402
    find_disc,
    read_form1,
    load_u32,
    decode_token,
)

EXE = ROOT / "build/disc1.candidate.exe"
EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
SELECTOR = 0x80192030
SELECTOR_STOP = (0x80074D28,)
STORY_ADDR = 0x800A7918
FLAGS_ADDR = 0x800A77FC
SELECTION_ADDR = 0x8019CA68
DEST_ADDR = 0x8009D280
PERSIST1_ADDR = 0x800A77F4


def load() -> tuple[bytes, Path, bytes, int]:
    exe = EXE.read_bytes()
    assert hashlib.sha1(exe).hexdigest() == EXE_SHA1, "EXE SHA-1"
    disc = find_disc(ROOT)
    assert disc is not None, "Disc 1 BIN required (local/pe_disc1.path)"
    ranges = struct.unpack_from("<5H", exe, 0x93168 - 0x10000 + 0x800)
    base = load_u32(exe, 0x80011614)
    overlay = read_form1(disc, 1013 + ranges[3], ranges[4] - ranges[3])
    return exe, disc, overlay, base


def run_case(
    exe: bytes, overlay: bytes, base: int, story: int, flags: int, selection: int
) -> dict:
    ram = bytearray(0x200000)
    ram[0x10000 : 0x10000 + len(exe) - 0x800] = exe[0x800:]
    ram[base & 0x1FFFFF : (base & 0x1FFFFF) + len(overlay)] = overlay
    for addr, value in (
        (STORY_ADDR, story),
        (FLAGS_ADDR, flags),
        (SELECTION_ADDR, selection),
    ):
        struct.pack_into("<I", ram, addr & 0x1FFFFF, value)
    regs = execute(ram, SELECTOR, stop_at=SELECTOR_STOP)
    assert regs[31] == 0x80192254, "selector did not reach its first SDK call"
    token = struct.unpack_from("<I", ram, DEST_ADDR & 0x1FFFFF)[0]
    dest = decode_token(exe, token).rstrip(b"\0").decode("ascii")
    return {
        "story": f"{story:08X}",
        "flags": f"{flags:08X}",
        "selection": f"{selection:08X}",
        "destination": dest,
        "token": f"{token:08X}",
        "persist1": f"{struct.unpack_from('<I', ram, PERSIST1_ADDR & 0x1FFFFF)[0]:08X}",
    }


def main() -> int:
    exe, disc, overlay, base = load()
    stories = list(range(0, 0x160)) + [0x3E6, 0x7FFFFFFF, 0xFFFFFFFF]
    cases = [run_case(exe, overlay, base, s, 0, 0xFFFFFFFF) for s in stories]
    # flags=0x2000 is the Day-2 branch exercised by the audit; confirm it is
    # invariant for the terminal markers.
    extra = [
        run_case(exe, overlay, base, s, 0x2000, 0xFFFFFFFF)
        for s in (0x78, 0x80, 0x88, 0x138, 0x140)
    ]
    # group contiguous runs of identical destination
    groups = []
    for case in cases:
        if groups and groups[-1]["destination"] == case["destination"]:
            groups[-1]["last"] = case["story"]
            groups[-1]["count"] += 1
        else:
            groups.append(
                {
                    "first": case["story"],
                    "last": case["story"],
                    "count": 1,
                    "destination": case["destination"],
                    "persist1": case["persist1"],
                }
            )
    payload = {
        "scope": (
            "retail transition-overlay exit selector @80192030 executed per "
            "persist[74] value by the project MIPS interpreter; one invocation "
            "per case, no async handlers or frame loop"
        ),
        "overlay_base": f"{base:08X}",
        "selector_pc": f"{SELECTOR:08X}",
        "story_address": f"{STORY_ADDR:08X}",
        "cases": cases,
        "groups": groups,
        "flags_2000_cases": extra,
    }
    # Pin the proven mapping: the selector has exactly two non-default exits.
    by_story = {c["story"]: c for c in cases}
    assert by_story["00000078"]["destination"] == "M0036I", "Day-1 exit"
    assert by_story["00000080"]["destination"] == "M0351I", "0x80 hop"
    for story, case in by_story.items():
        if story not in ("00000078", "00000080"):
            assert case["destination"] == "M5000I", (story, case["destination"])
        assert case["persist1"] == "000003E7", story
    for case in extra:
        reference = by_story[case["story"]]
        assert case["destination"] == reference["destination"], ("flags 0x2000", case)
        assert case["persist1"] == reference["persist1"], ("flags 0x2000", case)
    out = ROOT / "local/live/story-destination-map.json"
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(payload, indent=2) + "\n")
    for g in groups:
        print(
            f"{g['first']}..{g['last']} ({g['count']:3d}) -> {g['destination']} "
            f"persist1={g['persist1']}"
        )
    print(out)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
