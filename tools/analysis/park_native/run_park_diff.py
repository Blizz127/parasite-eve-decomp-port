#!/usr/bin/env python3
"""Serial directed retail/native differentials for the seven added park leaves."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import sys
import time

ANALYSIS = Path(__file__).resolve().parents[1]
ROOT = ANALYSIS.parents[1]
sys.path.insert(0, str(ANALYSIS))

import e21_diff as differential
import park_inputs_m0086 as m86
import park_inputs_m0089 as m89
import park_inputs_m0089_effect as effect


# Deliberately excludes 80190574, which belongs to the composed fixture suite.
TARGETS = (
    ("room_m0086i", "func_8018F004", m86),
    ("room_m0086i", "func_8018F734", m86),
    ("room_m0086i", "func_801900CC", m86),
    ("room_m0089i", "func_801924F8", m89),
    ("room_m0089i", "func_80192CD4", m89),
    ("room_m0089i", "func_80193200", m89),
    ("room_m0089i", "func_80193618", effect),
)

OUT_ARGS = {
    "func_8018F004": ((0x800CFB7C, 2), (0x80079754, 1), (0x80078CC4, 0)),
    "func_8018F734": ((0x800CFB7C, 2), (0x800CE8F0, 3), (0x800CEAE8, 2),
                      (0x800CE9D4, 2), (0x80079754, 1), (0x80078CC4, 0)),
    "func_80192CD4": ((0x80079754, 1), (0x80078CC4, 0)),
    "func_80193618": ((0x80079754, 1), (0x80078CC4, 0), (0x800CF3AC, 1)),
}


def discover(targets):
    """Resolve current generated hashes from manifest membership and provenance.

    Refuse missing or ambiguous candidates instead of silently dropping a
    function or choosing an obsolete generated hash left in the directory.
    """
    manifest = differential.o.nonmatching_leaves()
    directory = ROOT / "pc_port/game/decomp_ovl/rooms"
    found = []
    for overlay, name, provider in targets:
        if name not in manifest.get(overlay, {}):
            raise ValueError(f"{overlay}/{name} is absent from nonmatching.yaml")
        provenance = re.compile(r"decomp-source-overlay:\s*" + re.escape(overlay + "/" + name) + r"\b")
        paths = [path for path in sorted(directory.glob(name + "__*_port.c"))
                 if provenance.search(path.read_text())]
        if len(paths) != 1:
            raise ValueError(f"{overlay}/{name}: expected one generated TU, found {len(paths)}; regenerate and resolve stale candidates")
        found.append((overlay, name, provider, paths[0]))
    return found


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--only", default="", help="function-name or address substring; default: all seven")
    parser.add_argument("--output-dir", type=Path, default=Path("build/park-evidence"),
                        help="evidence directory; relative paths are rooted at the repository")
    parser.add_argument("--cc", default=shutil.which("cc") or shutil.which("gcc"),
                        help="existing host C compiler")
    args = parser.parse_args(argv)
    targets = [target for target in TARGETS if args.only in target[1]]
    if not targets:
        parser.error("--only matched none of the seven added functions")
    if not args.cc:
        parser.error("an existing C compiler is required (--cc)")
    try:
        candidates = discover(targets)
    except ValueError as error:
        parser.error(str(error))
    output = args.output_dir if args.output_dir.is_absolute() else ROOT / args.output_dir
    output.mkdir(parents=True, exist_ok=True)
    tag = re.sub(r"[^A-Za-z0-9_-]", "_", args.only) or "all"
    results = []
    for overlay, name, provider, path in candidates:
        attempted = []

        def directed_inputs(n, ov, data, seeds, recorded, spec):
            for seed, ram, call_args in provider.cases(n, ov, data):
                attempted.append(seed)
                yield seed, ram, call_args

        specs = differential.directed_specs()
        extra = [target << 4 | arg for target, arg in OUT_ARGS.get(name, ())]
        if extra:
            specs.setdefault((overlay, name), []).append(("out", extra))
        previous_inputs = differential.inputs
        started = time.monotonic()
        print("START", overlay, name, flush=True)
        try:
            differential.inputs = directed_inputs
            status, detail = differential.one(path, name, overlay, 0, args.cc, True,
                                               directed=specs, check_gte=True)
        except Exception as error:
            status, detail = "error", f"{type(error).__name__}: {error}"
        finally:
            differential.inputs = previous_inputs
        match = re.search(r"(\d+) seeds", detail)
        accepted = int(match.group(1)) if match else 0
        complete = status == "ok" and bool(attempted) and accepted == len(attempted)
        result = dict(overlay=overlay, name=name, status=status, detail=detail,
                      attempted=len(attempted), accepted=accepted, complete=complete,
                      seconds=round(time.monotonic() - started, 2),
                      generated_source=str(path.relative_to(ROOT)),
                      generated_sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
                      provider_sha256=hashlib.sha256(Path(provider.__file__).read_bytes()).hexdigest(),
                      oracle_sha256=hashlib.sha256(Path(differential.__file__).read_bytes()).hexdigest(),
                      gte_sha256=hashlib.sha256((ROOT / "pc_port/platform/pe_gte.c").read_bytes()).hexdigest(),
                      coverage_addresses=sorted(differential.COVER or []))
        results.append(result)
        (output / f"coverage-{name}.json").write_text(json.dumps(result, indent=2) + "\n")
        (output / f"directed-{tag}.json").write_text(json.dumps(results, indent=2) + "\n")
        print(json.dumps({key: value for key, value in result.items()
                          if key != "coverage_addresses"}), flush=True)
        if not complete:
            return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
