#!/usr/bin/env python3
"""Integrate a batch of verified leaves into the YAML plan + build profiles.

Input: one or more JSON files shaped like

  {"matched": [{"name": "func_8004BC80", "vram": "0x8004BC80",
                "size": "0x34", "flags": ["-O2","-G0"], "env": {},
                "notes": "..."}], ...}

For each entry this
  * re-verifies the leaf with `era_link_check.py` under its own flags/env
    (a batch is only as trustworthy as its weakest claim),
  * carves the `c` row via `carve_leaf.py`,
  * resolves flags+env to an existing build profile, or defines a new one,
  * records the assignment only when the profile is NOT the default
    (`profile_necessity.py` treats a redundant assignment as a defect).

Nothing is written until every leaf in the batch has re-verified, so a bad
claim cannot land a half-integrated plan.

Usage: tools/build/integrate_leaves.py <result.json>... [--skip-verify]
"""
import argparse
import json
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
PROFILES = ROOT / "configs" / "USA" / "disc1_build_profiles.json"


def profile_name_for(flags, env):
    """Derive a stable, descriptive profile name from flags + env knobs."""
    parts = ["era"]
    opt = next((f for f in flags if re.fullmatch(r"-O\d", f)), "-O2")
    gp = next((f for f in flags if re.fullmatch(r"-G\d+", f)), "-G0")
    parts.append(opt[1:].lower())
    parts.append(gp[1:].lower())
    for f in flags:
        if f.startswith("-f"):
            parts.append(f[2:].replace("-", "_"))
    # House convention for a folded dispatch table (see the existing
    # era_o2_g0_dispatch_* profiles): the table's VRAM names the profile and
    # the THREE_WORD knob it always travels with is left implicit.
    dispatch = env.get("MASPSX_DISPATCH_FOLD")
    if dispatch:
        parts.append("dispatch")
        # A multi-table fold (comma list) names every table in order.
        for table in dispatch.split(","):
            parts.append(table.strip().replace("jtbl_", "").lower())
    for k, v in sorted(env.items()):
        if k in ("MASPSX_DISPATCH_FOLD",):
            continue
        if dispatch and k == "MASPSX_THREE_WORD_SYMBOL_STORE":
            continue
        tag = k.replace("MASPSX_", "").replace("ERA_", "").lower()
        if k == "MASPSX_FORCE_ABSOLUTE_SYMBOLS":
            # D_8009D2F0 -> d8009d2f0, matching era_o2_g8_force_d8009d2f0_absolute
            syms = [x.strip().replace("_", "").lower() for x in v.split(",")]
            if len(syms) > 3:
                # A long forced-absolute list is a per-leaf property; name it by
                # its first symbol and count rather than spelling out every one.
                tag = f"force_{syms[0]}_plus{len(syms) - 1}_absolute"
            else:
                tag = "force_" + "_".join(syms) + "_absolute"
        elif k == "ERA_ASPSX_VER":
            tag = "aspsx_" + v.replace(".", "")
        elif k == "ERA_CC1_VER":
            tag = "cc1_" + v.replace(".", "")
        parts.append(tag)
    return "_".join(parts)


def find_profile(profiles, flags, env):
    """An exact flags+environment match among the defined profiles, if any."""
    for name, spec in profiles.items():
        if spec.get("toolchain", "era") != "era":
            continue
        if list(spec.get("flags", [])) != list(flags):
            continue
        if dict(spec.get("environment", {})) != dict(env):
            continue
        return name
    return None


def verify(leaf):
    env = dict(os.environ)
    env.update({k: str(v) for k, v in (leaf.get("env") or {}).items()})
    lib = ROOT / "tools" / "mipsel-host" / "usr" / "lib" / "x86_64-linux-gnu"
    env["LD_LIBRARY_PATH"] = f"{lib}:{env.get('LD_LIBRARY_PATH', '')}"
    cmd = [sys.executable, str(ROOT / "tools/analysis/era_link_check.py"),
           f"src/{leaf['name']}.c", leaf["vram"], leaf["size"],
           *(leaf.get("flags") or ["-O2", "-G0"])]
    r = subprocess.run(cmd, cwd=ROOT, env=env, capture_output=True, text=True)
    return r.returncode == 0 and "LINK_EXACT" in r.stdout, (r.stdout + r.stderr).strip()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("results", nargs="+")
    ap.add_argument("--skip-verify", action="store_true")
    args = ap.parse_args()

    leaves = []
    seen = set()
    for path in args.results:
        data = json.loads(Path(path).read_text())
        for leaf in data.get("matched", []):
            if leaf["name"] in seen:
                sys.exit(f"duplicate leaf {leaf['name']} across result files")
            seen.add(leaf["name"])
            leaves.append(leaf)
    if not leaves:
        print("integrate_leaves: nothing to integrate")
        return 0

    rejected = []
    if not args.skip_verify:
        for leaf in leaves:
            ok, out = verify(leaf)
            tail = out.split("\n")[-1] if out else ""
            print(f"  verify {leaf['name']:<18} {'LINK_EXACT' if ok else 'REJECT  ' + tail}")
            if not ok:
                rejected.append(leaf["name"])
    leaves = [l for l in leaves if l["name"] not in rejected]
    if rejected:
        print(f"integrate_leaves: {len(rejected)} rejected, not integrated: "
              f"{' '.join(rejected)}")
    if not leaves:
        return 1

    spec = json.loads(PROFILES.read_text())
    profiles, assignments = spec["profiles"], spec["assignments"]
    default = spec["default_profile"]
    new_profiles = []

    for leaf in leaves:
        flags = list(leaf.get("flags") or ["-O2", "-G0"])
        env = {k: str(v) for k, v in (leaf.get("env") or {}).items()}
        name = find_profile(profiles, flags, env)
        if name is None:
            name = profile_name_for(flags, env)
            body = {"toolchain": "era", "flags": flags}
            if env:
                body["environment"] = env
            profiles[name] = body
            new_profiles.append(name)
        for plist in assignments.values():
            if leaf["name"] in plist:
                plist.remove(leaf["name"])
        if name != default:
            assignments.setdefault(name, []).append(leaf["name"])
        leaf["_profile"] = name

        r = subprocess.run(
            [sys.executable, str(ROOT / "tools/build/carve_leaf.py"),
             leaf["name"], leaf["size"],
             "--comment", (leaf.get("notes") or "matching C leaf")[:96]],
            cwd=ROOT, capture_output=True, text=True)
        print(f"  carve  {leaf['name']:<18} {r.stdout.strip() or r.stderr.strip()}")
        if r.returncode != 0:
            sys.exit(f"carve failed for {leaf['name']}")

    assignments = {k: v for k, v in assignments.items() if v}
    spec["assignments"] = assignments
    PROFILES.write_text(json.dumps(spec, indent=2) + "\n")
    print(f"integrate_leaves: {len(leaves)} leaves, "
          f"{len(new_profiles)} new profiles ({', '.join(new_profiles) or 'none'})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
