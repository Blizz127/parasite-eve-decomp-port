#!/usr/bin/env python3
"""Full-image verification sweep over every matched `c` span.

Motivation: an oversized declared span can *mask* a non-exact match.  The
historic example was ``func_800906B4`` — declared ``0x68`` while its C compiled
to ``0x30``; the surplus swallowed the next real function and hid a
register-allocation mismatch.  A per-leaf ``LINK_EXACT`` check at the wrong size
does not catch this, because the comparator reads ``min(linked, size)`` words and
happily tolerates ``size > linked``.

This driver verifies, for every ``c`` span in ``configs/USA/disc1.yaml``:

  A. **span size** — compiled ``.text`` is at least the declared span, and the
     bytes past the span are all zero (the documented gas 16-byte alignment
     pad).  ``compiled < span`` or a non-zero tail is a FAIL.
  B. **link exactness** — the object, linked at its retail VMA with every
     undefined symbol bound to its address-named retail value, matches the
     retail EXE word-for-word over the whole declared span, with zero non-zero
     pad past it.
  C. **profile necessity** — deferred to ``tools/analysis/profile_necessity.py``
     (era leaves); this driver records the assigned profile so the two results
     can be joined.
  D. **source exists** — the span's ``src/*.c`` is present (``build_plan``
     already enforces this; reported for completeness).

It is one command, idempotent, and fail-loud.  Exit 0 iff every leaf passes.

Usage:
  tools/analysis/verify_matched_leaves.py                  # whole image
  tools/analysis/verify_matched_leaves.py --jobs 12
  tools/analysis/verify_matched_leaves.py --only func_800906B4
  tools/analysis/verify_matched_leaves.py --json docs/evidence/verify-mask/report.json
  tools/analysis/verify_matched_leaves.py --allow-truncate-pad   # diagnostic only
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import struct
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from dataclasses import asdict, dataclass, field
from pathlib import Path

_THIS_DIR = Path(__file__).resolve().parent
REPO_ROOT = _THIS_DIR.parents[1]
for candidate in (REPO_ROOT / "tools/build", REPO_ROOT / "tools/analysis"):
    if str(candidate) not in sys.path:
        sys.path.insert(0, str(candidate))

from disc1_plan import PlanError, build_plan  # noqa: E402
from leaf_strong_check import (  # noqa: E402
    LOAD_FILE_START,
    LOAD_VRAM,
    check_object,
    resolve_nm,
    sanitize_environment,
)

EXE_REL = Path("build/extracted/disc1/SLUS_006.62")


@dataclass
class LeafResult:
    name: str
    index: int
    toolchain: str
    profile: str | None
    start: int
    vram: int
    span: int
    compiled: int | None = None
    size_ok: bool = False
    tail_nonzero: bool = False
    mismatches: int | None = None
    pad_nonzero: int | None = None
    link_ok: bool = False
    terminator: str | None = None
    terminator_ok: bool = False
    jr_ra_total: int | None = None
    interior_jr_ra: list[int] = field(default_factory=list)
    interior_count: int | None = None
    interior_ok: bool = False
    error: str | None = None
    first_mismatch: list[list[int]] = field(default_factory=list)

    @property
    def ok(self) -> bool:
        return (
            self.error is None
            and self.size_ok
            and self.link_ok
            and self.terminator_ok
            and self.interior_ok
        )

    def failures(self) -> list[str]:
        out: list[str] = []
        if self.error:
            out.append("build")
        if not self.size_ok:
            out.append("size")
        if self.tail_nonzero:
            out.append("tail")
        if not self.link_ok:
            out.append("link")
        if not self.terminator_ok:
            out.append("terminator")
        if not self.interior_ok:
            out.append("interior")
        return out


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=REPO_ROOT)
    parser.add_argument("--config", type=Path, default=Path("configs/USA/disc1.yaml"))
    parser.add_argument(
        "--profiles", type=Path, default=Path("configs/USA/disc1_build_profiles.json")
    )
    parser.add_argument("--jobs", type=int, default=min(12, os.cpu_count() or 4))
    parser.add_argument("--only", action="append", default=[], metavar="FUNC")
    parser.add_argument("--json", type=Path, help="write the full per-leaf report as JSON")
    parser.add_argument(
        "--allow-truncate-pad",
        action="store_true",
        help="diagnostic: accept span > compiled (masks the defect under study)",
    )
    parser.add_argument("--quiet", action="store_true")
    args = parser.parse_args(argv)

    root = args.root.resolve()
    leaked = sanitize_environment(root)
    retail_path = root / EXE_REL
    if not retail_path.is_file():
        print(f"verify_matched_leaves: missing retail EXE {retail_path}", file=sys.stderr)
        return 2
    retail = retail_path.read_bytes()

    config_path = args.config if args.config.is_absolute() else root / args.config
    profiles_path = args.profiles if args.profiles.is_absolute() else root / args.profiles
    # Freeze the authoritative inputs without changing the plan identity (the
    # plan hash covers the configured path, so the live path must stay the
    # authority).  Read the bytes, build the plan, then re-read: if the bytes
    # are unchanged the plan provably corresponds to the hash we report.  A
    # sibling writing between the two reads would change the bytes and we retry.
    for _ in range(4):
        plan_bytes = config_path.read_bytes()
        profiles_bytes = profiles_path.read_bytes()
        try:
            plan = build_plan(root=root, config=args.config, profiles_path=args.profiles)
        except PlanError as exc:
            print(f"verify_matched_leaves: plan error: {exc}", file=sys.stderr)
            return 2
        if config_path.read_bytes() == plan_bytes and profiles_path.read_bytes() == profiles_bytes:
            break
    else:
        print(
            "verify_matched_leaves: FAIL race — configs changed on every read; "
            "rerun when siblings pause",
            file=sys.stderr,
        )
        return 3
    yaml_sha = hashlib.sha256(plan_bytes).hexdigest()
    profiles_sha = hashlib.sha256(profiles_bytes).hexdigest()

    units = [u for u in plan["units"] if u["kind"] == "c"]
    if args.only:
        wanted = set(args.only)
        units = [u for u in units if u["name"] in wanted]
        missing = wanted - {u["name"] for u in units}
        if missing:
            print(f"verify_matched_leaves: unknown --only: {sorted(missing)}", file=sys.stderr)
            return 2

    def sources_fingerprint() -> str:
        digest = hashlib.sha256()
        for unit in units:
            source = root / unit["source"]
            digest.update(unit["name"].encode())
            digest.update(b"\0")
            digest.update(source.read_bytes() if source.is_file() else b"<missing>")
        return digest.hexdigest()

    sources_sha_start = sources_fingerprint()

    # Compile exactly the leaves under test, exactly as the build does (same
    # profiles/flags/knobs), into a scratch object dir inside build/ so the
    # live build tree is untouched and dispatch handling still resolves.
    import disc1_build  # noqa: PLC0415

    try:
        tools = disc1_build.find_toolchain()
    except Exception as exc:
        print(f"verify_matched_leaves: toolchain unavailable: {exc}", file=sys.stderr)
        return 2

    (root / "build").mkdir(parents=True, exist_ok=True)
    scratch = Path(
        tempfile.mkdtemp(prefix="verify-sweep-", dir=(root / "build"))
    )
    build_units = json.loads(json.dumps(units))
    for unit in build_units:
        unit["object"] = str(scratch / Path(unit["object"]).name)
    disc1_build.compile_all({"units": build_units, "header": plan["header"]}, tools)

    ld = tools.linker
    nm = resolve_nm(root)

    results: list[LeafResult] = []
    for i, unit in enumerate(units):
        results.append(
            LeafResult(
                name=unit["name"],
                index=i,
                toolchain=unit["toolchain"],
                profile=unit["profile"],
                start=unit["start"],
                vram=unit["vram"],
                span=unit["size"],
            )
        )

    obj_by_name = {u["name"]: Path(bu["object"]) for u, bu in zip(units, build_units)}

    def check(result: LeafResult) -> LeafResult:
        unit = next(u for u in units if u["name"] == result.name)
        strong = check_object(
            obj_by_name[result.name], unit, retail, ld, nm,
            allow_truncate_pad=args.allow_truncate_pad,
        )
        result.compiled = strong.compiled
        result.size_ok = strong.size_ok
        result.tail_nonzero = strong.tail_nonzero
        result.mismatches = strong.mismatches
        result.pad_nonzero = strong.pad_nonzero
        result.link_ok = strong.link_ok
        result.terminator = strong.terminator
        result.terminator_ok = strong.terminator_ok
        result.jr_ra_total = strong.jr_ra_total
        result.interior_jr_ra = strong.interior_jr_ra
        result.interior_count = strong.interior_count
        result.interior_ok = strong.interior_ok
        result.error = strong.error
        result.first_mismatch = strong.first_mismatch
        return result

    with ThreadPoolExecutor(max_workers=max(1, args.jobs)) as executor:
        results = list(executor.map(check, results))

    import shutil  # noqa: PLC0415

    shutil.rmtree(scratch, ignore_errors=True)

    failures = [r for r in results if not r.ok]
    race = sources_fingerprint() != sources_sha_start
    if config_path.read_bytes() != plan_bytes or profiles_path.read_bytes() != profiles_bytes:
        race = True
        print(
            "verify_matched_leaves: WARN configs changed mid-sweep; results may mix "
            "plan revisions — rerun for a clean proof",
            file=sys.stderr,
        )
    if sources_fingerprint() != sources_sha_start:
        print(
            "verify_matched_leaves: WARN a src/ leaf changed mid-sweep; results may "
            "mix revisions — rerun for a clean proof",
            file=sys.stderr,
        )
    whole = (
        "PASS"
        if not failures and not race
        else (f"FAIL {len(failures)}/{len(results)}" if failures else "RACE")
    )
    if not args.quiet:
        print(
            f"verify-sweep: {len(results)} c leaves, {len(results)-len(failures)} OK, "
            f"{len(failures)} failing — {whole}"
        )
        for r in failures:
            classes = ",".join(r.failures())
            print(f"  FAIL [{classes}] {r.name} span=0x{r.span:X} vram=0x{r.vram:08X} "
                  f"compiled={('0x%X' % r.compiled) if r.compiled is not None else '?'} "
                  f"profile={r.profile} toolchain={r.toolchain}")
            if r.error:
                print(f"       error: {r.error}")
            if r.mismatches:
                print(f"       word mismatches={r.mismatches} pad_nonzero={r.pad_nonzero}")
                for addr, rw, lw in r.first_mismatch:
                    print(f"         {addr:#x}: ROM {rw:08x}  LNK {lw:08x}")
            if not r.size_ok:
                print(
                    f"       size: declared span 0x{r.span:X} > compiled "
                    f"0x{r.compiled:X} (masked-match class)"
                )
            if r.tail_nonzero:
                print("       tail: non-zero bytes past the declared span")
            if not r.terminator_ok:
                print(f"       terminator: span ends in {r.terminator}, not jr $ra / tail j")
            if not r.interior_ok:
                print(
                    f"       interior: {r.interior_count} jr $ra before the terminal "
                    f"one at {[hex(o) for o in r.interior_jr_ra]} (swallowed function?)"
                )

    if leaked:
        print(f"verify-sweep: stripped leaked build knobs from env: {leaked}", file=sys.stderr)

    if args.json:
        out = args.json
        if not out.is_absolute():
            out = root / out
        out.parent.mkdir(parents=True, exist_ok=True)
        payload = {
            "schema_version": 1,
            "plan_sha256": plan["plan_sha256"],
            "yaml_sha256": yaml_sha,
            "profiles_sha256": profiles_sha,
            "sources_sha256": sources_sha_start,
            "sources_stable": not race,
            "spans": plan["counts"],
            "verified": len(results),
            "failing": len(failures),
            "result": whole,
            "leaves": [asdict(r) for r in results],
        }
        out.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
        if not args.quiet:
            print(f"verify-sweep: report -> {out}")

    print(f"VERIFY_SWEEP={whole} leaves={len(results)} plan={plan['plan_sha256']} "
          f"yaml={yaml_sha}")
    return 0 if (not failures and not race) else 1


if __name__ == "__main__":
    raise SystemExit(main())
