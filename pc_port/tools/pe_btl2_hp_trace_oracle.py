#!/usr/bin/env python3
"""PE-BTL3 TRACE oracle: 0x55 rendezvous through the bound first command.

Runs the 144FC and 293F4 oracles, then checks btl2_trace.csv written by
pe-native-tests. Does not import production C.
"""

from __future__ import annotations

import csv
import pathlib
import subprocess
import sys

COLS = [
    "field_tick",
    "field_scene",
    "field_script_pc",
    "encounter_id",
    "transition_state",
    "battle_mode",
    "formation_id",
    "player_state_hash",
    "persist_hash",
    "rng_state",
    "battle_tick",
]

REQUIRED_STATES = [
    "encounter_55", "hp_copied", "first_command", "command_bound",
    "overlay_wait",
]
FORBIDDEN_STATES = ["mode6_consumed", "wait_mode7", "post_return"]


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    for name in ("pe_btl2_144fc_oracle.py", "pe_btl2_293f4_oracle.py"):
        rc = subprocess.run(
            [sys.executable, str(root / "pc_port" / "tools" / name)],
            check=False,
        )
        require(rc.returncode == 0, name)

    contract = (
        root / "docs" / "evidence" / "pe-btl0-field-battle-handoff"
        / "TRACE_CONTRACT.md"
    ).read_text()
    for col in COLS:
        require(col in contract, f"contract missing column {col}")
    require("hp_copied" in contract, "hp_copied")
    require("encounter_55" in contract, "encounter_55")
    require("first_command" in contract, "first_command named as absent")
    require("command_bound" in contract, "command_bound named as absent")

    csv_path = root / "pc_port" / "build" / "btl2_trace.csv"
    require(csv_path.is_file(), f"missing {csv_path} (run pe-native-tests first)")
    with csv_path.open(newline="") as fh:
        reader = csv.DictReader(fh)
        require(reader.fieldnames == COLS, f"header {reader.fieldnames}")
        rows = list(reader)
    got = [r["transition_state"] for r in rows]
    for state in REQUIRED_STATES:
        require(state in got, f"missing {state}")
    for state in FORBIDDEN_STATES:
        require(state not in got, f"unexpected {state}")
    for row in rows:
        require(row["battle_mode"] == "0", "mode stays 0")
        require(row["encounter_id"] == "m0005i_mod6_4140", "NYPD 0x55(2)")
    hp = [r for r in rows if r["transition_state"] == "hp_copied"]
    require(len(hp) == 1, "one hp_copied row")
    require(hp[0]["player_state_hash"] != "partial", "HP hash after copy")
    require(len(hp[0]["player_state_hash"]) == 64, "sha256 hex")
    command = [r for r in rows if r["transition_state"] == "first_command"]
    require(len(command) == 1, "one first_command row")
    require(command[0]["player_state_hash"] == hp[0]["player_state_hash"],
            "HP hash stable through first command")
    bound = [r for r in rows if r["transition_state"] == "command_bound"]
    require(len(bound) == 1, "one command_bound row")
    require(bound[0]["player_state_hash"] == hp[0]["player_state_hash"],
            "HP hash stable through command_bound")
    wait = [r for r in rows if r["transition_state"] == "overlay_wait"]
    require(len(wait) == 1, "one overlay_wait row")
    require(wait[0]["player_state_hash"] == hp[0]["player_state_hash"],
            "HP hash stable through overlay_wait")
    print(
        "PASS: BTL3 TRACE encounter_55 → hp_copied → first_command → "
        "command_bound → overlay_wait; mode 0"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
