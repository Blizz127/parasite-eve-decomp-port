#!/usr/bin/env python3
"""Psy-Q exclusion guard for the native PC port (pc_port/).

Owner directive (2026-09-28): the port must not compile or link Sony Psy-Q
SDK library code -- neither original objects nor decompiled / translated
Psy-Q C -- and must not carry Psy-Q-derived data tables.  The MATCHING build
keeps its Psy-Q TUs for byte-exactness; this guard only looks at the port.

Checks
  1. source  -- every C/C++ file under pc_port/ (build dirs excluded) that
                DEFINES `func_XXXXXXXX` for an address classified as Psy-Q in
                configs/USA/psyq_classification.tsv, or carries a
                `decomp-source: func_XXXXXXXX` header for one.
  2. binary  -- (--binary, repeatable) `nm` of a linked port/test executable
                or static library: any defined `func_XXXXXXXX` symbol whose
                address is Psy-Q-classified.
  2b. include -- game TUs (pc_port/game/**, pc_port/bootstrap/**) that include a
                PsyCross header (PsyX/..., psx/...), a Sony Psy-Q SDK header
                (libgpu.h, libgte.h, libcd.h, inline_c.h, ...), a host/OS header
                (SDL, GL, AL, X11, windows.h, pthread.h, ...) or an in-house
                BACKEND header (pe_gpu.h, pe_spu*.h, pe_cdreg.h, pe_mdec.h,
                host_*.h, ...) instead of the pe_plat_* interfaces
                (docs/ARCHITECTURE-PORT.md rule 1).
  3. tables  -- Psy-Q data tables listed (address ranges only, no bytes) in
                configs/USA/psyq_data_ranges.tsv.  With the user's retail EXE
                (--exe, default build/extracted/disc1/SLUS_006.62) the guard
                reads the table bytes from THE USER'S DISC and looks for them
                (a) as raw bytes in every --binary and (b) as numeric literals
                in pc_port sources/headers (u16 runs or distinct u32 words).
                Without the EXE the table check is skipped (warning) unless
                --require-exe.

Allowlist (pc_port/psyq_guard_allowlist.txt) records today's known offenders
so the guard can run in CI before the replacement lands.  It must SHRINK:
  * any finding not in the allowlist fails (new Psy-Q code/tables);
  * any allowlist entry that is no longer found fails (stale -- delete it).
--strict ignores the allowlist entirely: that is the goal state (exit 0 only
when nothing Psy-Q remains).  --write-allowlist regenerates it from findings.

Exit 0 = clean (relative to allowlist, or absolutely with --strict), 1 = fail.
"""
from __future__ import annotations

import argparse
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DEF_CLASS = ROOT / "configs/USA/psyq_classification.tsv"
DEF_RANGES = ROOT / "configs/USA/psyq_data_ranges.tsv"
DEF_ALLOW = ROOT / "pc_port/psyq_guard_allowlist.txt"
DEF_EXE = ROOT / "build/extracted/disc1/SLUS_006.62"
TEXT_VRAM_BIAS = 0x8000F800
SRC_EXT = (".c", ".cc", ".cpp", ".C", ".h", ".hpp", ".inc")
SKIP_DIRS = {"build", ".git", "__pycache__"}

FUNC_DEF_RE = re.compile(
    r"^[ \t]*(?:static[ \t]+|extern[ \t]+|inline[ \t]+|__attribute__\(\([^)]*\)\)[ \t]*)*"
    r"(?:[A-Za-z_][\w \t\*]*?)?\bfunc_([0-9A-Fa-f]{8})\b[ \t]*\(([^;{}()]*(?:\([^;{}()]*\)[^;{}()]*)*)\)[ \t\r\n]*"
    r"(?://[^\n]*\n[ \t\r\n]*|/\*.*?\*/[ \t\r\n]*)*\{",
    re.M | re.S,
)
DECOMP_SRC_RE = re.compile(r"decomp-source:\s*(?:\S+/)?func_([0-9A-Fa-f]{8})")


# --------------------------------------------------------------------------- inputs
def load_classification(path: Path) -> dict[int, tuple[str, str, str]]:
    """addr -> (library, confidence, name) for every Psy-Q row."""
    out: dict[int, tuple[str, str, str]] = {}
    for line in path.read_text().splitlines():
        f = line.split("\t")
        if not f or f[0] == "addr" or len(f) < 4:
            continue
        if f[3] == "-" or f[2] == "game":
            continue
        a = int(f[0].split(":")[-1], 16)
        out[a] = (f[2], f[3], f[1])
    return out


def load_ranges(path: Path) -> list[tuple[str, int, int, str, str]]:
    """name, start vram, end vram, library, source ('exe' or 'ovl_XXXX@0xBASE')."""
    rows = []
    if not path.exists():
        return rows
    for line in path.read_text().splitlines():
        if not line.strip() or line.startswith(("#", "name\t")):
            continue
        f = line.split("\t")
        rows.append((f[0], int(f[1], 16), int(f[2], 16), f[3] if len(f) > 3 else "",
                     f[4] if len(f) > 4 and f[4] else "exe"))
    return rows


def range_bytes(exe: bytes, disc_root: Path, lo: int, hi: int, source: str) -> bytes | None:
    if source == "exe":
        return exe[lo - TEXT_VRAM_BIAS: hi - TEXT_VRAM_BIAS]
    ovl, _, base = source.partition("@")
    blob = disc_root / "peimg" / f"{ovl}.bin"
    if not blob.exists() or not base:
        return None
    b = blob.read_bytes()
    base_i = int(base, 16)
    return b[lo - base_i: hi - base_i]


def load_allow(path: Path) -> set[tuple[str, str, str]]:
    out = set()
    if not path.exists():
        return out
    for line in path.read_text().splitlines():
        s = line.split("#", 1)[0].rstrip()
        if not s.strip():
            continue
        f = s.split("\t")
        if len(f) >= 3:
            out.add((f[0], f[1], f[2]))
    return out


def _git_ignored(path: Path) -> bool:
    path = path.resolve()        # `git -C dir` resolves a relative path against dir
    try:
        r = subprocess.run(["git", "-C", str(path.parent), "check-ignore", "-q", str(path)],
                           capture_output=True)
    except OSError:
        return False
    return r.returncode == 0


def iter_sources(tree: Path):
    """Source files of the tree.  A git-ignored vendor checkout directly under
    a `third_party/` directory (fetched, never committed, e.g. PsyCross) is
    skipped: it is not part of the committed tree or the build.  Only vendor
    trees are exempt -- other git-ignored sources (the generated
    game/decomp*/ TUs) ARE compiled and stay scanned -- and anything that is
    ever linked in is still caught by the --binary checks (nm symbols and
    table bytes)."""
    for root, dirs, files in os.walk(tree):
        dirs[:] = [d for d in dirs if d not in SKIP_DIRS and not d.startswith("build")]
        if Path(root).name == "third_party":
            dirs[:] = [d for d in dirs if not _git_ignored(Path(root) / d)]
        for fn in files:
            if fn.endswith(SRC_EXT):
                yield Path(root) / fn


def rel(p: Path) -> str:
    try:
        return str(p.resolve().relative_to(ROOT))
    except ValueError:
        return str(p)


# --------------------------------------------------------------------------- checks
def check_sources(tree: Path, cls) -> list[tuple[str, str, str, str]]:
    """findings: (kind, key, path, detail)"""
    out = []
    for p in iter_sources(tree):
        try:
            text = p.read_text(errors="replace")
        except OSError:
            continue
        seen = set()
        for m in FUNC_DEF_RE.finditer(text):
            a = int(m.group(1), 16)
            if a in cls and a not in seen:
                seen.add(a)
                lib, conf, _ = cls[a]
                out.append(("func", f"0x{a:08X}", rel(p), f"{lib}/{conf} defined"))
        for m in DECOMP_SRC_RE.finditer(text):
            a = int(m.group(1), 16)
            if a in cls and a not in seen:
                seen.add(a)
                lib, conf, _ = cls[a]
                out.append(("func", f"0x{a:08X}", rel(p), f"{lib}/{conf} decomp-source"))
    return out


# Game TUs (docs/ARCHITECTURE-PORT.md rule 1) may include neither PsyCross nor
# Sony Psy-Q headers, nor host/OS headers; they call pe_plat_* interfaces only.
GAME_DIRS = ("game", "bootstrap")
INCLUDE_RE = re.compile(r'^[ \t]*#[ \t]*include[ \t]*[<"]([^>"]+)[>"]', re.M)
PSYQ_HEADERS = {
    "libapi.h", "libc.h", "libcd.h", "libds.h", "libetc.h", "libgpu.h", "libgte.h",
    "libgs.h", "libspu.h", "libsnd.h", "libpress.h", "libpad.h", "libmcrd.h",
    "libcard.h", "libmath.h", "libsn.h", "kernel.h", "inline_c.h", "gtemac.h",
    "gtereg.h", "r3000.h", "asm.h", "abs.h", "rand.h", "sys/types.h_psx",
}
HOST_HEADER_RE = re.compile(r"^(SDL2?/|SDL\.h|GL/|GLES|AL/|al\.h|alc\.h|X11/|windows\.h|"
                            r"winsock|pthread\.h|unistd\.h|dlfcn\.h|sys/(mman|ioctl|socket)\.h)")
BACKEND_HEADER_RE = re.compile(r"^(host_\w+|pe_gpu|pe_spu\w*|pe_cdreg|pe_mdec|pe_disc|pe_sio0|"
                               r"pe_rcnt2|pe_timer1|pe_str_feed|pe_mmio|pe_irq\w*|pe_audio_driver|"
                               r"pe_vsync)\.h$")


def check_includes(tree: Path) -> list[tuple[str, str, str, str]]:
    out = []
    for sub in GAME_DIRS:
        base = tree / sub
        if not base.is_dir():
            continue
        for p in iter_sources(base):
            try:
                text = p.read_text(errors="replace")
            except OSError:
                continue
            for m in INCLUDE_RE.finditer(text):
                inc = m.group(1)
                leaf = inc.split("/")[-1]
                if inc.startswith(("PsyX/", "psx/")) or "PsyX" in inc or "psycross" in inc.lower():
                    out.append(("include", inc, rel(p), "psycross-header"))
                elif leaf in PSYQ_HEADERS and not inc.startswith("sys/"):
                    out.append(("include", inc, rel(p), "psyq-sdk-header"))
                elif HOST_HEADER_RE.match(inc):
                    out.append(("include", inc, rel(p), "host-header"))
                elif BACKEND_HEADER_RE.match(leaf):
                    out.append(("include", inc, rel(p), "backend-header (use pe_plat_*)"))
    return out


def nm_symbols(binary: Path) -> set[int]:
    nm = shutil.which("nm") or shutil.which("llvm-nm")
    if not nm:
        raise SystemExit("psyq_port_guard: nm not found (needed for --binary)")
    r = subprocess.run([nm, "--defined-only", str(binary)], capture_output=True, text=True)
    if r.returncode != 0 and not r.stdout:
        raise SystemExit(f"psyq_port_guard: nm failed on {binary}: {r.stderr.strip()}")
    addrs = set()
    for line in r.stdout.splitlines():
        m = re.search(r"\s[TtWw]\s+_?func_([0-9A-Fa-f]{8})\b", line)
        if m:
            addrs.add(int(m.group(1), 16))
    return addrs


def check_binary(binary: Path, cls) -> list[tuple[str, str, str, str]]:
    out = []
    for a in sorted(nm_symbols(binary)):
        if a in cls:
            lib, conf, _ = cls[a]
            out.append(("func", f"0x{a:08X}", f"bin:{binary.name}", f"{lib}/{conf} linked"))
    return out


def table_bytes(exe: bytes, lo: int, hi: int) -> bytes:
    return exe[lo - TEXT_VRAM_BIAS: hi - TEXT_VRAM_BIAS]


def check_tables_binary(binary: Path, tables) -> list[tuple[str, str, str, str]]:
    blob = binary.read_bytes()
    out = []
    for name, data in tables:
        win = 32
        # only windows with real content (>= 8 distinct byte values) count, so
        # zero/flag runs never match by accident
        wins = [data[i:i + win] for i in range(0, max(len(data) - win, 0) + 1, win)]
        wins = [w for w in wins if len(set(w)) >= 8]
        hits = sum(1 for w in wins if w in blob)
        if hits >= 2 or (len(data) <= 64 and len(set(data)) >= 8 and data in blob):
            out.append(("table", name, f"bin:{binary.name}", f"{hits} x32B windows of the table bytes"))
    return out


NUM_RE = re.compile(r"(?<![\w.])(-?0[xX][0-9A-Fa-f]+|-?\d+)[uUlL]*(?![\w.])")


def check_tables_sources(tree: Path, tables, run_len: int = 24, distinct_words: int = 24):
    out = []
    prepared = []
    for name, data in tables:
        u16 = [int.from_bytes(data[i:i + 2], "little") for i in range(0, len(data) - 1, 2)]
        u32 = {int.from_bytes(data[i:i + 4], "little") for i in range(0, len(data) - 3, 4)}
        u32 = {w for w in u32 if w >= 0x10000}  # distinctive words only
        # index of u16 run starts for quick lookup
        starts: dict[tuple[int, ...], None] = {}
        for i in range(0, len(u16) - run_len + 1):
            run = tuple(u16[i:i + run_len])
            if len(set(run)) >= 8:  # ignore low-entropy runs (0/1 flags, fills)
                starts[run] = None
        prepared.append((name, starts, u32))
    for p in iter_sources(tree):
        try:
            text = p.read_text(errors="replace")
        except OSError:
            continue
        nums = []
        for m in NUM_RE.finditer(text):
            try:
                nums.append(int(m.group(1), 0) & 0xFFFFFFFF)
            except ValueError:
                pass
        if len(nums) < distinct_words:
            continue
        numset = set(nums)
        n16 = [v & 0xFFFF for v in nums]
        for name, starts, u32 in prepared:
            hit = None
            common = len(u32 & numset)
            if common >= distinct_words:
                hit = f"{common} distinct table words as literals"
            else:
                for i in range(0, len(n16) - run_len + 1):
                    if tuple(n16[i:i + run_len]) in starts:
                        hit = f"run of >= {run_len} consecutive table values"
                        break
            if hit:
                out.append(("table", name, rel(p), hit))
    return out


# --------------------------------------------------------------------------- main
def run(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--tree", default=str(ROOT / "pc_port"))
    ap.add_argument("--classification", default=str(DEF_CLASS))
    ap.add_argument("--ranges", default=str(DEF_RANGES))
    ap.add_argument("--allowlist", default=str(DEF_ALLOW))
    ap.add_argument("--binary", action="append", default=[])
    ap.add_argument("--exe", default=str(DEF_EXE))
    ap.add_argument("--require-exe", action="store_true")
    ap.add_argument("--no-sources", action="store_true")
    ap.add_argument("--strict", action="store_true", help="ignore the allowlist (goal state)")
    ap.add_argument("--write-allowlist", action="store_true")
    ap.add_argument("--report", help="write every finding as TSV here")
    ap.add_argument("-q", "--quiet", action="store_true")
    a = ap.parse_args(argv)

    cls = load_classification(Path(a.classification))
    if not cls:
        print("psyq_port_guard: empty classification", file=sys.stderr)
        return 1
    tree = Path(a.tree)
    findings: list[tuple[str, str, str, str]] = []
    if not a.no_sources:
        findings += check_sources(tree, cls)
        findings += check_includes(tree)
    for b in a.binary:
        findings += check_binary(Path(b), cls)

    exe_path = Path(a.exe)
    tables = []
    ranges = load_ranges(Path(a.ranges))
    if ranges:
        if exe_path.exists():
            exe = exe_path.read_bytes()
            disc_root = exe_path.parent
            for n, lo, hi, _lib, src in ranges:
                data = range_bytes(exe, disc_root, lo, hi, src)
                if data:
                    tables.append((n, data))
                else:
                    print(f"psyq_port_guard: WARNING {n}: source {src} not extracted; skipped", file=sys.stderr)
        elif a.require_exe:
            print(f"psyq_port_guard: --require-exe but {exe_path} is missing", file=sys.stderr)
            return 1
        else:
            print(f"psyq_port_guard: WARNING table check skipped (no retail EXE at {exe_path}; "
                  "run scripts/extract_us.sh 1 with your own disc)", file=sys.stderr)
    if tables:
        if not a.no_sources:
            findings += check_tables_sources(tree, tables)
        for b in a.binary:
            findings += check_tables_binary(Path(b), tables)

    findings = sorted(set(findings))
    if a.report:
        with open(a.report, "w") as fh:
            fh.write("kind\tkey\twhere\tdetail\n")
            for f in findings:
                fh.write("\t".join(f) + "\n")

    keys = {(k, key, where) for k, key, where, _ in findings}
    if a.write_allowlist:
        with open(a.allowlist, "w") as fh:
            fh.write("# Psy-Q port guard allowlist -- known offenders pending replacement.\n"
                     "# This file must only SHRINK. The goal state is an empty file and\n"
                     "# `tools/analysis/psyq_port_guard.py --strict` passing.\n"
                     "# kind<TAB>key<TAB>where   (kind: func=Psy-Q function, table=Psy-Q data table)\n")
            for k in sorted(keys):
                fh.write("\t".join(k) + "\n")
        print(f"psyq_port_guard: wrote {len(keys)} allowlist entries to {a.allowlist}")
        return 0

    allow = set() if a.strict else load_allow(Path(a.allowlist))
    new = [f for f in findings if (f[0], f[1], f[2]) not in allow]
    # stale entries are only judged for the checks that actually ran
    ran_bins = {f"bin:{Path(b).name}" for b in a.binary}

    def ran(entry):
        k, _key, where = entry
        if where.startswith("bin:"):
            return where in ran_bins and (k == "func" or bool(tables))
        if a.no_sources:
            return False
        return k in ("func", "include") or bool(tables)

    stale = sorted(e for e in allow if ran(e) and e not in keys)

    from collections import Counter
    by = Counter((f[0], f[1] if f[0] == "table" else f[3].split()[0]) for f in findings)
    files = len({f[2] for f in findings})
    if not a.quiet:
        print(f"psyq_port_guard: {len(findings)} Psy-Q findings in {files} places "
              f"({'strict' if a.strict else f'{len(allow)} allowlisted'})")
        for (k, lib), n in sorted(by.items()):
            print(f"  {k:5s} {lib:22s} {n}")
    for f in new[:200]:
        print("NEW   " + "\t".join(f))
    if len(new) > 200:
        print(f"... {len(new) - 200} more")
    for e in stale:
        print("STALE " + "\t".join(e) + "   (no longer found: delete it from the allowlist)")
    if new or stale:
        print(f"psyq_port_guard: FAIL ({len(new)} new, {len(stale)} stale)")
        return 1
    print("psyq_port_guard: PASS" + (" (strict: no Psy-Q code or tables)" if a.strict else
                                     f" (relative to allowlist; {len(findings)} offenders remain)"))
    return 0


if __name__ == "__main__":
    sys.exit(run())
