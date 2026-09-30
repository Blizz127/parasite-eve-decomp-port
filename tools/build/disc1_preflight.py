#!/usr/bin/env python3
"""Fast preflight validation for the Disc 1 exact-rebuild inputs.

Both historical live-tree blockers were cheaply detectable in seconds but only
surfaced after a ~30 minute split/build cycle:

  * a hard-wrapped comment continuation made ``configs/USA/disc1.yaml``
    syntactically invalid (splat aborts at split time), and
  * a ``c`` span declared 0x94 bytes while the era toolchain emitted 0x90
    (``trim_elf_section_pad.py`` aborts at build time, after the split).

This validator runs on the same inputs and fails loudly *before* the expensive
stages, printing the offending span/file/line and the exact remedy.

Checks (fast, default):
  1. ``yaml-syntax``   — parse the YAML exactly as the real consumer
                        (splat uses ``yaml.load(..., Loader=yaml.SafeLoader)``).
  2. ``yaml-sha1``     — top-level ``sha1:`` present, 40 hex.
  3. ``geometry``      — spans start at 0x800, close at 0x1EE800, strictly
                        ascending with no duplicate/gap/overlap, and every ``c``
                        symbol's name encodes its own mapped VRAM.
  4. ``c-source``      — every ``c`` span has its ``src/<name>.c``.
  5. ``profile-ref``   — build-profile JSON valid; every assignment names an
                        existing profile and an existing ``c`` span; no span
                        assigned twice; default profile defined.

Deep check (``--deep``, parallel, ~6s for ~500 leaves):
  6. compiles every ``c`` leaf with its exact per-leaf profile and runs the
     **strong** leaf check (``tools/analysis/leaf_strong_check.py``): the span
     must not exceed the compiled ``.text`` (the 0x94-vs-0x90 defect class), any
     bytes past a smaller span must be zero (the gas 16-byte alignment pad), the
     object linked at its retail VMA must match retail word-for-word over the
     whole span, the span must end on a bare ``jr $ra`` (or a tail ``j`` /
     indirect ``jr $reg``), and
     it must contain **no interior ``jr $ra``** (a swallowed adjacent function
     would leave one).  The weak per-leaf helper ``era_link_check.py`` used by
     ``check_leaf.sh`` does NOT catch an oversized span; this does.

Exit codes: 0 PASS, 1 FAIL.  This tool never weakens the gate: it can only add
a FAIL before the gate's own checks run.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import struct
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass
from pathlib import Path
from typing import Any

_THIS_DIR = Path(__file__).resolve().parent
REPO_ROOT = _THIS_DIR.parents[1]
if str(_THIS_DIR) not in sys.path:
    sys.path.insert(0, str(_THIS_DIR))

from disc1_plan import (  # noqa: E402
    ALLOWED_KINDS,
    EXPECTED_FILE_END,
    FUNC_RE,
    LOAD_FILE_START,
    SHA1_RE,
    SUBSEGMENT_RE,
    PlanError,
    build_plan,
)
from overlay_targets import MAIN_TARGET, Target, TargetError, load_target  # noqa: E402

DEFAULT_CONFIG = Path(MAIN_TARGET.config)
DEFAULT_PROFILES = Path(MAIN_TARGET.profiles)
ERA_CPP = Path("tools/era/gcc-2.7.2-psx/cpp")
ERA_CC1 = Path("tools/era/gcc-2.7.2-psx/cc1")
MASPSX = Path("tools/era/maspsx/maspsx.py")


@dataclass(frozen=True)
class Finding:
    check: str
    message: str
    remedy: str


def _repo_path(root: Path, path: Path) -> Path:
    return path if path.is_absolute() else root / path


def _source_line(text: str, line_number: int) -> str:
    lines = text.splitlines()
    if 1 <= line_number <= len(lines):
        return lines[line_number - 1]
    return ""


# ---------------------------------------------------------------------------
# 1. YAML syntax (same loader as splat)
# ---------------------------------------------------------------------------
def check_yaml_syntax(config: Path) -> list[Finding]:
    try:
        import yaml  # noqa: PLC0415
    except ImportError:
        print(
            "disc1_preflight: WARN PyYAML unavailable; skipping yaml-syntax "
            "(structural checks still run)",
            file=sys.stderr,
        )
        return []

    text = config.read_text(encoding="utf-8")
    try:
        yaml.load(text, Loader=yaml.SafeLoader)
    except yaml.MarkedYAMLError as exc:
        mark = getattr(exc, "problem_mark", None) or getattr(exc, "context_mark", None)
        line = (mark.line + 1) if mark is not None else 0
        column = (mark.column + 1) if mark is not None else 0
        problem = getattr(exc, "problem", None) or str(exc)
        context = getattr(exc, "context", None)
        where = f"{config}:{line}:{column}" if line else str(config)
        detail = f"{context}: {problem}" if context else problem
        snippet = _source_line(text, line)
        message = f"{where}: {detail}"
        if snippet:
            message += f"\n    {snippet}\n    {' ' * max(column - 1, 0)}^"
        return [
            Finding(
                "yaml-syntax",
                message,
                "fix the YAML (splat parses it with yaml.SafeLoader); check for a "
                "hard-wrapped comment continuation at the flagged line",
            )
        ]
    except yaml.YAMLError as exc:  # pragma: no cover - non-marked error
        return [
            Finding(
                "yaml-syntax",
                f"{config}: {exc}",
                "fix the YAML (splat parses it with yaml.SafeLoader)",
            )
        ]
    return []


# ---------------------------------------------------------------------------
# 2/3. Structural span geometry (line numbers for actionable output)
# ---------------------------------------------------------------------------
@dataclass(frozen=True)
class Row:
    line: int
    offset: int
    kind: str
    name: str


def parse_rows(config: Path) -> list[Row]:
    rows: list[Row] = []
    for line_number, line in enumerate(config.read_text(encoding="utf-8").splitlines(), 1):
        match = SUBSEGMENT_RE.match(line)
        if not match:
            continue
        rows.append(
            Row(
                line=line_number,
                offset=int(match.group(1), 16),
                kind=(match.group(2) or "").strip(),
                name=(match.group(3) or "").strip(),
            )
        )
    return rows


def check_geometry(
    config: Path, rows: list[Row], target: Target = MAIN_TARGET
) -> list[Finding]:
    findings: list[Finding] = []
    file_start, file_end, load_vram = target.file_start, target.file_end, target.vram
    if not rows:
        return [
            Finding(
                "geometry",
                f"{config}: no bracket-form subsegments found",
                "the YAML must be a splat vram/segments list of `- [0xADDR, kind, name]` rows",
            )
        ]

    seen: dict[int, int] = {}
    for row in rows:
        if row.offset in seen:
            findings.append(
                Finding(
                    "geometry",
                    f"{config}:{row.line}: duplicate VMA 0x{row.offset:X} "
                    f"(first declared on line {seen[row.offset]})",
                    "remove the duplicate row or renumber it to its true start",
                )
            )
        else:
            seen[row.offset] = row.line

    if rows[0].offset != file_start:
        findings.append(
            Finding(
                "geometry",
                f"{config}:{rows[0].line}: first span starts 0x{rows[0].offset:X}, "
                f"expected 0x{file_start:X}",
                f"the loadable image must begin at 0x{file_start:X}",
            )
        )

    if rows[-1].offset != file_end or rows[-1].kind or rows[-1].name:
        findings.append(
            Finding(
                "geometry",
                f"{config}:{rows[-1].line}: final row must be "
                f"`- [0x{file_end:X}]`, got 0x{rows[-1].offset:X}"
                f"{' '+rows[-1].kind if rows[-1].kind else ''}",
                f"terminate the segment list with `- [0x{file_end:X}]`",
            )
        )

    for current, following in zip(rows, rows[1:]):
        if following.offset <= current.offset:
            findings.append(
                Finding(
                    "geometry",
                    f"{config}:{following.line}: offset 0x{following.offset:X} does not "
                    f"ascend past line {current.line} (0x{current.offset:X}) "
                    f"-> {kind_desc(current)} overlaps/duplicates the previous span",
                    "span offsets must increase strictly; fix the overlapping edge",
                )
            )

    for row in rows[:-1]:
        if row.kind not in ALLOWED_KINDS:
            findings.append(
                Finding(
                    "geometry",
                    f"{config}:{row.line}: unsupported span kind {row.kind!r}",
                    f"use one of {sorted(ALLOWED_KINDS)}",
                )
            )
        elif row.kind == "c" and not row.name:
            findings.append(
                Finding(
                    "geometry",
                    f"{config}:{row.line}: `c` span has no symbol",
                    "name it `- [0xADDR, c, func_XXXXXXXX]`",
                )
            )
        elif row.kind != "c" and row.name:
            findings.append(
                Finding(
                    "geometry",
                    f"{config}:{row.line}: `{row.kind}` span unexpectedly names {row.name}",
                    "only `c` spans carry a symbol",
                )
            )

    for row, following in zip(rows[:-1], rows[1:]):
        if row.kind != "c" or not row.name:
            continue
        vram = load_vram + row.offset - file_start
        match = FUNC_RE.fullmatch(row.name)
        if not match:
            findings.append(
                Finding(
                    "geometry",
                    f"{config}:{row.line}: invalid C symbol {row.name!r}",
                    "C symbols must be `func_XXXXXXXX`",
                )
            )
            continue
        named = int(match.group(1), 16)
        if named != vram:
            findings.append(
                Finding(
                    "geometry",
                    f"{config}:{row.line}: {row.name} names 0x{named:08X} but its span "
                    f"edge 0x{row.offset:X} maps to 0x{vram:08X}",
                    f"rename to func_{vram:08X} or move the span edge",
                )
            )
        if following.offset - row.offset <= 0:
            continue

    return findings


def kind_desc(row: Row) -> str:
    return f"{row.kind}{' '+row.name if row.name else ''}"


def check_yaml_sha1(config: Path) -> list[Finding]:
    text = config.read_text(encoding="utf-8")
    if not SHA1_RE.search(text):
        return [
            Finding(
                "yaml-sha1",
                f"{config}: missing top-level `sha1: <40 hex>`",
                "add the retail executable SHA-1 that the rebuild must reproduce",
            )
        ]
    return []


# ---------------------------------------------------------------------------
# 4. C sources
# ---------------------------------------------------------------------------
def check_c_sources(
    config: Path, root: Path, rows: list[Row], src_dir: str = MAIN_TARGET.src_dir
) -> list[Finding]:
    findings: list[Finding] = []
    for row in rows[:-1]:
        if row.kind != "c" or not row.name:
            continue
        source = root / src_dir / f"{row.name}.c"
        if not source.is_file():
            findings.append(
                Finding(
                    "c-source",
                    f"{config}:{row.line}: `c` span {row.name} has no {source.relative_to(root)}",
                    f"create {source.relative_to(root)} (or drop/convert the span); "
                    "its object is a required link input",
                )
            )
    return findings


# ---------------------------------------------------------------------------
# 5. Build profiles
# ---------------------------------------------------------------------------
def check_profiles(
    profiles_path: Path, rows: list[Row], root: Path | None = None
) -> list[Finding]:
    findings: list[Finding] = []
    try:
        manifest = json.loads(profiles_path.read_text(encoding="utf-8"))
    except FileNotFoundError:
        return [
            Finding(
                "profile-ref",
                f"missing build-profile manifest: {profiles_path}",
                "restore configs/USA/disc1_build_profiles.json",
            )
        ]
    except json.JSONDecodeError as exc:
        return [
            Finding(
                "profile-ref",
                f"{profiles_path}:{exc.lineno}: invalid JSON: {exc.msg}",
                "fix the JSON syntax",
            )
        ]

    profiles = manifest.get("profiles")
    default = manifest.get("default_profile")
    inherited_from = manifest.get("profiles_from")
    if inherited_from is not None:
        # Overlay manifests inherit the profile definitions from the EXE
        # manifest (disc1_plan._load_profiles does the same).
        base_path = _repo_path(root or REPO_ROOT, Path(inherited_from))
        try:
            base = json.loads(base_path.read_text(encoding="utf-8"))
        except (FileNotFoundError, json.JSONDecodeError) as exc:
            return [
                Finding(
                    "profile-ref",
                    f"{profiles_path}: profiles_from {inherited_from} unreadable: {exc}",
                    "point profiles_from at a valid build-profile manifest",
                )
            ]
        if profiles is not None:
            findings.append(
                Finding(
                    "profile-ref",
                    f"{profiles_path}: has both `profiles` and `profiles_from`",
                    "keep only one of them",
                )
            )
        profiles = base.get("profiles")
        if default is None:
            default = base.get("default_profile")
        local = manifest.get("local_profiles")
        if local is not None and not isinstance(local, dict):
            findings.append(
                Finding(
                    "profile-ref",
                    f"{profiles_path}: local_profiles must be an object",
                    "make local_profiles a {name: profile} object or remove it",
                )
            )
        elif isinstance(local, dict) and isinstance(profiles, dict):
            clash = sorted(set(local) & set(profiles))
            if clash:
                findings.append(
                    Finding(
                        "profile-ref",
                        f"{profiles_path}: local_profiles shadow inherited {clash}",
                        "rename the local profile",
                    )
                )
            profiles = {**profiles, **local}
    assignments = manifest.get("assignments")
    if not isinstance(profiles, dict) or not profiles:
        return [
            Finding(
                "profile-ref",
                f"{profiles_path}: `profiles` must be a non-empty object",
                "define at least the default profile",
            )
        ]
    if default not in profiles:
        findings.append(
            Finding(
                "profile-ref",
                f"{profiles_path}: default_profile {default!r} is not defined",
                "add it to `profiles` or fix the name",
            )
        )
    if not isinstance(assignments, dict):
        return findings + [
            Finding(
                "profile-ref",
                f"{profiles_path}: `assignments` must be an object",
                "map profile name -> list of YAML C symbols",
            )
        ]

    c_names = {row.name for row in rows[:-1] if row.kind == "c" and row.name}
    owners: dict[str, str] = {}
    for profile_name, symbols in assignments.items():
        if profile_name not in profiles:
            findings.append(
                Finding(
                    "profile-ref",
                    f"{profiles_path}: assignment uses unknown profile {profile_name!r}",
                    f"define {profile_name!r} in `profiles` or fix the reference",
                )
            )
        elif profile_name == default:
            findings.append(
                Finding(
                    "profile-ref",
                    f"{profiles_path}: assignment duplicates the default profile "
                    f"{profile_name!r}",
                    "default-profile symbols are implicit; remove this assignment block",
                )
            )
        if not isinstance(symbols, list) or not symbols:
            findings.append(
                Finding(
                    "profile-ref",
                    f"{profiles_path}: assignment {profile_name!r} must be a non-empty list",
                    "list the YAML C symbols assigned to this profile",
                )
            )
            continue
        for symbol in symbols:
            if not isinstance(symbol, str):
                findings.append(
                    Finding(
                        "profile-ref",
                        f"{profiles_path}: non-string symbol in {profile_name!r}",
                        "symbols must be strings",
                    )
                )
                continue
            if symbol not in c_names:
                findings.append(
                    Finding(
                        "profile-ref",
                        f"{profiles_path}: stale assignment {profile_name!r} -> {symbol!r}; "
                        "no such YAML C span",
                        "remove the assignment or add the missing YAML span",
                    )
                )
            if symbol in owners:
                findings.append(
                    Finding(
                        "profile-ref",
                        f"{profiles_path}: {symbol} assigned by both {owners[symbol]} "
                        f"and {profile_name}",
                        "assign each leaf to exactly one profile",
                    )
                )
            owners[symbol] = profile_name

    for name, profile in profiles.items():
        if not isinstance(profile, dict):
            findings.append(
                Finding(
                    "profile-ref",
                    f"{profiles_path}: profile {name!r} must be an object",
                    "use {toolchain, flags, environment}",
                )
            )
    return findings


# ---------------------------------------------------------------------------
# 6. Deep size check
# ---------------------------------------------------------------------------
def _elf_section(obj: Path, want: str) -> tuple[int, int, int] | None:
    """Return (sh_offset, sh_size, sh_addralign) for `want` in a 32-bit LE ELF."""
    data = obj.read_bytes()
    if data[:4] != b"\x7fELF" or data[4] != 1:
        return None
    shoff = struct.unpack_from("<I", data, 32)[0]
    shentsize = struct.unpack_from("<H", data, 46)[0]
    shnum = struct.unpack_from("<H", data, 48)[0]
    shstrndx = struct.unpack_from("<H", data, 50)[0]
    if shoff == 0 or shnum == 0:
        return None
    shstr_off = struct.unpack_from("<I", data, shoff + shstrndx * shentsize + 16)[0]
    for i in range(shnum):
        base = shoff + i * shentsize
        name_off = struct.unpack_from("<I", data, base)[0]
        start = shstr_off + name_off
        end = data.index(b"\x00", start)
        if data[start:end].decode("ascii", "replace") == want:
            return (
                struct.unpack_from("<I", data, base + 16)[0],
                struct.unpack_from("<I", data, base + 20)[0],
                struct.unpack_from("<I", data, base + 32)[0],
            )
    return None


@dataclass(frozen=True)
class DeepResult:
    name: str
    span: int
    obj_size: int | None
    nonzero_tail: bool
    error: str | None


def _run(command: list[str], **kwargs: Any) -> subprocess.CompletedProcess[Any]:
    return subprocess.run(command, check=True, **kwargs)


# Mirrors disc1_build.ERA_ASPSX_VER_DEFAULT / maspsx_sdata_flag (the fast path
# must not import the build module).
ERA_ASPSX_VER_DEFAULT = "2.30"


def _maspsx_sdata_flag(flags: list[str]) -> str:
    limit = "0"
    it = iter(flags)
    for flag in it:
        if flag == "-G":
            limit = next(it, "0")
        elif flag.startswith("-G") and flag[2:].isdigit():
            limit = flag[2:]
    return f"-G{limit}"


def _force_absolute_symbols(assembly: Path, specification: str) -> None:
    text = assembly.read_text(encoding="utf-8")
    for symbol in specification.split(","):
        text = re.sub(rf"\t\.extern\t{re.escape(symbol)}, \d+\n", "", text)
    assembly.write_text(text, encoding="utf-8")


def _compile_leaf(
    unit: dict[str, Any], root: Path, tools: Any, modern_flags: list[str]
) -> DeepResult:
    span = unit["size"]
    name = unit["name"]
    env = os.environ.copy()
    env.update(unit["environment"])
    try:
        with tempfile.TemporaryDirectory(prefix="pe-preflight-") as temporary:
            temp = Path(temporary)
            source = root / unit["source"]
            output = temp / "x.o"
            if unit["toolchain"] == "modern":
                _run(
                    [*tools.runner, tools.compiler, *modern_flags, *unit["flags"],
                     "-c", "-o", str(output), str(source)],
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL,
                )
            else:
                # Same per-leaf ERA_CC1_VER selection as disc1_build.compile_era,
                # or this pass would report false mismatches for 2.8.1 leaves.
                import disc1_build  # noqa: PLC0415

                era_cpp, era_cc1 = disc1_build.era_compiler(unit["environment"])
                preprocessed = temp / "x.i"
                assembly = temp / "x.s"
                expanded = temp / "xm.s"
                with preprocessed.open("wb") as stream:
                    _run(
                        [str(root / era_cpp), str(source)],
                        stdout=stream,
                        stderr=subprocess.DEVNULL,
                        env=env,
                    )
                _run(
                    [str(root / era_cc1), "-quiet", *unit["flags"],
                     str(preprocessed), "-o", str(assembly)],
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL,
                    env=env,
                )
                forced = unit["environment"].get("MASPSX_FORCE_ABSOLUTE_SYMBOLS")
                if forced:
                    _force_absolute_symbols(assembly, forced)
                command = [
                    sys.executable,
                    str(root / MASPSX),
                    f"--aspsx-version={unit['environment'].get('ERA_ASPSX_VER', ERA_ASPSX_VER_DEFAULT)}",
                    "--dont-expand-li",
                ]
                if unit["environment"].get("MASPSX_EXPAND_DIV") == "1":
                    command.append("--expand-div")
                if unit["environment"].get("MASPSX_USE_COMM_SECTION") == "1":
                    command.append("--use-comm-section")
                command.append(_maspsx_sdata_flag(list(unit["flags"])))
                command.append(str(assembly))
                with expanded.open("wb") as stream:
                    # See era_link_check.py: maspsx blocks on an inherited
                    # non-tty stdin instead of reading its file argument.
                    _run(command, stdin=subprocess.DEVNULL, stdout=stream,
                         stderr=subprocess.DEVNULL, env=env)
                _run(
                    [*tools.runner, tools.assembler, "-EL", "-mips1", "-mabi=32",
                     "-I", str(root / "include"), "-o", str(output), str(expanded)],
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL,
                    env=env,
                )
            section = _elf_section(output, unit["primary_section"])
            if section is None:
                return DeepResult(name, span, None, False, "no .text section in object")
            offset, size, _align = section
            data = output.read_bytes()
            tail = data[offset + span: offset + size] if size > span else b""
            return DeepResult(name, span, size, any(tail), None)
    except FileNotFoundError as exc:
        return DeepResult(name, span, None, False, f"tool not found: {exc.filename or exc}")
    except subprocess.CalledProcessError as exc:
        return DeepResult(name, span, None, False, f"compile failed: {exc}")
    except Exception as exc:  # pragma: no cover - defensive
        return DeepResult(name, span, None, False, f"{type(exc).__name__}: {exc}")


def parse_source_overrides(values: list[str]) -> dict[str, Path]:
    """``FUNC=PATH`` pairs -> {FUNC: absolute PATH}; fails loudly."""
    overrides: dict[str, Path] = {}
    for value in values:
        name, sep, path = value.partition("=")
        if not sep or not name or not path:
            raise SystemExit(f"--source-override expects FUNC=PATH, got {value!r}")
        resolved = Path(path).resolve()
        if not resolved.is_file():
            raise SystemExit(f"--source-override: {resolved} is not a file")
        overrides[name] = resolved
    return overrides


def check_deep(
    config: Path,
    profiles_path: Path,
    root: Path,
    jobs: int,
    only: set[str] | None,
    target: Target = MAIN_TARGET,
    source_overrides: dict[str, Path] | None = None,
) -> list[Finding]:
    # Import lazily so the fast path has no toolchain dependency.
    import disc1_build  # noqa: PLC0415

    findings: list[Finding] = []
    try:
        plan = build_plan(root=root, config=config, profiles_path=profiles_path, target=target)
    except PlanError as exc:
        return [
            Finding(
                "deep-size",
                f"cannot build plan for deep check: {exc}",
                "fix the structural defects above first",
            )
        ]

    units = [u for u in plan["units"] if u["kind"] == "c"]
    if only:
        units = [u for u in units if u["name"] in only]
        missing = only - {u["name"] for u in units}
        if missing:
            findings.append(
                Finding(
                    "deep-size",
                    f"--only names not in the plan: {', '.join(sorted(missing))}",
                    "pass YAML C symbols",
                )
            )

    if source_overrides:
        # --source-override: compile a draft in place of the YAML leaf's
        # src/<name>.c (absolute path, so ROOT / source resolves to it). Only
        # the C file changes; span, profile and retail bytes are the plan's.
        unknown = set(source_overrides) - {u["name"] for u in units}
        if unknown:
            findings.append(
                Finding(
                    "deep-size",
                    f"--source-override names not checked: {', '.join(sorted(unknown))}",
                    "pass YAML C symbols that are also selected by --only",
                )
            )
        units = [
            dict(u, source=str(source_overrides[u["name"]]))
            if u["name"] in source_overrides else u
            for u in units
        ]
        for name, path in sorted(source_overrides.items()):
            print(f"disc1_preflight: NOTE {name} compiled from override {path}",
                  file=sys.stderr)

    if not units:
        return findings

    # Deep size validation is best-effort: if the era/mipsel tooling is not
    # present the *build* step is the authority on prerequisites (it reports a
    # clean ENV/FAIL). Skipping here keeps this preflight from misclassifying a
    # missing-toolchain environment as a configuration failure.
    required_tools: list[Path] = [ERA_CPP, ERA_CC1, MASPSX]
    for unit in units:
        if unit["toolchain"] == "era":
            required_tools.extend(disc1_build.era_compiler(unit["environment"]))
    missing_tools = sorted({str(tool) for tool in required_tools if not (root / tool).is_file()})
    if missing_tools:
        print(
            "disc1_preflight: WARN deep size check skipped (missing era tool(s): "
            + ", ".join(missing_tools)
            + "); run scripts/setup_era.sh",
            file=sys.stderr,
        )
        return findings
    try:
        tools = disc1_build.find_toolchain()
    except Exception as exc:
        print(
            f"disc1_preflight: WARN deep size check skipped (no mipsel toolchain: {exc})",
            file=sys.stderr,
        )
        return findings

    results: list[DeepResult] = []
    with ThreadPoolExecutor(max_workers=max(1, jobs)) as executor:
        futures = [
            executor.submit(
                _compile_leaf, unit, root, tools, disc1_build.MODERN_C_FLAGS
            )
            for unit in units
        ]
        for future in futures:
            results.append(future.result())

    weak_by_name = {r.name: r for r in results}
    unit_by_name = {u["name"]: u for u in units}

    # Strong pass: compile the same leaves with the build's own profile-aware
    # compiler into a scratch dir and run the shared strong check (size + link
    # at the retail VMA + terminator + no interior jr $ra).  This is what makes
    # the deep preflight catch an oversized span, which the weak per-leaf
    # helper era_link_check.py cannot.
    strong_error: str | None = None
    try:
        retail_path = root / target.retail
        if not retail_path.is_file():
            remedy = "scripts/extract_us.sh 1" if target.is_main else "scripts/extract_overlays.sh"
            strong_error = f"missing retail {'EXE' if target.is_main else 'overlay'} {retail_path} (run {remedy})"
        else:
            import json as _json  # noqa: PLC0415
            import shutil as _shutil  # noqa: PLC0415
            import tempfile as _tempfile  # noqa: PLC0415

            _THIS = Path(__file__).resolve()
            analysis_dir = REPO_ROOT / "tools/analysis"
            if str(analysis_dir) not in sys.path:
                sys.path.insert(0, str(analysis_dir))
            import leaf_strong_check as strong  # noqa: PLC0415

            retail = retail_path.read_bytes()
            # Unique per process: a sibling might run its own
            # `check_leaf.sh` -> `disc1_preflight.py --deep` concurrently, and a
            # fixed scratch path would let one run rmtree the other's objects.
            (root / "build").mkdir(parents=True, exist_ok=True)
            scratch = Path(_tempfile.mkdtemp(prefix="preflight-strong-", dir=root / "build"))
            try:
                build_units = _json.loads(_json.dumps(units))
                for unit in build_units:
                    unit["object"] = str(scratch / Path(unit["object"]).name)
                # Build chatter goes to stderr so --json keeps stdout machine-clean.
                import contextlib  # noqa: PLC0415

                with contextlib.redirect_stdout(sys.stderr):
                    disc1_build.compile_all(
                        {"units": build_units, "header": plan["header"], "target": plan.get("target")},
                        tools,
                    )
                ld = tools.linker
                nm = strong.resolve_nm(root)
                strong_by_name: dict[str, Any] = {}
                with ThreadPoolExecutor(max_workers=max(1, jobs)) as executor:
                    futures = {
                        unit["name"]: executor.submit(
                            strong.check_object,
                            scratch / Path(build_unit["object"]).name,
                            unit, retail, ld, nm,
                        )
                        for unit, build_unit in zip(units, build_units)
                    }
                    for name, future in futures.items():
                        strong_by_name[name] = future.result()
            finally:
                _shutil.rmtree(scratch, ignore_errors=True)
    except FileNotFoundError as exc:
        strong_error = f"strong check tool missing: {exc}"
    except subprocess.CalledProcessError as exc:
        strong_error = f"strong check compile failed: {exc}"
    except Exception as exc:  # pragma: no cover - defensive
        strong_error = f"strong check unavailable: {type(exc).__name__}: {exc}"

    for result in results:
        unit = unit_by_name[result.name]
        if result.error:
            findings.append(
                Finding(
                    "deep-build",
                    f"{result.name}: {result.error}",
                    f"fix {unit['source']} or its profile",
                )
            )
            continue
        assert result.obj_size is not None
        if result.obj_size < result.span:
            start = unit["start"]
            delta = result.span - result.obj_size
            findings.append(
                Finding(
                    "deep-size",
                    f"{result.name}: declared span 0x{result.span:X} "
                    f"(0x{start:X}->0x{start + result.span:X}) exceeds compiled "
                    f".text 0x{result.obj_size:X} by 0x{delta:X}; "
                    "trim_elf_section_pad.py will abort with "
                    f"`target size 0x{result.span:X} > current 0x{result.obj_size:X}`",
                    f"end the span at 0x{start + result.obj_size:X} "
                    f"(move the following span/edge up 0x{delta:X})",
                )
            )
        elif result.obj_size > result.span and result.nonzero_tail:
            findings.append(
                Finding(
                    "deep-pad",
                    f"{result.name}: compiled .text 0x{result.obj_size:X} is larger than "
                    f"span 0x{result.span:X} and the tail past the span is not all zero",
                    "the surplus bytes are real code/data, not gas alignment pad; "
                    "extend the span to cover them",
                )
            )

    if strong_error is not None:
        print(
            f"disc1_preflight: WARN strong per-leaf deep pass skipped ({strong_error})",
            file=sys.stderr,
        )
        return findings

    for name, strong_result in strong_by_name.items():
        unit = unit_by_name[name]
        span = unit["size"]
        start = unit["start"]
        if strong_result.error:
            findings.append(
                Finding(
                    "deep-build",
                    f"{name}: {strong_result.error}",
                    f"fix {unit['source']} or its profile",
                )
            )
            continue
        if not strong_result.link_ok:
            first = ", ".join(
                f"{addr:#x}: ROM {rw:08x} LNK {lw:08x}"
                for addr, rw, lw in strong_result.first_mismatch[:4]
            )
            findings.append(
                Finding(
                    "deep-link",
                    f"{name}: object does not match retail at its retail VMA over the "
                    f"declared span ({strong_result.mismatches} word mismatch(es), "
                    f"pad_nonzero={strong_result.pad_nonzero}); first: {first}",
                    "fix the C or the span size; do not rely on "
                    "era_link_check.py, which compares min(linked, size) words and "
                    "can print LINK_EXACT for an oversized span",
                )
            )
        if not strong_result.terminator_ok:
            findings.append(
                Finding(
                    "deep-boundary",
                    f"{name}: span 0x{span:X} (0x{start:X}->0x{start + span:X}) does not "
                    f"end on a function boundary (terminator {strong_result.terminator}); "
                    "a genuine function ends in a bare `jr $ra`, a tail `j`, or an indirect `jr $reg`",
                    "re-derive the true function end and fix the span edge",
                )
            )
        if not strong_result.interior_ok:
            offsets = ", ".join(f"0x{o:X}" for o in strong_result.interior_jr_ra[:6])
            findings.append(
                Finding(
                    "deep-interior-return",
                    f"{name}: span 0x{span:X} contains {strong_result.interior_count} "
                    f"interior `jr $ra` before the terminal one (offsets {offsets}); the "
                    "span likely swallows an adjacent function",
                    "split the span at the interior return so each function is carved "
                    "separately",
                )
            )
    return findings


# ---------------------------------------------------------------------------
# Driver
# ---------------------------------------------------------------------------
def run_checks(args: argparse.Namespace) -> tuple[list[Finding], dict[str, Any]]:
    root = args.root.resolve()
    # `target` is optional on the namespace so existing callers (and the test
    # fixtures) that never set it keep the EXE behaviour.
    try:
        target = load_target(getattr(args, "target", None), root=root)
    except TargetError as exc:
        return [Finding("target", str(exc), "pass a manifest overlay id or omit --target")], {}
    config_arg = args.config if args.config is not None else Path(target.config)
    profiles_arg = args.profiles if args.profiles is not None else Path(target.profiles)
    config = _repo_path(root, config_arg)
    profiles_path = _repo_path(root, profiles_arg)

    findings: list[Finding] = []
    if not config.is_file():
        return [
            Finding(
                "geometry",
                f"missing config: {config}",
                "run from the repository root",
            )
        ], {}

    findings.extend(check_yaml_syntax(config))
    rows = parse_rows(config)
    findings.extend(check_yaml_sha1(config))
    findings.extend(check_geometry(config, rows, target))
    findings.extend(check_c_sources(config, root, rows, target.src_dir))
    findings.extend(check_profiles(profiles_path, rows, root))

    # Authority cross-check: the real plan builder must also accept the tree.
    # Its message is only used if the granular checks missed something.
    try:
        plan = build_plan(root=root, config=config_arg, profiles_path=profiles_arg, target=target)
    except PlanError as exc:
        if not findings:
            findings.append(
                Finding("plan", str(exc), "disc1_plan.py rejects this configuration")
            )
        plan = {}
    counts = plan.get("counts", {}) if plan else {}

    if args.deep:
        findings.extend(
            check_deep(
                config,
                profiles_path,
                root,
                args.jobs,
                set(args.only) if args.only else None,
                target,
                parse_source_overrides(getattr(args, "source_override", None) or []),
            )
        )

    summary = {
        "config": str(config),
        "rows": len(rows),
        "counts": counts,
        "deep": bool(args.deep),
        "findings": len(findings),
    }
    return findings, summary


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=REPO_ROOT)
    parser.add_argument(
        "--target",
        default=None,
        metavar="ID",
        help="preflight target: disc1 (default, the EXE) or an overlay id from "
        "configs/USA/overlays/manifest.yaml",
    )
    parser.add_argument("--config", type=Path, default=None,
                        help="YAML authority (default: the target's config)")
    parser.add_argument("--profiles", type=Path, default=None,
                        help="build-profile JSON (default: the target's)")
    parser.add_argument(
        "--deep",
        action="store_true",
        help="compile every C leaf and compare its .text size to the declared span",
    )
    parser.add_argument(
        "--only",
        action="append",
        default=[],
        metavar="FUNC",
        help="restrict --deep to this C symbol (repeatable)",
    )
    parser.add_argument(
        "--jobs",
        type=int,
        default=min(12, os.cpu_count() or 4),
        help="parallel compile jobs for --deep (default 12)",
    )
    parser.add_argument(
        "--source-override",
        action="append",
        default=[],
        metavar="FUNC=PATH",
        help="with --deep: compile PATH instead of the leaf's src/FUNC.c "
        "(repeatable; verifies a draft re-spelling without touching src/)",
    )
    parser.add_argument("--json", action="store_true", help="emit a JSON summary")
    args = parser.parse_args(argv)

    findings, summary = run_checks(args)

    if args.json:
        print(
            json.dumps(
                {
                    **summary,
                    "findings": [
                        {"check": f.check, "message": f.message, "remedy": f.remedy}
                        for f in findings
                    ],
                },
                indent=2,
            )
        )
    elif findings:
        for finding in findings:
            print(f"disc1_preflight: FAIL [{finding.check}] {finding.message}")
            print(f"  remedy: {finding.remedy}")
    else:
        counts = summary.get("counts") or {}
        extra = (
            f", {counts.get('c', '?')} c / {counts.get('asm', '?')} asm / "
            f"{counts.get('rodata', '?')} rodata"
            if counts
            else ""
        )
        mode = "deep" if summary.get("deep") else "fast"
        print(f"disc1_preflight: PASS ({mode}{extra})")

    if findings:
        print(
            f"disc1_preflight: FAIL {len(findings)} finding(s) — "
            "fix before running the expensive split/build",
            file=sys.stderr,
        )
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
