#!/usr/bin/env python3
"""Loud check that the build-profile record is truthful and load-bearing.

``configs/USA/disc1_build_profiles.json`` may declare a per-leaf profile, but a
missing or stale assignment silently falls back to ``default_profile``. That is
a correctness-of-record hazard in both directions:

  * **MISSING** — a leaf that the default profile does NOT reproduce has no
    assignment, so it silently falls back and (if the object happens to still
    match) the record lies about how it was built. Dangerous.
  * **WRONG** — a leaf's assigned profile does not reproduce it at all.
  * **REDUNDANT** — a leaf's assigned profile is not load-bearing: the default
    profile reproduces it too, so the manifest claims a profile that isn't
    required.

The check compiles each era-toolchain leaf at its retail VMA via
``tools/analysis/era_link_check.py`` (which honors the profile's flags, the
maspsx knobs ``MASPSX_FORCE_ABSOLUTE_SYMBOLS``, ``ERA_ASPSX_VER`` and
``MASPSX_EXPAND_DIV``, and the per-leaf cc1 selector ``ERA_CC1_VER``) and
asserts:

    default exact  -> assignment must be absent (or is REDUNDANT)
    default !exact -> assignment must be present, and exact (else MISSING/WRONG)

Default and per-profile results are cached on disk (``--cache``), so reruns are
cheap. Non-era (modern-toolchain) leaves are reported as unverifiable, because
``era_link_check.py`` cannot reproduce non-era codegen — they are neither
passes nor failures.

Usage:
  python3 tools/analysis/profile_necessity.py                 # full audit
  python3 tools/analysis/profile_necessity.py --limit 20      # smoke
  python3 tools/analysis/profile_necessity.py --only func_80075B84
  python3 tools/analysis/profile_necessity.py --profiles /tmp/mutated.json
  python3 tools/analysis/profile_necessity.py --refresh       # ignore cache
"""
from __future__ import annotations

import argparse
import concurrent.futures
import hashlib
import json
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(ROOT / "tools" / "build"))

from disc1_plan import PlanError, build_plan  # noqa: E402

LINK_CHECK = ROOT / "tools" / "analysis" / "era_link_check.py"
DEFAULT_PROFILES = ROOT / "configs" / "USA" / "disc1_build_profiles.json"
DEFAULT_CACHE = ROOT / "build" / "profile_necessity_cache.json"


def _exact(src: Path, vram: int, size: int, flags: list[str], env: dict[str, str],
           cache: dict, cache_key: str, refresh: bool) -> tuple[bool, list[str]]:
    """Return (exact, detail) for one compile/link/compare of a leaf."""
    # Key on everything that changes the bytes: source, flags, environment and
    # the vendored maspsx (a profile fixed under the same name, or a new knob,
    # must not reuse a stale result).
    digest = hashlib.sha1()
    digest.update(Path(src).read_bytes())
    digest.update(json.dumps([flags, sorted(env.items())]).encode())
    maspsx = ROOT / "tools" / "era" / "maspsx" / "maspsx" / "__init__.py"
    if maspsx.is_file():
        digest.update(maspsx.read_bytes())
    cache_key = f"{cache_key}|{digest.hexdigest()}"
    if not refresh and cache_key in cache:
        return cache[cache_key]["exact"], cache[cache_key]["out"]
    child = os.environ.copy()
    # Strip leaked per-leaf knobs (maspsx patches, ERA_ASPSX_VER, ERA_CC1_VER)
    # so the default run really is the default; the profile's own environment
    # block is layered back on below.
    for name in list(child):
        if name.startswith(("MASPSX_", "ERA_")):
            child.pop(name)
    child.update(env)
    child["PATH"] = (
        f"{ROOT / 'tools' / 'mipsel-host' / 'bin'}{os.pathsep}{child.get('PATH', '')}"
    )
    run = subprocess.run(
        [sys.executable, str(LINK_CHECK), str(src), hex(vram), hex(size), *flags],
        capture_output=True,
        text=True,
        env=child,
        cwd=str(ROOT),
    )
    out = (run.stdout + run.stderr).strip().splitlines()
    exact = "LINK_EXACT" in (run.stdout + run.stderr) and run.returncode == 0
    cache[cache_key] = {"exact": exact, "out": out}
    return exact, out


def _unit_facts(unit: dict) -> dict:
    return {
        "name": unit["name"],
        "source": ROOT / unit["source"],
        "vram": unit["vram"],
        "size": unit["size"],
        "profile": unit["profile"],
        "toolchain": unit["toolchain"],
        "flags": list(unit["flags"]),
        "environment": dict(unit["environment"]),
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--only", action="append", default=[], help="leaf name(s)")
    parser.add_argument("--limit", type=int, default=0, help="first N leaves")
    parser.add_argument("--profiles", type=Path, default=DEFAULT_PROFILES)
    parser.add_argument("--cache", type=Path, default=DEFAULT_CACHE)
    parser.add_argument("--refresh", action="store_true", help="ignore cache")
    parser.add_argument("--jobs", type=int, default=os.cpu_count() or 4)
    parser.add_argument("--allow-redundant", action="store_true",
                        help="report REDUNDANT but exit 0 (WRONG/MISSING still fail)")
    args = parser.parse_args(argv)

    try:
        plan = build_plan(root=ROOT, profiles_path=args.profiles)
    except PlanError as exc:
        print(f"PROFILE_NECESSITY=FAIL (manifest rejected: {exc})")
        return 1

    default_name = json.loads(args.profiles.read_text())["default_profile"]
    defaults = next(u for u in plan["units"]
                    if u["kind"] == "c" and u["profile"] == default_name)
    default_facts = _unit_facts(defaults)

    all_c = [_unit_facts(u) for u in plan["units"] if u["kind"] == "c"]
    if args.only:
        wanted = set(args.only)
        all_c = [u for u in all_c if u["name"] in wanted]
    if args.limit:
        all_c = all_c[: args.limit]

    era = [u for u in all_c if u["toolchain"] == "era"]
    modern = [u for u in all_c if u["toolchain"] != "era"]

    try:
        cache = json.loads(args.cache.read_text())["entries"]
    except Exception:
        cache = {}
    manifest_hash = hashlib.sha256(args.profiles.read_bytes()).hexdigest()[:16]
    cache["__manifest__"] = {"hash": manifest_hash}
    if not args.refresh and cache.get("__manifest__", {}).get("hash") != manifest_hash:
        cache = {"__manifest__": {"hash": manifest_hash}}

    print(f"default_profile={default_name} flags={' '.join(default_facts['flags'])} "
          f"env={default_facts['environment'] or '{}'}")
    print(f"era leaves checked: {len(era)}; non-era (unverifiable): {len(modern)}\n")

    def default_exact(u: dict):
        key = f"default|{u['name']}|{u['profile']}|{u['vram']:x}|{u['size']:x}"
        return _exact(u["source"], u["vram"], u["size"], default_facts["flags"],
                      default_facts["environment"], cache, key, args.refresh)

    def assigned_exact(u: dict):
        key = f"assigned|{u['name']}|{u['profile']}|{u['vram']:x}|{u['size']:x}"
        return _exact(u["source"], u["vram"], u["size"], u["flags"],
                      u["environment"], cache, key, args.refresh)

    with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, args.jobs)) as pool:
        defaults_map = dict(zip(
            (u["name"] for u in era), pool.map(default_exact, era)))
    need = [u for u in era if not defaults_map[u["name"]][0]
            and u["profile"] != default_name]
    with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, args.jobs)) as pool:
        assigned_map = dict(zip(
            (u["name"] for u in need), pool.map(assigned_exact, need)))

    rows = []
    for u in era:
        d_exact, _ = defaults_map[u["name"]]
        if u["profile"] == default_name:
            verdict = "OK" if d_exact else "MISSING"
        elif not d_exact:
            a_exact, _ = assigned_map[u["name"]]
            verdict = "OK" if a_exact else "WRONG"
        else:
            verdict = "REDUNDANT"
        rows.append((verdict, u))

    for verdict, u in rows:
        if verdict != "OK":
            print(f"  {verdict:16} {u['name']:18} {u['profile']}")

    bad = [r for r in rows if r[0] != "OK"]
    for verdict, u in bad:
        print(f"\n--- {verdict}: {u['name']} ({u['profile']})")
        if verdict == "MISSING":
            print("  the default profile does NOT reproduce this leaf and it has "
                  "no assignment: it is silently falling back to a profile that "
                  "cannot build it. Add an explicit assignment.")
        elif verdict == "WRONG":
            print("  the assigned profile does not reproduce this leaf.")
        else:
            print("  the default profile reproduces this leaf too, so the "
                  "assignment is stale decoration. Remove it (or find a profile "
                  "that is actually load-bearing).")
        for line in (defaults_map[u["name"]][1]
                     if verdict == "MISSING" else assigned_map.get(u["name"], (None, []))[1])[-8:]:
            print(f"    {line}")

    args.cache.parent.mkdir(parents=True, exist_ok=True)
    args.cache.write_text(json.dumps({"entries": cache}, indent=0) + "\n")

    ok = sum(1 for r in rows if r[0] == "OK")
    hard = [r for r in bad if r[0] != "REDUNDANT"]
    redundant = [r for r in bad if r[0] == "REDUNDANT"]
    print(f"\nprofile-necessity: {ok}/{len(rows)} era leaves clean; "
          f"{len(hard)} hard defect(s); {len(redundant)} redundant")
    if hard or (redundant and not args.allow_redundant):
        print("PROFILE_NECESSITY=FAIL")
        return 1
    print("PROFILE_NECESSITY=PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
