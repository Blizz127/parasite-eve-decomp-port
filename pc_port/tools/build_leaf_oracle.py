#!/usr/bin/env python3
"""Build matched src/ leaves to PSX machine code for the test-only oracle.

Native-only rule (2026-10-07): the port never executes MIPS code.  The test
suite still needs an objective reference for hand adapters (game/decomp_hand)
that the generator cannot derive.  This tool compiles the MATCHED C leaves in
src/ with the era toolchain (scripts/setup_era.sh: gcc-2.7.2/2.8.1-psx cc1 +
maspsx + GNU as) using each leaf's build profile from
configs/USA/disc1_build_profiles.json, links them together in one image at
ORACLE_BASE (a RAM window the AKAO/field data never uses; data symbols D_X
resolve to their real addresses, calls between the leaves resolve inside the
image), and writes the raw image plus a symbol manifest into a git-ignored
build directory.

The oracle needs the leaves' SEMANTICS, not their retail bytes, so per-leaf
yaml-only maspsx levers are not applied and the image is relocated (a leaf
whose era output runs a few words past its retail span cannot clobber its
neighbour).  The image is still close to retail code, so it is written ONLY
under build/ (git-ignored) and must never be committed
(tools/analysis/retail_data_guard.py).  No disc is read.

The test executable pe-leaf-oracle-tests loads the blobs into guest RAM and
runs them in tests/pe_akao_interp_oracle.c against the native C on the same
randomised guest state.

Usage: pc_port/tools/build_leaf_oracle.py [--out build/leaf-oracle] [func_X ...]
       (no leaves = DEFAULT_LEAVES, the set pe-leaf-oracle-tests checks)
"""
import argparse
import json
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PROFILES = ROOT / "configs/USA/disc1_build_profiles.json"
YAML = ROOT / "configs/USA/disc1.yaml"
MASPSX = ROOT / "tools/era/maspsx/maspsx.py"
AS = "mips-linux-gnu-as"
LD = "mips-linux-gnu-ld"
OBJCOPY = "mips-linux-gnu-objcopy"
EXE_BASE = 0x80010000 - 0x800


def profile_for(name, prof):
    for pname, names in prof["assignments"].items():
        if name in names:
            p = prof["profiles"][pname]
            if p["toolchain"] == "era":
                return pname, p
            # "modern" leaves need a MIPS gcc this box does not have; the
            # oracle only needs the leaf's semantics, so use the default era
            # profile and say so in the manifest.
            d = prof["default_profile"]
            return f"{d} (substituted for {pname})", prof["profiles"][d]
    d = prof["default_profile"]
    return d, prof["profiles"][d]


def spans():
    out = []
    for m in re.finditer(r"-\s*\[\s*(0x[0-9A-Fa-f]+)\s*,\s*([a-z_]+)\s*(?:,\s*([A-Za-z0-9_]+))?",
                         YAML.read_text(encoding="utf-8")):
        out.append((int(m.group(1), 16), m.group(2), m.group(3)))
    return out


def run(cmd, **kw):
    r = subprocess.run(cmd, capture_output=True, **kw)
    if r.returncode:
        sys.stderr.write(r.stderr.decode(errors="replace"))
        raise SystemExit(f"build_leaf_oracle: failed: {' '.join(map(str, cmd))}")
    return r


ORACLE_BASE = 0x801C0000

# The leaves pe-leaf-oracle-tests checks plus their callee closure.
DEFAULT_LEAVES = [
    "func_8008A750", "func_8008AC40", "func_80089F58", "func_8008F178", "func_8008F0D0",
    "func_80089FE0", "func_80089960", "func_80089B28", "func_80089CF0", "func_8008D820",
    "func_8008A92C", "func_8008A400", "func_8008A8CC", "func_8008F1B0", "func_8008AB1C",
    "func_8008B084", "func_8008B168", "func_8008B124", "func_8008B0C8", "func_80090C88",
]


def compile_leaf(name, prof, t):
    src = ROOT / "src" / f"{name}.c"
    if not src.exists():
        raise SystemExit(f"build_leaf_oracle: no matched source {src}")
    pname, p = profile_for(name, prof)
    # FOLD knobs only retarget local switch/rodata tables onto the retail
    # table symbols (which live in retail rodata we do not have); dropping
    # them keeps cc1's own table in the object with identical semantics.
    e = {k: v for k, v in p.get("environment", {}).items()
         if k not in ("MASPSX_DISPATCH_FOLD", "MASPSX_RODATA_FOLD", "MASPSX_CDK_SPLIT_DISPATCH")}
    env = os.environ.copy()
    for k in ("MASPSX_DISPATCH_FOLD", "MASPSX_RODATA_FOLD", "MASPSX_CDK_SPLIT_DISPATCH"):
        env.pop(k, None)
    env.update(e)
    gdir = ROOT / f"tools/era/gcc-{e.get('ERA_CC1_VER', '2.7.2')}-psx"
    with open(t / f"{name}.i", "wb") as f:
        subprocess.run([str(gdir / "cpp"), str(src)], stdout=f, stderr=subprocess.DEVNULL,
                       env=env, check=True)
    run([str(gdir / "cc1"), "-quiet", *p["flags"], str(t / f"{name}.i"), "-o", str(t / f"{name}.s")], env=env)
    g = next((f for f in p["flags"] if re.fullmatch(r"-G\d+", f)), "-G0")
    cmd = [sys.executable, str(MASPSX), f"--aspsx-version={e.get('ERA_ASPSX_VER', '2.30')}", g]
    if e.get("MASPSX_EXPAND_LI") != "1":
        cmd.append("--dont-expand-li")
    if e.get("MASPSX_EXPAND_DIV") == "1":
        cmd.append("--expand-div")
    if e.get("MASPSX_USE_COMM_SECTION") == "1":
        cmd.append("--use-comm-section")
    cmd.append(str(t / f"{name}.s"))
    r = run(cmd, env=env, stdin=subprocess.DEVNULL)
    (t / f"{name}.m.s").write_bytes(r.stdout)
    run([AS, "-EL", "-mips1", "-mabi=32", "-I", str(ROOT / "include"),
         "-o", str(t / f"{name}.o"), str(t / f"{name}.m.s")], env=env)
    sects = run(["mips-linux-gnu-objdump", "-h", str(t / f"{name}.o")]).stdout.decode()
    # read-only data (switch jump tables, constants) is merged into the image
    # by the linker script below; writable sections are not supported.
    for sec in (".data", ".sdata", ".bss", ".sbss"):
        m = re.search(rf"\s{re.escape(sec)}\s+([0-9a-f]+)", sects)
        if m and int(m.group(1), 16):
            raise SystemExit(f"build_leaf_oracle: {name}: non-empty {sec}; not supported")
    return pname


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default=str(ROOT / "build/leaf-oracle"))
    ap.add_argument("--range", nargs=2, metavar=("LO", "HI"),
                    help="also add every src/func_X.c with LO <= X < HI (hex)")
    ap.add_argument("leaves", nargs="*")
    a = ap.parse_args()
    if not a.leaves:
        a.leaves = list(DEFAULT_LEAVES)
    if a.range:
        lo, hi = int(a.range[0], 16), int(a.range[1], 16)
        for f in sorted((ROOT / "src").glob("func_*.c")):
            m = re.fullmatch(r"func_([0-9A-F]{8})", f.stem)
            if m and lo <= int(m.group(1), 16) < hi and f.stem not in a.leaves:
                a.leaves.append(f.stem)
    # callee closure from the matched sources
    i = 0
    while i < len(a.leaves):
        src = ROOT / "src" / f"{a.leaves[i]}.c"
        if src.exists():
            for c in sorted(set(re.findall(r"func_[0-9A-F]{8}", src.read_text(errors="replace")))):
                if c not in a.leaves:
                    a.leaves.append(c)
        i += 1
    out = Path(a.out)
    out.mkdir(parents=True, exist_ok=True)
    prof = json.loads(PROFILES.read_text())
    sp = spans()
    with tempfile.TemporaryDirectory(prefix="pe-oracle-") as t:
        t = Path(t)
        profiles, bad = {}, {}
        for n in a.leaves:
            try:
                profiles[n] = compile_leaf(n, prof, t)
            except SystemExit as e:
                bad[n] = str(e)
        # leaves that need retail-only data (external jump tables etc.)
        for n in list(profiles):
            for line in run(["mips-linux-gnu-nm", str(t / f"{n}.o")]).stdout.decode().splitlines():
                parts = line.split()
                if (len(parts) == 2 and parts[0] == "U"
                        and not re.fullmatch(r"(D|func)_[0-9A-Fa-f]{8}", parts[1])):
                    bad[n] = f"needs external {parts[1]}"
                    del profiles[n]
                    break
        # drop failed leaves and (transitively) every leaf that calls one
        changed = True
        while changed:
            changed = False
            for n in list(profiles):
                txt = (ROOT / "src" / f"{n}.c").read_text(errors="replace")
                if any(c in bad for c in re.findall(r"func_[0-9A-F]{8}", txt) if c != n):
                    bad[n] = "calls a dropped leaf"
                    del profiles[n]
                    changed = True
        for n, why in bad.items():
            print(f"  dropped {n}: {why.splitlines()[-1] if why else ''}")
        a.leaves = [n for n in a.leaves if n in profiles]
        objs = [str(t / f"{n}.o") for n in a.leaves]
        undef = set()
        for o in objs:
            for line in run(["mips-linux-gnu-nm", o]).stdout.decode().splitlines():
                parts = line.split()
                if len(parts) == 2 and parts[0] == "U":
                    undef.add(parts[1])
        defs = []
        for u in sorted(undef - set(a.leaves)):
            m = re.fullmatch(r"D_([0-9A-Fa-f]{8})", u)
            if not m:
                raise SystemExit(f"build_leaf_oracle: {u} is called but not in the leaf set; add it")
            defs += ["--defsym", f"{u}=0x{m.group(1)}"]
        (t / "oracle.ld").write_text(
            "SECTIONS { . = 0x%08X; .text : { *(.text) . = ALIGN(4); "
            "*(.rodata) *(.rodata.*) *(.rdata) } /DISCARD/ : { *(.reginfo) "
            "*(.MIPS.abiflags) *(.pdr) *(.comment) *(.gnu.attributes) } }\n" % ORACLE_BASE)
        run([LD, "-EL", "-T", str(t / "oracle.ld"), "-e", a.leaves[0], *defs,
             "-o", str(t / "image.elf"), *objs])
        run([OBJCOPY, "-O", "binary", "-j", ".text", str(t / "image.elf"), str(out / "image.bin")])
        syms = {}
        for line in run(["mips-linux-gnu-nm", str(t / "image.elf")]).stdout.decode().splitlines():
            parts = line.split()
            if len(parts) == 3 and parts[2] in a.leaves:
                syms[parts[2]] = int(parts[0], 16)
    size = (out / "image.bin").stat().st_size
    with open(out / "manifest.txt", "w") as f:
        f.write(f"image {ORACLE_BASE:08X} {size}\n")
        for n in a.leaves:
            f.write(f"{n} {syms[n]:08X}\n")
    for n in a.leaves:
        print(f"{n} -> {syms[n]:08X} [{profiles[n]}]")
    print(f"build_leaf_oracle: {len(a.leaves)} leaves, {size} bytes at {ORACLE_BASE:08X} -> {out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
