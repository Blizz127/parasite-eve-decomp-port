#!/usr/bin/env python3
"""Derive the Disc 1 build and verification plans from the splat YAML.

The YAML owns span membership, order, and geometry.  The JSON build-profile
file owns only compiler behavior that cannot be inferred from a span.  This
module deliberately uses only the Python standard library so the public
consistency gate does not need the retail image, splat, or PyYAML.

Every function is parametrized over a rebuild ``Target`` (``overlay_targets``):
the default, ``MAIN_TARGET``, is the Disc 1 executable with its historical
geometry (0x800 PS-X header, VRAM 0x80010000, file end 0x1EE800, ``src/``), so
the default behaviour and the plan hash are unchanged; ``--target <id>`` selects
a PE.IMG overlay from ``configs/USA/overlays/manifest.yaml`` (no header, fixed
VRAM, sources under ``src/overlays/<id>/``).
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import subprocess
import sys
from collections import Counter
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any

_THIS_DIR = Path(__file__).resolve().parent
if str(_THIS_DIR) not in sys.path:
    sys.path.insert(0, str(_THIS_DIR))

from overlay_targets import MAIN_TARGET, Target, TargetError, load_target  # noqa: E402


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_CONFIG = Path(MAIN_TARGET.config)
DEFAULT_PROFILES = Path(MAIN_TARGET.profiles)
DEFAULT_STATUS = Path("docs/generated/DISC1_MATCHING_STATUS.md")
EXPECTED_FILE_END = MAIN_TARGET.file_end
LOAD_FILE_START = MAIN_TARGET.file_start
LOAD_VRAM = MAIN_TARGET.vram
# `bin` is a raw byte span (splat `bin` subsegment, reassembled with .incbin);
# overlays use it for their container header and data tail.
ALLOWED_KINDS = {"asm", "c", "rodata", "bin"}
ALLOWED_TOOLCHAINS = {"era", "modern"}
SUBSEGMENT_RE = re.compile(
    r"^\s*- \[(0x[0-9A-Fa-f]+)"
    r"(?:,\s*([^,\]]+)(?:,\s*([^,\]]+))?)?\]\s*(?:#.*)?$"
)
SHA1_RE = re.compile(r"^sha1:\s*([0-9a-fA-F]{40})\s*$", re.MULTILINE)
FUNC_RE = re.compile(r"^func_([0-9A-Fa-f]{8})$")
ENV_RE = re.compile(r"^[A-Z][A-Z0-9_]*$")
GENERATED_ASM_UNIT_RE = re.compile(r"^[0-9A-F]+\.s$")
ASM_OFFSET_RE = re.compile(r"/\*\s*([0-9A-Fa-f]+)\s+")


class PlanError(RuntimeError):
    """A configuration invariant failed."""


@dataclass(frozen=True)
class Unit:
    index: int
    kind: str
    start: int
    end: int
    size: int
    vram: int
    name: str | None
    source: str
    object: str
    primary_section: str
    profile: str | None = None
    toolchain: str | None = None
    flags: tuple[str, ...] = ()
    environment: tuple[tuple[str, str], ...] = ()

    def serializable(self) -> dict[str, Any]:
        out = asdict(self)
        out["start_hex"] = f"0x{self.start:X}"
        out["end_hex"] = f"0x{self.end:X}"
        out["size_hex"] = f"0x{self.size:X}"
        out["vram_hex"] = f"0x{self.vram:08X}"
        out["flags"] = list(self.flags)
        out["environment"] = dict(self.environment)
        return out


def _repo_path(root: Path, path: Path | str) -> Path:
    candidate = Path(path)
    return candidate if candidate.is_absolute() else root / candidate


def _parse_rows(
    config_path: Path,
    file_start: int = LOAD_FILE_START,
    file_end: int = EXPECTED_FILE_END,
) -> tuple[str, list[tuple[int, int, str, str]]]:
    text = config_path.read_text(encoding="utf-8")
    sha_match = SHA1_RE.search(text)
    if not sha_match:
        raise PlanError(f"{config_path}: missing top-level sha1")

    rows: list[tuple[int, int, str, str]] = []
    for line_number, line in enumerate(text.splitlines(), 1):
        match = SUBSEGMENT_RE.match(line)
        if not match:
            continue
        offset = int(match.group(1), 16)
        kind = (match.group(2) or "").strip()
        name = (match.group(3) or "").strip()
        rows.append((line_number, offset, kind, name))

    if not rows:
        raise PlanError(f"{config_path}: no bracket-form subsegments found")
    if rows[-1][2] or rows[-1][1] != file_end:
        raise PlanError(
            f"{config_path}:{rows[-1][0]}: final row must be [0x{file_end:X}]"
        )
    for line_number, offset, kind, name in rows[:-1]:
        if kind not in ALLOWED_KINDS:
            raise PlanError(
                f"{config_path}:{line_number}: unsupported subsegment kind {kind!r}"
            )
        if kind == "c" and not name:
            raise PlanError(f"{config_path}:{line_number}: C span has no symbol")
        if kind != "c" and name:
            raise PlanError(
                f"{config_path}:{line_number}: {kind} span unexpectedly names {name}"
            )
        if offset < file_start:
            raise PlanError(
                f"{config_path}:{line_number}: loadable span starts before 0x{file_start:X}"
            )
    return sha_match.group(1).lower(), rows


def _unit_paths(
    kind: str, start: int, name: str, target: Target = MAIN_TARGET
) -> tuple[str, str, str]:
    stem = f"{start:X}"
    if kind == "c":
        return (
            f"{target.src_dir}/{name}.c",
            f"build/{target.src_dir}/{name}.c.o",
            ".text",
        )
    if kind == "asm":
        return (
            f"{target.asm_dir}/{stem}.s",
            f"build/{target.asm_dir}/{stem}.s.o",
            ".text",
        )
    if kind == "bin":
        # splat writes a `bin` subsegment to <asset_path>/<start hex>.bin; the
        # build wraps it in a .incbin unit so the bytes round-trip verbatim.
        return (
            f"{target.asset_dir}/{stem}.bin",
            f"build/{target.asm_dir}/{stem}.bin.o",
            ".data",
        )
    return (
        f"{target.asm_dir}/data/{stem}.rodata.s",
        f"build/{target.asm_dir}/data/{stem}.rodata.s.o",
        ".rodata",
    )


def _read_profile_manifest(path: Path) -> dict[str, Any]:
    try:
        manifest = json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError as exc:
        raise PlanError(f"missing build-profile manifest: {path}") from exc
    except json.JSONDecodeError as exc:
        raise PlanError(f"{path}:{exc.lineno}: invalid JSON: {exc.msg}") from exc
    if manifest.get("schema_version") != 1:
        raise PlanError(f"{path}: schema_version must be 1")
    return manifest


def _load_profiles(
    path: Path, c_names: set[str], root: Path = REPO_ROOT
) -> tuple[dict[str, Any], dict[str, str]]:
    manifest = _read_profile_manifest(path)
    # An overlay manifest may inherit the profile DEFINITIONS from the EXE
    # manifest (`profiles_from`) so a compiler-era fix lands once; it always
    # owns its own `assignments`.
    inherited_from = manifest.get("profiles_from")
    if inherited_from is not None:
        if "profiles" in manifest:
            raise PlanError(f"{path}: use either `profiles` or `profiles_from`, not both")
        base_path = _repo_path(root, inherited_from)
        if base_path.resolve() == path.resolve():
            raise PlanError(f"{path}: profiles_from refers to itself")
        base = _read_profile_manifest(base_path)
        profiles = base.get("profiles")
        default = manifest.get("default_profile", base.get("default_profile"))
        # Overlay-only: target-specific profiles (e.g. a MASPSX_DISPATCH_FOLD
        # naming this overlay's own jtbl_ pool) are added next to the
        # inherited ones; they may never shadow an inherited name.
        local = manifest.get("local_profiles")
        if local is not None:
            if not isinstance(local, dict) or not isinstance(profiles, dict):
                raise PlanError(f"{path}: local_profiles must be an object")
            clash = sorted(set(local) & set(profiles))
            if clash:
                raise PlanError(f"{path}: local_profiles shadow inherited {clash}")
            profiles = {**profiles, **local}
    else:
        profiles = manifest.get("profiles")
        default = manifest.get("default_profile")
    assignments = manifest.get("assignments")
    if not isinstance(profiles, dict) or not profiles:
        raise PlanError(f"{path}: profiles must be a non-empty object")
    if default not in profiles:
        raise PlanError(f"{path}: default_profile {default!r} is not defined")
    if not isinstance(assignments, dict):
        raise PlanError(f"{path}: assignments must be an object")

    for profile_name, profile in profiles.items():
        if not isinstance(profile, dict):
            raise PlanError(f"{path}: profile {profile_name!r} must be an object")
        if profile.get("toolchain") not in ALLOWED_TOOLCHAINS:
            raise PlanError(
                f"{path}: profile {profile_name!r} has invalid toolchain"
            )
        flags = profile.get("flags", [])
        environment = profile.get("environment", {})
        if not isinstance(flags, list) or not all(isinstance(x, str) for x in flags):
            raise PlanError(f"{path}: profile {profile_name!r} flags must be strings")
        if not isinstance(environment, dict) or not all(
            isinstance(k, str)
            and ENV_RE.fullmatch(k)
            and isinstance(v, str)
            for k, v in environment.items()
        ):
            raise PlanError(
                f"{path}: profile {profile_name!r} environment must be NAME:string"
            )

    resolved = {name: default for name in c_names}
    owners: dict[str, str] = {}
    for profile_name, symbols in assignments.items():
        if profile_name not in profiles:
            raise PlanError(f"{path}: assignment uses unknown profile {profile_name!r}")
        if profile_name == default:
            raise PlanError(
                f"{path}: default-profile symbols must be implicit, not duplicated"
            )
        if not isinstance(symbols, list) or not symbols:
            raise PlanError(
                f"{path}: assignment {profile_name!r} must be a non-empty list"
            )
        for symbol in symbols:
            if not isinstance(symbol, str):
                raise PlanError(f"{path}: non-string symbol in {profile_name!r}")
            if symbol not in c_names:
                raise PlanError(
                    f"{path}: stale build override {symbol!r}; no such YAML C span"
                )
            if symbol in owners:
                raise PlanError(
                    f"{path}: {symbol} assigned by both {owners[symbol]} and {profile_name}"
                )
            owners[symbol] = profile_name
            resolved[symbol] = profile_name

    return profiles, resolved


def build_plan(
    root: Path = REPO_ROOT,
    config: Path | None = None,
    profiles_path: Path | None = None,
    require_generated: bool = False,
    target: Target = MAIN_TARGET,
) -> dict[str, Any]:
    root = root.resolve()
    config = Path(target.config) if config is None else config
    profiles_path = Path(target.profiles) if profiles_path is None else profiles_path
    config_path = _repo_path(root, config)
    profile_path = _repo_path(root, profiles_path)
    expected_sha1, rows = _parse_rows(config_path, target.file_start, target.file_end)
    if target.expected_sha1 is not None and expected_sha1 != target.expected_sha1:
        raise PlanError(
            f"{config_path}: sha1 {expected_sha1} != manifest sha1 "
            f"{target.expected_sha1} for target {target.id}"
        )

    typed = rows[:-1]
    for current, following in zip(rows, rows[1:]):
        line_number, start, kind, name = current
        next_line, end, _, _ = following
        if end <= start:
            raise PlanError(
                f"{config_path}:{next_line}: non-increasing edge 0x{start:X} "
                f"({kind}{' '+name if name else ''}) -> 0x{end:X}"
            )

    c_names = [name for _, _, kind, name in typed if kind == "c"]
    duplicates = [name for name, count in Counter(c_names).items() if count != 1]
    if duplicates:
        raise PlanError(f"{config_path}: duplicate C symbols: {', '.join(duplicates)}")
    profiles, resolved = _load_profiles(profile_path, set(c_names), root)

    units: list[Unit] = []
    for index, ((_, start, kind, name), (_, end, _, _)) in enumerate(
        zip(rows, rows[1:])
    ):
        source, obj, section = _unit_paths(kind, start, name, target)
        vram = target.vram + start - target.file_start
        profile_name = None
        toolchain = None
        flags: tuple[str, ...] = ()
        environment: tuple[tuple[str, str], ...] = ()
        if kind == "c":
            func_match = FUNC_RE.fullmatch(name)
            if not func_match:
                raise PlanError(f"{config_path}: invalid C symbol {name!r}")
            named_vram = int(func_match.group(1), 16)
            if named_vram != vram:
                raise PlanError(
                    f"{config_path}: {name} names 0x{named_vram:08X}, "
                    f"but its YAML edge maps to 0x{vram:08X}"
                )
            profile_name = resolved[name]
            profile = profiles[profile_name]
            toolchain = profile["toolchain"]
            flags = tuple(profile.get("flags", []))
            environment = tuple(sorted(profile.get("environment", {}).items()))
        units.append(
            Unit(
                index=index,
                kind=kind,
                start=start,
                end=end,
                size=end - start,
                vram=vram,
                name=name or None,
                source=source,
                object=obj,
                primary_section=section,
                profile=profile_name,
                toolchain=toolchain,
                flags=flags,
                environment=environment,
            )
        )

    if units[0].start != target.file_start or units[-1].end != target.file_end:
        raise PlanError("YAML spans do not close over the complete loadable image")
    if sum(unit.size for unit in units) != target.load_size:
        raise PlanError("YAML span sizes do not sum to the loadable-image size")

    split_script = "scripts/split_us.sh" if target.is_main else f"scripts/split_overlay.sh {target.id}"
    header_source = target.header_source
    required_sources = [
        *([header_source] if header_source else []),
        *(unit.source for unit in units),
    ]
    missing_c = [unit.source for unit in units if unit.kind == "c" and not (root / unit.source).is_file()]
    if missing_c:
        raise PlanError("missing tracked C source(s): " + ", ".join(missing_c))
    if require_generated:
        missing = [source for source in required_sources if not (root / source).is_file()]
        if missing:
            raise PlanError(
                f"missing split-generated source(s); run {split_script}: "
                + ", ".join(missing[:12])
                + (f" (+{len(missing)-12} more)" if len(missing) > 12 else "")
            )
        stale = []
        for unit in units:
            if unit.kind != "c":
                continue
            asm_at_c_edge = root / f"{target.asm_dir}/{unit.start:X}.s"
            if asm_at_c_edge.exists():
                stale.append(str(asm_at_c_edge.relative_to(root)))
        if stale:
            raise PlanError(
                "stale generated asm overlaps YAML C span(s): " + ", ".join(stale)
            )

    profile_counts = Counter(unit.profile for unit in units if unit.kind == "c")
    kind_counts = Counter(unit.kind for unit in units)
    plan = {
        "schema_version": 1,
        "authority": str(config),
        "build_profiles": str(profiles_path),
        "expected_sha1": expected_sha1,
        "file": {
            "header_size": target.file_start,
            "load_start": target.file_start,
            "load_end": target.file_end,
            "load_size": target.load_size,
            "load_vram": target.vram,
        },
        "counts": {
            "units": len(units),
            "c": kind_counts["c"],
            "asm": kind_counts["asm"],
            "rodata": kind_counts["rodata"],
        },
        "profile_counts": dict(sorted(profile_counts.items())),
        "header": (
            {
                "source": header_source,
                "object": f"build/{header_source}.o",
                "section": ".data",
                "size": target.file_start,
            }
            if header_source
            else None
        ),
        "units": [unit.serializable() for unit in units],
    }
    if kind_counts["bin"]:
        # Only overlay plans carry raw spans; the EXE plan's key set (and so
        # its hash) is untouched when the count is zero.
        plan["counts"]["bin"] = kind_counts["bin"]
    canonical = json.dumps(plan, sort_keys=True, separators=(",", ":")).encode()
    plan["plan_sha256"] = hashlib.sha256(canonical).hexdigest()
    # The target descriptor comes from the CLI / manifest, not from the YAML
    # authority, so it is attached AFTER hashing: the plan hash of a given
    # YAML is the same as before targets existed.
    plan["target"] = target.serializable()
    return plan


def plan_target(plan: dict[str, Any]) -> Target:
    """Recover the descriptor a plan was built for (MAIN_TARGET if absent)."""
    data = plan.get("target")
    if not data:
        return MAIN_TARGET
    return Target(
        **{
            key: value
            for key, value in data.items()
            if key not in ("vram_hex",)
        }
        | {"derive_symbol_prefixes": tuple(data.get("derive_symbol_prefixes", ("D_",)))}
    )


def remove_stale_generated_asm_units(root: Path, plan: dict[str, Any]) -> list[Path]:
    """Delete ignored splat asm units superseded by the current YAML plan.

    Splat names Disc 1 text units after their uppercase hexadecimal file offset.
    A YAML carve can therefore leave an older unit behind when its old start is
    now C, or (for a retained start) leave old instructions beyond a new C edge.
    Only such generated names are considered, and every deletion is guarded by
    ``git check-ignore`` so tracked or user-owned paths are never touched.
    """

    asm_dir = root / plan_target(plan).asm_dir
    if not asm_dir.is_dir():
        return []

    expected_sources = {
        unit["source"] for unit in plan["units"] if unit["kind"] == "asm"
    }
    c_ranges = [
        (unit["start"], unit["end"])
        for unit in plan["units"]
        if unit["kind"] == "c"
    ]
    removed: list[Path] = []

    for path in sorted(asm_dir.glob("*.s")):
        if not GENERATED_ASM_UNIT_RE.fullmatch(path.name):
            continue
        relative = path.relative_to(root)
        offsets = [int(match.group(1), 16) for match in ASM_OFFSET_RE.finditer(path.read_text(encoding="utf-8"))]
        overlaps_c = any(
            start <= offset < end for offset in offsets for start, end in c_ranges
        )
        if str(relative) in expected_sources and not overlaps_c:
            continue

        ignored = subprocess.run(
            ["git", "-C", str(root), "check-ignore", "-q", "--", str(relative)],
            check=False,
        ).returncode == 0
        if not ignored:
            raise PlanError(
                f"refusing to delete non-ignored asm unit: {relative}"
            )
        path.unlink()
        removed.append(relative)

    return removed


def render_linker_script(plan: dict[str, Any]) -> str:
    units = plan["units"]
    geometry = plan["file"]
    lines = [
        f"/* Generated by tools/build/disc1_plan.py from {plan['authority']}.",
        " * Do not edit: YAML owns order and span geometry. */",
        "SECTIONS",
        "{",
    ]
    if plan.get("header"):
        lines += [
            "    .header : AT(0)",
            "    {",
            f"        {plan['header']['object']}({plan['header']['section']})",
            "    }",
            "",
        ]
    lines += [
        f"    .main 0x{geometry['load_vram']:08X} : AT(0x{geometry['load_start']:X})",
        "    {",
    ]
    for unit in units:
        lines.append(f"        {unit['object']}({unit['primary_section']})")
    # Secondary sections are deliberately generated for every unit. Missing
    # input sections match nothing; present ones retain deterministic YAML
    # order. This replaces the old hand-pruned lists.
    for section in (".data", ".rodata", ".bss"):
        for unit in units:
            lines.append(f"        {unit['object']}({section})")
    lines.extend(["", "    }", "", "    /DISCARD/ : { *(*) }", "}", ""])
    return "\n".join(lines)


def verification_manifest(plan: dict[str, Any]) -> dict[str, Any]:
    spans = [
        {
            "name": unit["name"],
            "start": unit["start"],
            "end": unit["end"],
            "size": unit["size"],
            "source": unit["source"],
            "object": unit["object"],
        }
        for unit in plan["units"]
        if unit["kind"] == "c"
    ]
    return {
        "schema_version": 1,
        "generated_from": plan["authority"],
        "plan_sha256": plan["plan_sha256"],
        "expected_sha1": plan["expected_sha1"],
        "matching_c_count": len(spans),
        "spans": spans,
    }


def matching_status(plan: dict[str, Any]) -> str:
    count = plan["counts"]["c"]
    return f"""# Disc 1 matching status

<!-- Generated by tools/build/disc1_plan.py. Do not edit by hand. -->

- Exact matching-C spans: **{count}**
- Span authority: [`configs/USA/disc1.yaml`](../../configs/USA/disc1.yaml)
- Retail SHA-1: `{plan['expected_sha1']}`
- Count command: `python3 tools/build/disc1_plan.py --check`

Accepted residuals are evidence dispositions, not YAML `c` spans, and are
tracked separately in [`MATCHING_RESIDUAL_POLICY.md`](../acceptance/MATCHING_RESIDUAL_POLICY.md).
"""


def write_generated(plan: dict[str, Any], output_dir: Path) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    (output_dir / "disc1_plan.json").write_text(
        json.dumps(plan, indent=2) + "\n", encoding="utf-8"
    )
    (output_dir / "disc1_verify_manifest.json").write_text(
        json.dumps(verification_manifest(plan), indent=2) + "\n", encoding="utf-8"
    )
    (output_dir / "disc1_romorder.ld").write_text(
        render_linker_script(plan), encoding="utf-8"
    )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=REPO_ROOT)
    parser.add_argument(
        "--target",
        default=None,
        metavar="ID",
        help="rebuild target: disc1 (default, the EXE) or an overlay id from "
        "configs/USA/overlays/manifest.yaml",
    )
    parser.add_argument("--config", type=Path, default=None,
                        help="YAML authority (default: the target's config)")
    parser.add_argument("--profiles", type=Path, default=None,
                        help="build-profile JSON (default: the target's)")
    parser.add_argument("--emit-dir", type=Path)
    parser.add_argument("--require-generated", action="store_true")
    parser.add_argument("--cleanup-stale-asm", action="store_true")
    parser.add_argument("--write-status", action="store_true")
    parser.add_argument("--check-status", action="store_true")
    parser.add_argument("--status", type=Path, default=DEFAULT_STATUS)
    parser.add_argument("--check", action="store_true", help="validate and print summary")
    args = parser.parse_args(argv)

    try:
        target = load_target(args.target, root=args.root.resolve())
        if not target.is_main and (args.write_status or args.check_status):
            raise PlanError(
                "the generated matching status is EXE-only; overlay counts are "
                "reported by --check per target"
            )
        plan = build_plan(
            root=args.root,
            config=args.config,
            profiles_path=args.profiles,
            require_generated=args.require_generated,
            target=target,
        )
        if args.cleanup_stale_asm:
            removed = remove_stale_generated_asm_units(args.root.resolve(), plan)
            for path in removed:
                print(f"removed stale generated asm: {path}")
        if args.emit_dir:
            output = _repo_path(args.root.resolve(), args.emit_dir)
            write_generated(plan, output)
        status_path = _repo_path(args.root.resolve(), args.status)
        rendered_status = matching_status(plan)
        if args.write_status:
            status_path.parent.mkdir(parents=True, exist_ok=True)
            status_path.write_text(rendered_status, encoding="utf-8")
        if args.check_status:
            try:
                actual_status = status_path.read_text(encoding="utf-8")
            except FileNotFoundError as exc:
                raise PlanError(f"missing generated status: {status_path}") from exc
            if actual_status != rendered_status:
                raise PlanError(
                    f"stale generated status: {status_path}; run "
                    "python3 tools/build/disc1_plan.py --write-status"
                )
    except (PlanError, TargetError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1

    print(
        f"{target.id} plan: "
        f"{plan['counts']['units']} spans "
        f"({plan['counts']['c']} c, {plan['counts']['asm']} asm, "
        f"{plan['counts']['rodata']} rodata"
        + (f", {plan['counts']['bin']} bin" if plan["counts"].get("bin") else "")
        + "), "
        f"geometry=0x{plan['file']['load_size']:X}, "
        f"plan={plan['plan_sha256'][:12]}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
