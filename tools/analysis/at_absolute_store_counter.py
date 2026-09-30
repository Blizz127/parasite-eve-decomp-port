#!/usr/bin/env python3
"""Count short $at absolute-store setters from yaml-live asm units.

Selection rule (Phase 5EE/5EG):
  - at most 7 instructions
  - contains lui $at,%hi(D_*)
  - matching absolute store through $at (sw/sb/sh ... %lo(D_*)($at))
  - contains jr $ra

Shape split:
  - pre-jr     : store mnemonic is sw, and the store appears before jr $ra
  - delay-slot : store mnemonic is sw, and the store is in the jr $ra delay slot
  - sb-sh      : any selected function whose absolute store width is sb or sh

Only yaml-live [offset, asm] units from configs/USA/disc1.yaml are scanned.
Orphan .s files at nearby offsets are ignored.

Sync invariant (hard fail):
  asm/-derived totals are only valid when asm/ is current with yaml. Before any
  SUMMARY/total is printed, this tool requires (1) every yaml ``, asm]`` unit
  has a matching .s on disk and (2) no yaml ``, c,`` leaf still appears as a
  glabel inside those scanned units (the stale pre-split 74FB0/func_80085728
  class). Stale or missing asm/ exits non-zero with no authoritative total —
  run ``scripts/split_us.sh``.

Usage:
  tools/analysis/at_absolute_store_counter.py
  tools/analysis/at_absolute_store_counter.py --yaml configs/USA/disc1.yaml
  tools/analysis/at_absolute_store_counter.py --shape pre-jr
"""

from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path

# Exchange shape (read/replace), not a simple setter — excluded from pre-jr harvest.
EXCLUDE_FROM_PRE_JR = frozenset({"func_80074A14"})

INSTR_RE = re.compile(
    r"/\*\s*([0-9A-Fa-f]+)\s+([0-9A-Fa-f]+)\s+([0-9A-Fa-f]+)\s*\*/\s*"
    r"(\S+)(.*)$"
)
GLABEL_RE = re.compile(r"^glabel\s+(func_\w+)\s*$")
LUI_AT_HI = re.compile(r"lui\s+\$at\s*,\s*%hi\((D_[0-9A-Fa-f]+)\)")
STORE_AT_LO = re.compile(r"(sw|sb|sh)\s+\$\w+\s*,\s*%lo\((D_[0-9A-Fa-f]+)\)\(\$at\)")
JR_RA = re.compile(r"jr\s+\$ra\b")
YAML_ASM_RE = re.compile(r"^\s*-\s*\[(0x[0-9A-Fa-f]+)\s*,\s*asm\]")
YAML_C_RE = re.compile(r"^\s*-\s*\[(0x[0-9A-Fa-f]+)\s*,\s*c\s*,\s*(\w+)\]")


@dataclass
class Instr:
    file_off: int
    vram: int
    word: int
    mnem: str
    rest: str
    raw: str

    @property
    def text(self) -> str:
        return f"{self.mnem}{self.rest}"


@dataclass
class Member:
    name: str
    unit: str
    unit_off: int
    file_off: int
    vram: int
    n_instr: int
    width: str  # sw | sb | sh | mixed
    shape: str  # pre-jr | delay-slot | sb-sh
    globals: list[str] = field(default_factory=list)
    words: list[int] = field(default_factory=list)
    instrs: list[str] = field(default_factory=list)
    excluded_reason: str | None = None


def repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def parse_yaml_asm_units(yaml_path: Path) -> list[tuple[int, str]]:
    """Return sorted (file_offset, unit_basename) for live asm subsegments."""
    units: list[tuple[int, str]] = []
    for line in yaml_path.read_text(encoding="utf-8").splitlines():
        m = YAML_ASM_RE.match(line)
        if m:
            off = int(m.group(1), 16)
            units.append((off, f"{off:X}"))
    return units


def parse_yaml_c_leaves(yaml_path: Path) -> set[str]:
    leaves: set[str] = set()
    for line in yaml_path.read_text(encoding="utf-8").splitlines():
        m = YAML_C_RE.match(line)
        if m:
            leaves.add(m.group(2))
    return leaves


def parse_yaml_c_entries(yaml_path: Path) -> list[tuple[int, str]]:
    """Return (file_offset, leaf_name) for every yaml ``, c,`` leaf."""
    entries: list[tuple[int, str]] = []
    for line in yaml_path.read_text(encoding="utf-8").splitlines():
        m = YAML_C_RE.match(line)
        if m:
            entries.append((int(m.group(1), 16), m.group(2)))
    return entries


def glabel_names_in_unit(s_path: Path) -> list[str]:
    """Return glabel function names in order of appearance (no body parse)."""
    names: list[str] = []
    try:
        text = s_path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return names
    for line in text.splitlines():
        gm = GLABEL_RE.match(line.strip())
        if gm:
            names.append(gm.group(1))
    return names


def check_asm_yaml_sync(yaml_path: Path, asm_root: Path) -> list[str]:
    """Return human-readable problems proving asm/ is not current with yaml.

    Half 1 — missing: yaml lists ``, asm]`` but no corresponding .s on disk.
    Half 2 — stale: a carved ``, c,`` leaf still appears as a glabel inside a
    yaml-live asm unit the counter would scan (pre-split 74FB0.s /
    func_80085728). Orphan .s at C offsets are not scanned and are not checked.

    Empty list means sync is acceptable for producing an authoritative total.
    There is no split manifest in this repo (no durable hash/timestamp of the
    last split), so content checks against yaml are the authority.
    """
    problems: list[str] = []
    units = parse_yaml_asm_units(yaml_path)
    c_names = {name for _, name in parse_yaml_c_entries(yaml_path)}

    if not units:
        problems.append(f"no yaml asm units in {yaml_path}")
        return problems

    if not asm_root.is_dir():
        problems.append(
            f"asm root missing: {asm_root} (run scripts/split_us.sh)"
        )
        return problems

    # Half 1: every yaml asm unit must exist on disk.
    present: list[tuple[int, str, Path]] = []
    for unit_off, unit_name in units:
        s_path = asm_root / f"{unit_name}.s"
        if not s_path.is_file():
            problems.append(f"missing asm unit: {unit_name}.s (yaml offset 0x{unit_off:X})")
        else:
            present.append((unit_off, unit_name, s_path))

    # Half 2: no yaml C leaf may still appear as a glabel in a scanned asm unit.
    # This is the real 37→36 bug: old 74FB0.s still held func_80085728 after the
    # mid-unit carve to C. (Orphan .s files at C offsets are NOT scanned by this
    # counter — only yaml-live asm units are — so presence of leftover splat
    # orphans at carved offsets is not a count-validity signal.)
    for unit_off, unit_name, s_path in present:
        for gname in glabel_names_in_unit(s_path):
            if gname in c_names:
                problems.append(
                    f"stale glabel in {unit_name}.s: {gname} is a yaml C leaf "
                    f"(asm/ predates carve — re-run scripts/split_us.sh)"
                )

    return problems


def report_sync_failure(problems: list[str], asm_root: Path) -> None:
    """Print a loud, non-authoritative failure; caller must exit non-zero."""
    print("ERROR: asm/ is not in sync with configs/USA/disc1.yaml", file=sys.stderr)
    print(
        "       asm/-derived counts are invalid until split is current.",
        file=sys.stderr,
    )
    print(f"       asm_root: {asm_root}", file=sys.stderr)
    for p in problems:
        print(f"  - {p}", file=sys.stderr)
    print("Remedy: scripts/split_us.sh", file=sys.stderr)
    print(
        "(No SUMMARY/total printed — refusing a stale or incomplete count.)",
        file=sys.stderr,
    )


def parse_functions(s_path: Path, unit_off: int) -> list[tuple[str, list[Instr]]]:
    text = s_path.read_text(encoding="utf-8", errors="replace")
    lines = text.splitlines()
    out: list[tuple[str, list[Instr]]] = []
    i = 0
    while i < len(lines):
        gm = GLABEL_RE.match(lines[i].strip())
        if not gm:
            i += 1
            continue
        name = gm.group(1)
        i += 1
        instrs: list[Instr] = []
        while i < len(lines):
            if GLABEL_RE.match(lines[i].strip()):
                break
            im = INSTR_RE.search(lines[i])
            if im:
                instrs.append(
                    Instr(
                        file_off=int(im.group(1), 16),
                        vram=int(im.group(2), 16),
                        word=int(im.group(3), 16),
                        mnem=im.group(4),
                        rest=im.group(5),
                        raw=lines[i].rstrip(),
                    )
                )
            i += 1
        out.append((name, instrs))
    return out


def classify(name: str, instrs: list[Instr], unit: str, unit_off: int) -> Member | None:
    if len(instrs) == 0 or len(instrs) > 7:
        return None
    texts = [ins.text for ins in instrs]
    full = "\n".join(texts)

    lui_globals = LUI_AT_HI.findall(full)
    if not lui_globals:
        return None
    if not JR_RA.search(full):
        return None

    store_hits: list[tuple[int, str, str]] = []  # (index, width, global)
    for idx, t in enumerate(texts):
        sm = STORE_AT_LO.search(t)
        if sm:
            store_hits.append((idx, sm.group(1), sm.group(2)))

    if not store_hits:
        return None

    # Require at least one store whose global was materialised via lui $at,%hi
    lui_set = set(lui_globals)
    store_hits = [h for h in store_hits if h[2] in lui_set]
    if not store_hits:
        return None

    widths = {h[1] for h in store_hits}
    if widths <= {"sw"}:
        width = "sw"
    elif widths <= {"sb"}:
        width = "sb"
    elif widths <= {"sh"}:
        width = "sh"
    else:
        width = "mixed"

    jr_idxs = [i for i, t in enumerate(texts) if JR_RA.search(t)]
    if not jr_idxs:
        return None
    jr_i = jr_idxs[0]

    # Shape: if any store is sb/sh (or mixed), bucket as sb-sh.
    if width != "sw":
        shape = "sb-sh"
    else:
        # delay-slot if every sw is at jr+1; pre-jr if every sw is before jr;
        # otherwise prefer delay if any sw is in delay (conservative for 5EG).
        sw_idxs = [h[0] for h in store_hits]
        if all(i == jr_i + 1 for i in sw_idxs):
            shape = "delay-slot"
        elif all(i < jr_i for i in sw_idxs):
            shape = "pre-jr"
        elif any(i == jr_i + 1 for i in sw_idxs):
            shape = "delay-slot"
        else:
            shape = "pre-jr"

    excluded = None
    if name in EXCLUDE_FROM_PRE_JR and shape == "pre-jr":
        excluded = "exchange-shape (not a simple setter)"
    # Also catch exchange even if delay-classified: has load of same global.
    if name == "func_80074A14":
        excluded = "exchange-shape (not a simple setter)"
        # Keep shape as classified; phase scopes exclude it from pre-jr harvest.

    globals_ordered: list[str] = []
    for _, _, g in store_hits:
        if g not in globals_ordered:
            globals_ordered.append(g)

    return Member(
        name=name,
        unit=unit,
        unit_off=unit_off,
        file_off=instrs[0].file_off,
        vram=instrs[0].vram,
        n_instr=len(instrs),
        width=width,
        shape=shape,
        globals=globals_ordered,
        words=[ins.word for ins in instrs],
        instrs=[ins.raw.strip() for ins in instrs],
        excluded_reason=excluded,
    )


def scan(yaml_path: Path, asm_root: Path) -> list[Member]:
    """Scan yaml-live asm units. Caller must run check_asm_yaml_sync first."""
    units = parse_yaml_asm_units(yaml_path)
    if not units:
        raise SystemExit(f"ERROR: no asm units in {yaml_path}")
    members: list[Member] = []
    seen: set[str] = set()

    for unit_off, unit_name in units:
        s_path = asm_root / f"{unit_name}.s"
        # Sync check already required presence; defensive skip if raced.
        if not s_path.is_file():
            continue
        for name, instrs in parse_functions(s_path, unit_off):
            m = classify(name, instrs, unit_name, unit_off)
            if m is None:
                continue
            if name in seen:
                # Should not happen with yaml-live units only.
                continue
            seen.add(name)
            members.append(m)

    members.sort(key=lambda m: m.file_off)
    return members


def fmt_words(words: list[int]) -> str:
    return " ".join(f"{w:08x}" for w in words)


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    root = repo_root()
    ap.add_argument(
        "--yaml",
        type=Path,
        default=root / "configs/USA/disc1.yaml",
        help="splat config (default: configs/USA/disc1.yaml)",
    )
    ap.add_argument(
        "--asm-root",
        type=Path,
        default=root / "asm/disc1",
        help="directory of yaml-live .s units (default: asm/disc1)",
    )
    ap.add_argument(
        "--shape",
        choices=["all", "pre-jr", "delay-slot", "sb-sh"],
        default="all",
        help="filter printed members (default: all)",
    )
    ap.add_argument(
        "--include-excluded",
        action="store_true",
        help="include exchange-shape exclusions in pre-jr listing",
    )
    ap.add_argument(
        "--verbose",
        action="store_true",
        help="print instruction bodies",
    )
    args = ap.parse_args(argv)

    yaml_path = args.yaml.resolve()
    asm_root = args.asm_root.resolve()
    if not yaml_path.is_file():
        print(f"ERROR: missing {yaml_path}", file=sys.stderr)
        return 1

    # Hard gate: never emit an authoritative total against stale/missing asm/.
    problems = check_asm_yaml_sync(yaml_path, asm_root)
    if problems:
        report_sync_failure(problems, asm_root)
        return 2

    c_leaves = parse_yaml_c_leaves(yaml_path)
    members = scan(yaml_path, asm_root)

    pre_jr = [
        m
        for m in members
        if m.shape == "pre-jr"
        and m.width == "sw"
        and (args.include_excluded or m.excluded_reason is None)
    ]
    delay = [m for m in members if m.shape == "delay-slot" and m.width == "sw"]
    sb_sh = [m for m in members if m.shape == "sb-sh"]
    excluded = [m for m in members if m.excluded_reason]

    print(f"yaml:      {yaml_path}")
    print(f"asm_root:  {asm_root}")
    print(f"c_leaves:  {len(c_leaves)}")
    print(f"asm_units: {len(parse_yaml_asm_units(yaml_path))}")
    print()
    print("=== population (yaml-live only) ===")
    print(f"total selected:     {len(members)}")
    print(f"  pre-jr (sw):      {len(pre_jr)}   [5EG harvest scope]")
    print(f"  delay-slot (sw):  {len(delay)}   [5EF scope]")
    print(f"  sb-sh:            {len(sb_sh)}   [own probes]")
    print(f"  excluded:         {len(excluded)}")
    if excluded:
        for m in excluded:
            print(f"    - {m.name}: {m.excluded_reason}")
    print()

    def dump(title: str, ms: list[Member]) -> None:
        print(f"=== {title} ({len(ms)}) ===")
        for m in ms:
            g = ",".join(m.globals)
            print(
                f"{m.name}  file=0x{m.file_off:X}  vram=0x{m.vram:08X}  "
                f"unit={m.unit}.s  n={m.n_instr}  width={m.width}  "
                f"shape={m.shape}  globals={g}"
            )
            print(f"  words: {fmt_words(m.words)}")
            if args.verbose:
                for line in m.instrs:
                    print(f"    {line}")
        print()

    if args.shape == "all":
        dump("pre-jr sw setters (5EG)", pre_jr)
        dump("delay-slot sw setters (5EF)", delay)
        dump("sb-sh absolute stores", sb_sh)
    elif args.shape == "pre-jr":
        dump("pre-jr sw setters (5EG)", pre_jr)
    elif args.shape == "delay-slot":
        dump("delay-slot sw setters (5EF)", delay)
    else:
        dump("sb-sh absolute stores", sb_sh)

    # Machine-readable summary line for scripts.
    print(
        f"SUMMARY pre-jr={len(pre_jr)} delay-slot={len(delay)} "
        f"sb-sh={len(sb_sh)} excluded={len(excluded)} total={len(members)}"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
