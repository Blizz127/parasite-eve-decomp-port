#!/usr/bin/env python3
"""
phase_state.py — read-only state reporter for the Parasite Eve decomp.

Derives, never writes. Every number is produced by the same authoritative
command the project already trusts (yaml grep for leaf count, git for tree
state, the committed $at counter for the family split). If a derivation can't
run, it says so loudly rather than printing a stale or guessed value.

Run from the repo root, or pass --root PATH.

    python3 tools/analysis/phase_state.py
    python3 tools/analysis/phase_state.py --gate      # also runs verify_us.sh (slow)
    python3 tools/analysis/phase_state.py --json       # machine-readable

Nothing here modifies the tree. Safe to run any time.
"""

import argparse
import json
import os
import re
import subprocess
import sys

TARGET_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
YAML_REL = "configs/USA/disc1.yaml"
COUNTER_REL = "tools/analysis/at_absolute_store_counter.py"
BLOCKERS_REL = "docs/ai_context/parked_blockers.json"
VERIFY_REL = "scripts/verify_us.sh"

# Untracked paths that are known-noise and should not be flagged as "dirty".
# Keep this list tiny and explicit — the whole point is that REAL dirt stands out.
KNOWN_UNTRACKED_NOISE = {".venv", "tools/era"}


def run(cmd, root, timeout=60):
    """Run a command, return (ok, stdout, stderr). Never raises."""
    try:
        p = subprocess.run(
            cmd, cwd=root, capture_output=True, text=True, timeout=timeout
        )
        return p.returncode == 0, p.stdout.strip(), p.stderr.strip()
    except FileNotFoundError:
        return False, "", f"command not found: {cmd[0]}"
    except subprocess.TimeoutExpired:
        return False, "", f"timed out after {timeout}s: {' '.join(cmd)}"
    except Exception as e:  # defensive: a reporter must never crash the session
        return False, "", f"{type(e).__name__}: {e}"


def git_tip(root):
    ok, out, err = run(["git", "rev-parse", "--short", "HEAD"], root)
    if not ok:
        return None, err or "not a git repo?"
    sha = out
    ok2, subject, _ = run(["git", "log", "-1", "--pretty=%s"], root)
    return {"sha": sha, "subject": subject if ok2 else ""}, None


def git_dirty(root):
    """Return (real_dirty_lines, noise_lines, error)."""
    # -uall expands untracked directories to individual files, so a dirty file
    # can't hide behind a collapsed "?? src/" entry at a stop.
    ok, out, err = run(["git", "status", "--short", "-uall"], root)
    if not ok:
        return None, None, err or "git status failed"
    real, noise = [], []
    for line in out.splitlines():
        if not line.strip():
            continue
        # format: "XY path"; path starts at col 3
        path = line[3:].strip()
        if path in KNOWN_UNTRACKED_NOISE and line.startswith("??"):
            noise.append(line)
        else:
            real.append(line)
    return real, noise, None


def leaf_count(root):
    """Authoritative leaf count: c-mapped units in disc1.yaml."""
    yaml_path = os.path.join(root, YAML_REL)
    if not os.path.isfile(yaml_path):
        return None, f"missing {YAML_REL}"
    # Mirror the trusted command: grep -c ',\s*c,' configs/USA/disc1.yaml
    count = 0
    pat = re.compile(r",\s*c,")
    try:
        with open(yaml_path, "r", errors="replace") as f:
            for line in f:
                if pat.search(line):
                    count += 1
    except OSError as e:
        return None, str(e)
    return count, None


def family_split(root):
    """Shell out to the committed $at counter; surface its 3-way split.

    The counter hard-fails when asm/ is out of sync with yaml (missing units
    or stale C-leaf glabels still in scanned .s). That failure must surface
    here as FAILED — never as a parsed total. Leaf count is yaml-only and
    independent of this path.
    """
    counter = os.path.join(root, COUNTER_REL)
    if not os.path.isfile(counter):
        return None, f"counter not found at {COUNTER_REL} (not yet promoted?)"
    ok, out, err = run(["python3", counter], root)
    if not ok:
        # Prefer stderr (sync failures); fall back to stdout; never invent a total.
        detail = (err or out or "counter failed to run").strip()
        # Compact multi-line counter noise into one reporter line + key bullets.
        lines = [ln for ln in detail.splitlines() if ln.strip()]
        if not lines:
            return None, "counter failed to run"
        head = lines[0]
        bullets = [ln.strip() for ln in lines[1:] if ln.strip().startswith("- ")]
        if bullets:
            return None, head + " | " + " ".join(bullets[:6])
        return None, head
    # Only parse SUMMARY from a successful counter run.
    # SUMMARY pre-jr=17 delay-slot=14 sb-sh=5 excluded=1 total=36
    m = re.search(
        r"pre-jr=(\d+)\s+delay-slot=(\d+)\s+sb-sh=(\d+)"
        r"(?:\s+excluded=(\d+))?(?:\s+total=(\d+))?",
        out,
    )
    if not m:
        # Don't guess — hand back the raw output so a human can read it.
        return {"raw": out}, None
    return {
        "pre_jr": int(m.group(1)),
        "delay_slot": int(m.group(2)),
        "sb_sh": int(m.group(3)),
        "excluded": int(m.group(4)) if m.group(4) else None,
        "total": int(m.group(5)) if m.group(5) else None,
    }, None


def parked_blockers(root):
    """Read the version-controlled blocker registry. Never hardcoded here."""
    path = os.path.join(root, BLOCKERS_REL)
    if not os.path.isfile(path):
        return None, f"no registry at {BLOCKERS_REL} (create it to track blockers)"
    try:
        with open(path) as f:
            data = json.load(f)
    except (OSError, json.JSONDecodeError) as e:
        return None, f"registry unreadable: {e}"
    return data, None


def run_gate(root):
    """Optionally run verify_us.sh. Slow; only on --gate."""
    verify = os.path.join(root, VERIFY_REL)
    if not os.path.isfile(verify):
        return None, f"missing {VERIFY_REL}"
    ok, out, err = run(["bash", verify], root, timeout=1800)
    tail = (out or err).splitlines()
    found = any(TARGET_SHA1 in ln for ln in tail)
    return {"ran": True, "sha_seen": found, "tail": tail[-6:]}, None


def gather(root, do_gate):
    state = {}
    state["tip"], state["tip_err"] = git_tip(root)
    real, noise, derr = git_dirty(root)
    state["dirty_real"], state["dirty_noise"], state["dirty_err"] = real, noise, derr
    state["leaves"], state["leaves_err"] = leaf_count(root)
    state["family"], state["family_err"] = family_split(root)
    state["blockers"], state["blockers_err"] = parked_blockers(root)
    if do_gate:
        state["gate"], state["gate_err"] = run_gate(root)
    else:
        state["gate"], state["gate_err"] = None, None
    return state


def fmt_text(s):
    L = []
    L.append("=" * 60)
    L.append("  PARASITE EVE — PHASE STATE (read-only)")
    L.append("=" * 60)

    # Tip
    if s["tip"]:
        L.append(f"tip        : {s['tip']['sha']}  {s['tip']['subject']}")
    else:
        L.append(f"tip        : ERROR — {s['tip_err']}")

    # Dirty tree — real dirt is the alarm; noise is footnoted.
    if s["dirty_err"]:
        L.append(f"tree       : ERROR — {s['dirty_err']}")
    elif s["dirty_real"]:
        L.append(f"tree       : *** DIRTY *** {len(s['dirty_real'])} real change(s):")
        for ln in s["dirty_real"]:
            L.append(f"             {ln}")
    else:
        note = f" ({len(s['dirty_noise'])} known-noise untracked ignored)" if s["dirty_noise"] else ""
        L.append(f"tree       : clean{note}")

    # Leaves
    if s["leaves"] is not None:
        L.append(f"leaves     : {s['leaves']}   (grep -c ',\\s*c,' {YAML_REL})")
    else:
        L.append(f"leaves     : ERROR — {s['leaves_err']}")

    # Gate
    if s["gate"] is not None:
        verdict = "SHA MATCH" if s["gate"]["sha_seen"] else "*** SHA NOT SEEN ***"
        L.append(f"gate       : {verdict}")
        for ln in s["gate"]["tail"]:
            L.append(f"             {ln}")
    elif s["gate_err"]:
        L.append(f"gate       : ERROR — {s['gate_err']}")
    else:
        L.append("gate       : not run (pass --gate to verify)")

    # $at family (asm/-derived; FAILED when counter hard-fails on stale/missing asm)
    L.append("-" * 60)
    if s["family_err"]:
        L.append(f"$at family : FAILED — {s['family_err']}")
    elif s["family"] and "raw" in s["family"]:
        L.append("$at family : FAILED — counter ran but no parseable SUMMARY (raw below)")
        for ln in s["family"]["raw"].splitlines():
            L.append(f"             {ln}")
    elif s["family"]:
        f = s["family"]
        extra = ""
        if f.get("excluded") is not None:
            extra += f"  excluded={f['excluded']}"
        if f.get("total") is not None:
            extra += f"  total={f['total']}"
        L.append(f"$at family : pre-jr={f['pre_jr']}  delay-slot={f['delay_slot']}  sb-sh={f['sb_sh']}{extra}")

    # Parked blockers
    L.append("-" * 60)
    if s["blockers_err"]:
        L.append(f"blockers   : {s['blockers_err']}")
    elif s["blockers"]:
        L.append("parked blockers:")
        for b in s["blockers"].get("blockers", []):
            name = b.get("id", "?")
            status = b.get("status", "?")
            note = b.get("note", "")
            L.append(f"  [{status}] {name}: {note}")
    L.append("=" * 60)
    return "\n".join(L)


def main():
    ap = argparse.ArgumentParser(description="Read-only PE decomp state reporter.")
    ap.add_argument("--root", default=".", help="repo root (default: cwd)")
    ap.add_argument("--gate", action="store_true", help="also run verify_us.sh (slow)")
    ap.add_argument("--json", action="store_true", help="machine-readable output")
    args = ap.parse_args()

    root = os.path.abspath(args.root)
    if not os.path.isdir(os.path.join(root, ".git")):
        print(f"warning: {root} has no .git — is this the repo root?", file=sys.stderr)

    state = gather(root, args.gate)

    if args.json:
        print(json.dumps(state, indent=2))
    else:
        print(fmt_text(state))

    # Exit non-zero if the tree is unexpectedly dirty — makes it usable as a
    # pre-phase guard in a shell one-liner without changing anything.
    if state.get("dirty_real"):
        sys.exit(2)


if __name__ == "__main__":
    main()
