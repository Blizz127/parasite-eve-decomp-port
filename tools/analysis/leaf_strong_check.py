#!/usr/bin/env python3
"""Shared strong per-leaf verification primitives.

The team's per-leaf helper (``tools/analysis/check_leaf.sh``) calls
``tools/analysis/era_link_check.py``, which compares only ``min(linked, size)``
words and therefore prints ``LINK_EXACT`` for a span that is *larger* than the
compiled object.  That hole has already hidden a real defect
(``func_800906B4``: declared ``0x68``, compiled ``0x30``; ``func_80086464``:
declared ``0x68``, compiled ``0x34``).

This module holds the **strong** leaf check used by both
``tools/build/disc1_preflight.py --deep`` and
``tools/analysis/verify_matched_leaves.py``, so there is a single definition of
"verified":

  * **size**       — compiled ``.text`` covers the declared span and every byte
                     past it is zero (the gas 16-byte alignment pad).
  * **link**       — the object, linked at the retail VMA with address-named
                     symbols bound, matches the retail EXE word-for-word over the
                     whole declared span, with zero non-zero pad past it.
  * **terminator** — the span's final instructions include a bare ``jr $ra``
                     (or a tail ``j``): a genuine function boundary.
  * **interior**   — **no** ``jr $ra`` before the terminal one.  A swallowed
                     adjacent function leaves an interior return; requiring
                     exactly one return closes the "span grew/subtracted but the
                     object grew to match" masking sub-case as far as is possible
                     without register-semantics review.

Everything here is deterministic and side-effect free apart from writing link
scripts into a temp dir.
"""

from __future__ import annotations

import re
import struct
import subprocess
import tempfile
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

LOAD_VRAM = 0x80010000
LOAD_FILE_START = 0x800
SYM_TAIL_RE = re.compile(r"([0-9A-Fa-f]{6,8})$")
ENV_LEAK_RE = re.compile(r"^(MASPSX_|ERA_)")

JR_RA = 0x03E00008


@dataclass
class LeafCheck:
    """Result of the strong check for one ``c`` leaf."""

    name: str
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

    def describe(self) -> str:
        return (
            f"{self.name} span=0x{self.span:X} vram=0x{self.vram:08X} "
            f"compiled={('0x%X' % self.compiled) if self.compiled is not None else '?'} "
            f"failures=[{','.join(self.failures()) or 'none'}]"
        )


def sanitize_environment(root: Path) -> list[str]:
    """Strip leaked per-leaf build knobs from this process's environment.

    ``disc1_build.compile_era`` copies ``os.environ`` and layers the unit's own
    knobs on top, so a leaked ``export MASPSX_SYMBOL_AT_TEMP=1`` from a
    persistent shell would silently change unrelated leaves.  Returns the names
    that were stripped so the caller can report them.
    """
    import os  # noqa: PLC0415

    leaked = sorted(k for k in os.environ if ENV_LEAK_RE.match(k))
    for key in leaked:
        del os.environ[key]
    lib = root / "tools/mipsel-host/usr/lib/x86_64-linux-gnu"
    binp = root / "tools/mipsel-host/bin"
    if binp.is_dir():
        os.environ["PATH"] = f"{binp}{os.pathsep}{os.environ.get('PATH', '')}"
    if lib.is_dir():
        existing = os.environ.get("LD_LIBRARY_PATH", "")
        os.environ["LD_LIBRARY_PATH"] = f"{lib}{os.pathsep}{existing}" if existing else str(lib)
    return leaked


def elf_section(data: bytes, want: str) -> bytes | None:
    """Return the raw bytes of a named ELF32 section, or None."""
    if data[:4] != b"\x7fELF":
        return None
    shoff = struct.unpack_from("<I", data, 32)[0]
    shentsize = struct.unpack_from("<H", data, 46)[0]
    shnum = struct.unpack_from("<H", data, 48)[0]
    shstrndx = struct.unpack_from("<H", data, 50)[0]
    if shoff == 0 or shnum == 0:
        return None
    shstr = struct.unpack_from("<I", data, shoff + shstrndx * shentsize + 16)[0]
    for i in range(shnum):
        sh = shoff + i * shentsize
        name_off = struct.unpack_from("<I", data, sh)[0]
        start = shstr + name_off
        end = data.index(b"\x00", start)
        if data[start:end].decode("ascii", "replace") != want:
            continue
        off = struct.unpack_from("<I", data, sh + 16)[0]
        size = struct.unpack_from("<I", data, sh + 20)[0]
        return data[off:off + size]
    return None


LIBCALL_ALIASES = Path(__file__).resolve().parent.parent.parent / "configs" / "USA" / "libcall_aliases.ld"
_ALIAS_RE = re.compile(r"^\s*([A-Za-z_][A-Za-z0-9_.]*)\s*=\s*(0x[0-9A-Fa-f]+)\s*;", re.M)


def libcall_aliases() -> dict:
    """Library-named routines (`__divdf3`, `memset`, ...) -> retail VMA."""
    if not LIBCALL_ALIASES.is_file():
        return {}
    text = re.sub(r"/\*.*?\*/", "", LIBCALL_ALIASES.read_text(encoding="utf-8"), flags=re.S)
    return {name: int(value, 16) for name, value in _ALIAS_RE.findall(text)}


def sym_address(name: str) -> int | None:
    """Resolve a repo address-named symbol to its retail VMA."""
    aliases = libcall_aliases()
    if name in aliases:
        return aliases[name]
    m = SYM_TAIL_RE.search(name)
    if not m:
        return None
    val = int(m.group(1), 16)
    if val < 0x80000000:
        val |= 0x80000000
    return val


def resolve_nm(root: Path) -> str:
    for candidate in (
        root / "tools/mipsel-host/bin/mipsel-linux-gnu-nm",
        root / "tools/mipsel-host/usr/bin/mipsel-linux-gnu-nm",
    ):
        if candidate.is_file():
            return str(candidate)
    import shutil  # noqa: PLC0415

    found = shutil.which("mipsel-linux-gnu-nm")
    return found or "mipsel-linux-gnu-nm"


def link_and_compare(
    obj: Path, vram: int, span: int, retail: bytes, ld: str, nm: str,
    file_offset: int | None = None,
) -> tuple[int | None, int | None, list[list[int]], str | None]:
    """Link ``obj`` at ``vram`` and compare ``span`` bytes to retail.

    Returns ``(mismatches, pad_nonzero, first_mismatch, error)``.  Unlike the
    weak per-leaf check, a declared span longer than the compiled object is
    *not* silently tolerated: the missing words are counted as mismatches (the
    caller's size check independently rejects it too).

    ``file_offset`` is where the span starts inside ``retail``; it defaults to
    the EXE mapping (``vram - 0x80010000 + 0x800``) and is passed explicitly for
    an overlay blob, whose retail bytes start at file offset 0 of its own VRAM.
    """
    try:
        undef_raw = subprocess.run(
            [nm, "-u", str(obj)], capture_output=True, text=True, check=True
        ).stdout
    except subprocess.CalledProcessError as exc:
        return None, None, [], f"nm failed: {exc}"
    undef: list[str] = []
    for line in undef_raw.splitlines():
        parts = line.split()
        if len(parts) >= 2 and parts[0] == "U":
            undef.append(parts[1])
    defsyms: list[str] = []
    unresolved: list[str] = []
    for symbol in undef:
        address = sym_address(symbol)
        if address is None:
            unresolved.append(symbol)
        else:
            defsyms += ["--defsym", f"{symbol}={address:#x}"]
    if unresolved:
        return None, None, [], f"unresolved symbols: {unresolved[:4]}"

    with tempfile.TemporaryDirectory(prefix="pe-strong-") as temporary:
        temp = Path(temporary)
        script = temp / "link.ld"
        script.write_text(
            "SECTIONS {\n"
            f"  . = {vram:#x};\n"
            "  .text : SUBALIGN(4) { *(.text) }\n"
            "  /DISCARD/ : { *(.MIPS.abiflags) *(.reginfo) *(.pdr) *(.comment) *(.note*) }\n"
            "}\n"
        )
        elf_path = temp / "x.elf"
        result = subprocess.run(
            [ld, "-T", str(script), "-o", str(elf_path), str(obj),
             "--defsym", "_gp=0x8009CD70", *defsyms],
            capture_output=True, text=True,
        )
        if result.returncode != 0:
            return None, None, [], f"ld failed: {result.stderr.strip()[:200]}"
        elf = elf_path.read_bytes()
    linked = elf_section(elf, ".text")
    if linked is None:
        return None, None, [], "no .text in linked ELF"

    off = file_offset if file_offset is not None else vram - LOAD_VRAM + LOAD_FILE_START
    rom = retail[off:off + span]
    n = min(len(rom), len(linked), span)
    mismatches = 0
    first: list[list[int]] = []
    for i in range(0, n, 4):
        rw = struct.unpack_from("<I", rom, i)[0]
        lw = struct.unpack_from("<I", linked, i)[0] if i + 4 <= len(linked) else 0
        if rw != lw:
            mismatches += 1
            if len(first) < 8:
                first.append([vram + i, rw, lw])
    if n < span:
        mismatches += (span - n) // 4
    extra = linked[span:]
    pad_nonzero = sum(
        1 for i in range(0, len(extra) - 3, 4)
        if struct.unpack_from("<I", extra, i)[0] != 0
    )
    return mismatches, pad_nonzero, first, None


def boundary_check(
    retail: bytes, vram: int, span: int, file_offset: int | None = None
) -> tuple[str, bool, int, list[int], bool]:
    """Inspect the retail words at the span boundary.

    Returns ``(terminator_label, terminator_ok, jr_ra_total, interior_offsets,
    interior_ok)``.  A genuine function ends in a bare ``jr $ra`` (with or
    without a delay-slot word after it), a tail ``j``, or an indirect tail
    ``jr $reg`` (computed goto / tail through a pointer; the delay slot may
    be the last word).  ``interior_ok`` is true iff there is no ``jr $ra``
    before the terminal one — a swallowed adjacent function would leave one.
    ``file_offset`` as in ``link_and_compare``.
    """
    base = file_offset if file_offset is not None else vram - LOAD_VRAM + LOAD_FILE_START
    if span < 8:
        # The smallest retail leaf is 0x8 (two words); anything shorter cannot
        # hold a return plus a delay slot, so treat it as trivially bounded.
        return "tiny", True, 0, [], True
    tail = struct.unpack_from("<I", retail, base + span - 4)[0]
    head = struct.unpack_from("<I", retail, base + span - 8)[0]
    if tail == JR_RA or head == JR_RA:
        label = "jr_ra"
        ok = True
        terminal_jr = True
    elif (tail >> 26) == 2 and tail != 0:
        label = "tail_j"
        ok = True
        terminal_jr = False
    elif (head >> 26) == 2 and head != 0:
        label = "tail_j"
        ok = True
        terminal_jr = False
    elif (tail & 0xFC00003F) == 0x08 or (head & 0xFC00003F) == 0x08:
        # Indirect tail: `jr $tN` / delay. Not `jr $ra` (handled above).
        label = "tail_jr"
        ok = True
        terminal_jr = False
    else:
        label = f"other:{head:08x}"
        ok = False
        terminal_jr = False
    # Interior = every word before the terminal pair; a jr $ra there means the
    # span covers more than one function.
    interior_limit = max(span - 8, 0)
    interior: list[int] = []
    for offset in range(0, interior_limit, 4):
        if struct.unpack_from("<I", retail, base + offset)[0] == JR_RA:
            interior.append(offset)
    total = len(interior) + (1 if terminal_jr else 0)
    return label, ok, total, interior, not interior


def check_object(
    obj: Path,
    unit: dict[str, Any],
    retail: bytes,
    ld: str,
    nm: str,
    *,
    allow_truncate_pad: bool = False,
) -> LeafCheck:
    """Run the full strong check on a compiled object for one plan unit."""
    result = LeafCheck(name=unit["name"], vram=unit["vram"], span=unit["size"])
    if not obj.is_file():
        result.error = "object not produced"
        return result
    text = elf_section(obj.read_bytes(), unit["primary_section"])
    if text is None:
        result.error = f"no {unit['primary_section']} section"
        return result
    result.compiled = len(text)
    span = result.span
    result.tail_nonzero = any(text[span:]) if len(text) > span else False
    # Mirror trim_elf_section_pad.py: it aborts when the target exceeds the
    # section size, or when bytes beyond the target are not all zero.
    if allow_truncate_pad:
        result.size_ok = not result.tail_nonzero
    else:
        result.size_ok = result.compiled >= span and not result.tail_nonzero

    # A plan unit's `start` is its offset in the retail file (for the EXE this
    # equals vram - 0x80010000 + 0x800; for an overlay blob it is the blob
    # offset).  Units without it keep the EXE mapping.
    file_offset = unit.get("start")
    mismatches, pad_nonzero, first, error = link_and_compare(
        obj, result.vram, span, retail, ld, nm, file_offset
    )
    if error:
        result.error = error
        return result
    result.mismatches = mismatches
    result.pad_nonzero = pad_nonzero
    result.first_mismatch = first
    result.link_ok = mismatches == 0 and pad_nonzero == 0

    label, term_ok, total, interior, interior_ok = boundary_check(
        retail, result.vram, span, file_offset
    )
    result.terminator = label
    result.terminator_ok = term_ok
    result.jr_ra_total = total
    result.interior_jr_ra = interior
    result.interior_count = len(interior)
    result.interior_ok = interior_ok
    return result


def boundary_only(unit: dict[str, Any], retail: bytes) -> LeafCheck:
    """Cheap boundary check that needs only the retail image (no compile)."""
    result = LeafCheck(name=unit["name"], vram=unit["vram"], span=unit["size"])
    label, term_ok, total, interior, interior_ok = boundary_check(
        retail, result.vram, result.span, unit.get("start")
    )
    result.terminator = label
    result.terminator_ok = term_ok
    result.jr_ra_total = total
    result.interior_jr_ra = interior
    result.interior_count = len(interior)
    result.interior_ok = interior_ok
    return result


def main(argv: list[str] | None = None) -> int:
    """Standalone strong check for one or more YAML ``c`` leaves."""
    import argparse  # noqa: PLC0415
    import json  # noqa: PLC0415
    import sys  # noqa: PLC0415

    this_dir = Path(__file__).resolve().parent
    root = this_dir.parents[1]
    for candidate in (root / "tools/build", root / "tools/analysis"):
        if str(candidate) not in sys.path:
            sys.path.insert(0, str(candidate))
    from disc1_plan import PlanError, build_plan  # noqa: PLC0415

    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("func", nargs="+", help="YAML C symbol(s), e.g. func_800906B4")
    parser.add_argument("--root", type=Path, default=root)
    parser.add_argument("--config", type=Path, default=Path("configs/USA/disc1.yaml"))
    parser.add_argument(
        "--profiles", type=Path, default=Path("configs/USA/disc1_build_profiles.json")
    )
    parser.add_argument(
        "--source-override",
        action="append",
        default=[],
        metavar="FUNC=PATH",
        help="compile PATH instead of the leaf's src/FUNC.c (repeatable)",
    )
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args(argv)
    overrides: dict[str, Path] = {}
    for value in args.source_override:
        name, sep, path = value.partition("=")
        resolved = Path(path).resolve()
        if not sep or not name or not resolved.is_file():
            print(f"leaf_strong_check: bad --source-override {value!r}", file=sys.stderr)
            return 2
        overrides[name] = resolved

    root = args.root.resolve()
    sanitize_environment(root)
    retail_path = root / "build/extracted/disc1/SLUS_006.62"
    if not retail_path.is_file():
        print(f"leaf_strong_check: missing retail EXE {retail_path}", file=sys.stderr)
        return 2
    retail = retail_path.read_bytes()
    try:
        plan = build_plan(root=root, config=args.config, profiles_path=args.profiles)
    except PlanError as exc:
        print(f"leaf_strong_check: plan error: {exc}", file=sys.stderr)
        return 2

    import disc1_build  # noqa: PLC0415
    import shutil  # noqa: PLC0415
    import tempfile as _tempfile  # noqa: PLC0415

    retail_path = root / "build/extracted/disc1/SLUS_006.62"
    tools = disc1_build.find_toolchain()
    ld = tools.linker
    nm = resolve_nm(root)
    units_by_name = {u["name"]: u for u in plan["units"] if u["kind"] == "c"}
    missing = [name for name in args.func if name not in units_by_name]
    if missing:
        print(f"leaf_strong_check: not YAML C spans: {missing}", file=sys.stderr)
        return 2

    (root / "build").mkdir(parents=True, exist_ok=True)
    scratch = Path(_tempfile.mkdtemp(prefix="leaf-strong-", dir=(root / "build")))
    selected = [units_by_name[name] for name in args.func]
    unknown = set(overrides) - set(args.func)
    if unknown:
        print(f"leaf_strong_check: --source-override for unchecked {sorted(unknown)}",
              file=sys.stderr)
        return 2
    selected = [
        dict(u, source=str(overrides[u["name"]])) if u["name"] in overrides else u
        for u in selected
    ]
    for name, path in sorted(overrides.items()):
        print(f"leaf_strong_check: NOTE {name} compiled from override {path}")
    build_units = json.loads(json.dumps(selected))
    for unit in build_units:
        unit["object"] = str(scratch / Path(unit["object"]).name)
    try:
        disc1_build.compile_all({"units": build_units, "header": plan["header"]}, tools)

        results = [
            check_object(scratch / Path(u["object"]).name, u, retail, ld, nm)
            for u in selected
        ]
    finally:
        shutil.rmtree(scratch, ignore_errors=True)
    if args.json:
        from dataclasses import asdict  # noqa: PLC0415

        print(json.dumps([asdict(r) for r in results], indent=2))
    ok = True
    for result in results:
        if result.ok:
            print(f"LEAF_STRONG=EXACT {result.describe()}")
        else:
            ok = False
            print(f"LEAF_STRONG=FAIL {result.describe()}")
            if result.error:
                print(f"  error: {result.error}")
            if result.mismatches:
                print(f"  word mismatches={result.mismatches} pad_nonzero={result.pad_nonzero}")
                for addr, rw, lw in result.first_mismatch:
                    print(f"    {addr:#x}: ROM {rw:08x}  LNK {lw:08x}")
            if not result.size_ok:
                print(f"  size: declared span 0x{result.span:X} vs compiled "
                      f"{('0x%X' % result.compiled) if result.compiled is not None else '?'}")
            if not result.terminator_ok:
                print(f"  terminator: {result.terminator}")
            if not result.interior_ok:
                print(f"  interior jr $ra offsets: {[hex(o) for o in result.interior_jr_ra]}")
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
