#!/usr/bin/env python3
"""Audit folded switch tables: C case labels must equal the retail jump table.

`MASPSX_DISPATCH_FOLD=<sym>[,<sym>...]` retargets cc1's switch-table
reference to the retail table symbol (`jtbl_XXXXXXXX`) and drops cc1's own
table, so the byte-exact gate never looks at which C `case` label sits on
which body.  A leaf can therefore byte-match with its case labels on the
wrong bodies (found in src/func_80017018.c, the field-VM operand decoder).

For every leaf whose build profile sets MASPSX_DISPATCH_FOLD this tool
compiles the leaf through the same era pipeline as era_link_check.py but
WITHOUT the fold, so cc1 emits its own table from the C labels; renames cc1's
local `$L` labels to real symbols before assembling so their linked addresses
can be read back; and compares, entry by entry, cc1's table (label -> body
offset from the function start) with the retail table read from the EXE or
overlay blob at the jtbl address.  Tables are matched to fold symbols in
`.rdata` order, as maspsx does.

Usage: tools/analysis/dispatch_fold_audit.py [--only NAME ...] [--exe-only]
Prints one line per table: OK / MISMATCH (with the differing cases) / SKIP
(with the reason), then a summary.  Exit 1 when any table mismatches.
"""
import json
import os
import re
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(ROOT / "tools" / "analysis"))
sys.path.insert(0, str(ROOT / "tools" / "build"))
import era_link_check as elc  # noqa: E402  (same pipeline + symbol convention)

EXE = elc.EXE if hasattr(elc, "EXE") else ROOT / "build" / "extracted" / "disc1" / "SLUS_006.62"


def profile_sets(path):
    """(leaf, flags, env) for every leaf on a fold profile in one JSON."""
    d = json.loads(Path(path).read_text())
    out = []
    profiles = {}
    if d.get("profiles_from"):   # overlay files inherit the EXE definitions
        profiles.update(json.loads((ROOT / d["profiles_from"]).read_text()).get("profiles", {}))
    profiles.update(d.get("profiles", {}))
    profiles.update(d.get("local_profiles", {}))
    for pname, prof in profiles.items():
        env = prof.get("environment", {})
        if "MASPSX_DISPATCH_FOLD" not in env:
            continue
        for leaf in d.get("assignments", {}).get(pname, []):
            out.append((leaf, prof.get("flags", ["-O2", "-G0"]), env))
    return out


def compile_unfolded(src, vram, flags, env):
    """Return (label_addr: dict, tables: list[list[label]], func_addr)."""
    cc1_ver = env.get("ERA_CC1_VER", elc.CC1_VER)
    cc1 = elc.ERA / f"gcc-{cc1_ver}-psx" / "cc1"
    cpp = elc.ERA / f"gcc-{cc1_ver}-psx" / "cpp"
    run_env = dict(os.environ)
    run_env.update({k: v for k, v in env.items() if k != "MASPSX_DISPATCH_FOLD"})
    run_env.pop("MASPSX_DISPATCH_FOLD", None)
    host_env = dict(elc.HOST_ENV)
    with tempfile.TemporaryDirectory(dir=os.environ.get("TMPDIR")) as d:
        d = Path(d)
        with open(d / "x.i", "wb") as fi:
            r = subprocess.run([str(cpp), str(src)], stdout=fi, stderr=subprocess.DEVNULL)
        if r.returncode:
            raise RuntimeError("cpp failed")
        r = subprocess.run([str(cc1), "-quiet", *flags, str(d / "x.i"), "-o", str(d / "x.s")],
                           stderr=subprocess.DEVNULL)
        if r.returncode:
            raise RuntimeError("cc1 failed")
        forced = run_env.get("MASPSX_FORCE_ABSOLUTE_SYMBOLS")
        if forced:
            text = (d / "x.s").read_text()
            for symbol in forced.split(","):
                text = re.sub(rf"\t\.extern\t{re.escape(symbol)}, \d+\n", "", text)
            (d / "x.s").write_text(text)
        cmd = [sys.executable, str(elc.MASPSX),
               f"--aspsx-version={run_env.get('ERA_ASPSX_VER', elc.ERA_ASPSX_VER_DEFAULT)}",
               "--dont-expand-li"]
        if run_env.get("MASPSX_EXPAND_DIV") == "1":
            cmd.append("--expand-div")
        if run_env.get("MASPSX_USE_COMM_SECTION") == "1":
            cmd.append("--use-comm-section")
        cmd += [elc.maspsx_sdata_flag(flags), str(d / "x.s")]
        with open(d / "xm.s", "w") as fo:
            r = subprocess.run(cmd, stdin=subprocess.DEVNULL, stdout=fo,
                               stderr=subprocess.DEVNULL, env=run_env)
        if r.returncode:
            raise RuntimeError("maspsx failed")
        asm = (d / "xm.s").read_text()
        # Collect cc1's switch tables: runs of `.word $Lnn` in .rdata.
        tables, cur, section = [], None, ".text"
        for line in asm.splitlines():
            s = line.strip()
            if s.startswith(".section") or s in (".text", ".data", ".rdata", ".rodata", ".sdata"):
                section = s.split()[-1] if s.startswith(".section") else s
                section = section.split(",")[0]
                cur = None
                continue
            m = re.match(r"\.(?:word|gpword)\s+\$L(\d+)$", s)
            if m and section in (".rdata", ".rodata"):
                if cur is None:
                    cur = []
                    tables.append(cur)
                cur.append(m.group(1))
            elif s and not s.startswith(".align"):
                if not re.match(r"\$L\d+:$", s):
                    cur = None
                elif cur is not None and cur:
                    cur = None
        asm = re.sub(r"\$L(\d+)", r"JTL_\1", asm)
        (d / "xm.s").write_text(asm)
        r = subprocess.run([str(elc.AS), "-EL", "-mips1", "-mabi=32", "-I", str(ROOT / "include"),
                            "-o", str(d / "x.o"), str(d / "xm.s")], env=host_env,
                           capture_output=True, text=True)
        if r.returncode:
            raise RuntimeError("as failed: " + r.stderr[:200])
        nm = subprocess.run([str(elc.NM), "-u", str(d / "x.o")], capture_output=True,
                            text=True, env=host_env).stdout
        defsyms = []
        for line in nm.splitlines():
            p = line.split()
            if len(p) >= 2 and p[0] == "U":
                a = elc.sym_address(p[1])
                if a is None:
                    raise RuntimeError(f"unresolved {p[1]}")
                defsyms += ["--defsym", f"{p[1]}={a:#x}"]
        (d / "link.ld").write_text(
            "SECTIONS {\n"
            f"  . = {vram:#x};\n"
            "  .text : SUBALIGN(4) { *(.text) }\n"
            "  .rodata : { *(.rdata) *(.rodata) }\n"
            "  /DISCARD/ : { *(.MIPS.abiflags) *(.reginfo) *(.pdr) *(.comment) }\n"
            "}\n")
        r = subprocess.run([str(elc.LD), "-T", str(d / "link.ld"), "-o", str(d / "x.elf"),
                            str(d / "x.o"), "--defsym", "_gp=0x8009CD70", *defsyms],
                           capture_output=True, text=True, env=host_env)
        if r.returncode:
            raise RuntimeError("ld failed: " + (r.stdout + r.stderr)[:200])
        nmall = subprocess.run([str(elc.NM), str(d / "x.elf")], capture_output=True,
                               text=True, env=host_env).stdout
    addr, func = {}, None
    for line in nmall.splitlines():
        p = line.split()
        if len(p) == 3:
            if p[2].startswith("JTL_"):
                addr[p[2][4:]] = int(p[0], 16)
            elif p[2] == Path(src).stem:
                func = int(p[0], 16)
    return addr, tables, func


def retail_words(target, address, count):
    if target is None:
        data, load_vram, file_start = EXE.read_bytes(), 0x80010000, 0x800
    else:
        from overlay_targets import load_target  # noqa: PLC0415
        t = load_target(target, root=ROOT)
        data, load_vram, file_start = (ROOT / t.retail).read_bytes(), t.vram, t.file_start
    off = address - load_vram + file_start
    return [struct.unpack_from("<I", data, off + 4 * i)[0] for i in range(count)]


def main():
    argv = sys.argv[1:]
    only = set()
    exe_only = "--exe-only" in argv
    argv = [a for a in argv if a != "--exe-only"]
    while "--only" in argv:
        i = argv.index("--only")
        only.add(argv[i + 1])
        del argv[i:i + 2]
    jobs = [(None, *s) for s in profile_sets(ROOT / "configs" / "USA" / "disc1_build_profiles.json")]
    if not exe_only:
        for p in sorted((ROOT / "configs" / "USA" / "overlays").glob("*_build_profiles.json")):
            tgt = p.name[: -len("_build_profiles.json")]
            jobs += [(tgt, *s) for s in profile_sets(p)]
    ok = bad = skip = 0
    for target, leaf, flags, env in jobs:
        if only and leaf not in only:
            continue
        src = ROOT / "src" / (f"overlays/{target}/{leaf}.c" if target else f"{leaf}.c")
        where = f"{target or 'exe'}:{leaf}"
        if not src.exists():
            print(f"SKIP {where}: no src"); skip += 1; continue
        syms = [s for s in env["MASPSX_DISPATCH_FOLD"].split(",") if s]
        try:
            vram = int(leaf.split("_")[1], 16)
            addr, tables, func = compile_unfolded(src, vram, flags, env)
        except Exception as e:  # noqa: BLE001
            print(f"SKIP {where}: {e}"); skip += 1; continue
        if func is None or len(tables) < len(syms):
            print(f"SKIP {where}: {len(tables)} cc1 table(s) for {len(syms)} fold symbol(s)")
            skip += 1
            continue
        for sym, table in zip(syms, tables):
            m = re.match(r"jtbl_([0-9A-Fa-f]{8})$", sym)
            if not m:
                print(f"SKIP {where} {sym}: not a jtbl_<addr> symbol"); skip += 1; continue
            retail = retail_words(target, int(m.group(1), 16), len(table))
            diffs = []
            for k, (lab, rw) in enumerate(zip(table, retail)):
                mine = addr.get(lab)
                if mine is None or mine - func != rw - vram:
                    diffs.append(f"case{k}: C->+{(mine - func) if mine else -1:#x} retail->+{rw - vram:#x}")
            if diffs:
                bad += 1
                print(f"MISMATCH {where} {sym} ({len(table)} entries): " + "; ".join(diffs[:8])
                      + (f" (+{len(diffs) - 8} more)" if len(diffs) > 8 else ""))
            else:
                ok += 1
                print(f"OK {where} {sym} ({len(table)} entries)")
    print(f"dispatch_fold_audit: {ok} ok, {bad} mismatch, {skip} skipped")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
