#!/usr/bin/env python3
"""Build the USA Disc 1 executable (or a PE.IMG overlay) from the YAML-derived plan.

The default target is the Disc 1 executable exactly as before; ``--target <id>``
builds an overlay described in ``configs/USA/overlays/manifest.yaml`` into
``build/overlays/<id>/candidate.bin`` and compares it with the extracted retail
blob.  Every path and geometry decision comes from the plan's target
descriptor (``disc1_plan.plan_target``), never from a second copy of the EXE
constants.
"""

from __future__ import annotations

import argparse
import hashlib
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Sequence

from disc1_plan import PlanError, build_plan, plan_target, write_generated
from overlay_targets import MAIN_TARGET, Target, TargetError, load_target


ROOT = Path(__file__).resolve().parents[2]
GENERATED = Path(MAIN_TARGET.generated_dir)
EXE = Path(MAIN_TARGET.retail)
ERA_CPP = Path("tools/era/gcc-2.7.2-psx/cpp")
ERA_CC1 = Path("tools/era/gcc-2.7.2-psx/cc1")
ERA_CC1_VER_DEFAULT = "2.7.2"
MASPSX = Path("tools/era/maspsx/maspsx.py")
TRIM = Path("tools/trim_elf_section_pad.py")
AS_FLAGS = ["-EL", "-mips1", "-mabi=32"]
MODERN_C_FLAGS = [
    "-EL",
    "-mips1",
    "-mfp32",
    "-mabi=32",
    "-G0",
    "-fno-pic",
    "-mno-abicalls",
    "-ffreestanding",
    "-fno-builtin",
    "-O1",
]


# Retail SLUS_006.62 was assembled with ASPSX >= 2.30 throughout (1246
# signature sites for 2.30 behaviour, 0 for 2.21 -- see
# docs/ai_context/TOOLCHAIN_REBUILD.md).  A profile's ERA_ASPSX_VER still
# overrides this per leaf.
ERA_ASPSX_VER_DEFAULT = "2.30"


def maspsx_sdata_flag(flags: list[str]) -> str:
    """Return cc1's `-G<n>` small-data limit as the `-G<n>` maspsx expects.

    maspsx must know the same limit cc1 used: a `.extern SYM, size` whose size
    fits the limit resolves gp-relative (one instruction), and the MIPS-I
    load-delay nop before such a store is only emitted when maspsx can see
    that.  A missing `-G` means `-G0` (nothing gp-relative).
    """
    limit = "0"
    it = iter(flags)
    for flag in it:
        if flag == "-G":
            limit = next(it, "0")
        elif flag.startswith("-G") and flag[2:].isdigit():
            limit = flag[2:]
    return f"-G{limit}"


class BuildError(RuntimeError):
    """The generated build could not be completed or did not match retail."""


@dataclass(frozen=True)
class Toolchain:
    runner: tuple[str, ...]
    assembler: str
    linker: str
    objcopy: str
    readelf: str
    compiler: str
    note: str

    def command(self, tool: str, *arguments: str) -> list[str]:
        return [*self.runner, tool, *arguments]


def step(label: str) -> None:
    print(f"\n=== {label} ===", flush=True)


def info(message: str) -> None:
    print(f"  {message}", flush=True)


def run(
    command: Sequence[str],
    *,
    cwd: Path = ROOT,
    env: dict[str, str] | None = None,
    stdin: Any = None,
    stdout: Any = None,
    stderr: Any = None,
    check: bool = True,
) -> subprocess.CompletedProcess[Any]:
    try:
        return subprocess.run(
            list(command),
            cwd=cwd,
            env=env,
            stdin=stdin,
            stdout=stdout,
            stderr=stderr,
            check=check,
        )
    except FileNotFoundError as exc:
        raise BuildError(f"command not found: {command[0]}") from exc
    except subprocess.CalledProcessError as exc:
        raise BuildError(
            f"command failed ({exc.returncode}): {' '.join(command)}"
        ) from exc


def first_version_line(command: Sequence[str]) -> str:
    result = run(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
        check=False,
    )
    text = result.stdout.decode(errors="replace") if result.stdout else ""
    return text.splitlines()[0] if text.splitlines() else "unknown"


def find_toolchain() -> Toolchain:
    names = {
        "assembler": "mipsel-linux-gnu-as",
        "linker": "mipsel-linux-gnu-ld",
        "objcopy": "mipsel-linux-gnu-objcopy",
        "readelf": "mipsel-linux-gnu-readelf",
        "compiler": "mipsel-linux-gnu-gcc",
    }
    resolved = {key: shutil.which(value) for key, value in names.items()}
    if all(resolved.values()):
        return Toolchain(
            runner=(),
            note="host PATH",
            **{key: str(value) for key, value in resolved.items()},
        )

    # Tree-local unprivileged install (scripts/setup_mipsel_host.sh); its bin/
    # shims carry the LD_LIBRARY_PATH the relocated Debian cross libs need.
    local_bin = ROOT / "tools/mipsel-host/bin"
    if local_bin.is_dir():
        os.environ["PATH"] = f"{local_bin}{os.pathsep}{os.environ.get('PATH', '')}"
        resolved = {key: shutil.which(value) for key, value in names.items()}
        if all(resolved.values()):
            return Toolchain(
                runner=(),
                note="repo-local tools/mipsel-host/bin",
                **{key: str(value) for key, value in resolved.items()},
            )

    distrobox = shutil.which("distrobox")
    if distrobox:
        listed = run(
            [distrobox, "list"],
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            check=False,
        )
        if b"pe-mipsel" in (listed.stdout or b""):
            return Toolchain(
                runner=(distrobox, "enter", "pe-mipsel", "--"),
                note="distrobox pe-mipsel",
                **names,
            )
    raise BuildError(
        "mipsel-linux-gnu toolchain not found; run inside pe-mipsel-img, "
        "install binutils-mipsel-linux-gnu and gcc-mipsel-linux-gnu, or run "
        "scripts/setup_mipsel_host.sh for a rootless tree-local install"
    )


def era_compiler(environment: dict[str, str]) -> tuple[Path, Path]:
    """Resolve the (cpp, cc1) pair for a leaf from its profile ``environment``.

    ``ERA_CC1_VER`` selects the stock decompals/old-gcc release directory
    ``tools/era/gcc-<ver>-psx`` (2.7.2 by default, 2.8.1 for the leaves whose
    delay slots only 2.8.1 fills). It is read from the unit's own environment
    block exactly like ``ERA_ASPSX_VER`` so the SHA-1 gate, the deep preflight
    and ``era_link_check.py`` all pick the same compiler. Paths are ROOT-relative.
    """
    version = environment.get("ERA_CC1_VER", ERA_CC1_VER_DEFAULT)
    if version == ERA_CC1_VER_DEFAULT:
        return ERA_CPP, ERA_CC1
    directory = Path(f"tools/era/gcc-{version}-psx")
    return directory / "cpp", directory / "cc1"


def require_inputs(plan: dict[str, Any]) -> None:
    if not (ROOT / "CLAUDE.md").is_file():
        raise BuildError(f"not a repository root: {ROOT}")
    target = plan_target(plan)
    exe = ROOT / target.retail
    if not exe.is_file():
        remedy = "scripts/extract_us.sh 1" if target.is_main else "scripts/extract_overlays.sh"
        raise BuildError(f"missing {target.retail}; run {remedy}")
    actual = hashlib.sha1(exe.read_bytes()).hexdigest()
    if actual != plan["expected_sha1"]:
        raise BuildError(
            f"retail {'executable' if target.is_main else 'overlay'} SHA-1 {actual} != {plan['expected_sha1']}"
        )
    for path in (ERA_CPP, ERA_CC1, MASPSX, TRIM):
        if not (ROOT / path).is_file():
            raise BuildError(f"missing required tool: {path}")
    if not os.access(ROOT / ERA_CPP, os.X_OK) or not os.access(ROOT / ERA_CC1, os.X_OK):
        raise BuildError("era cpp/cc1 are not executable; run scripts/setup_era.sh")
    # Every non-default ERA_CC1_VER a leaf's profile names must be installed
    # too, or the SHA-1 gate would fail mid-run on that leaf.
    for unit in plan["units"]:
        if unit["kind"] != "c" or unit["toolchain"] != "era":
            continue
        for path in era_compiler(unit["environment"]):
            if not (ROOT / path).is_file() or not os.access(ROOT / path, os.X_OK):
                raise BuildError(
                    f"{unit['name']}: ERA_CC1_VER="
                    f"{unit['environment'].get('ERA_CC1_VER')} needs {path}; "
                    "run scripts/setup_era.sh"
                )
    info(f"OK original {'EXE' if target.is_main else target.id} SHA-1 {actual}")
    info(
        f"OK YAML plan: {plan['counts']['units']} spans, "
        f"{plan['counts']['c']} C leaves, geometry 0x{plan['file']['load_size']:X}"
    )


def absolutize_rodata(plan: dict[str, Any]) -> None:
    pattern = re.compile(r"(\.word\s+)\.L([0-9A-Fa-f]{8})(?=\s|$)")
    total = 0
    for unit in plan["units"]:
        if unit["kind"] != "rodata":
            continue
        path = ROOT / unit["source"]
        source = path.read_text(encoding="utf-8")

        def replace(match: re.Match[str]) -> str:
            nonlocal total
            address = int(match.group(2), 16)
            if not 0x80010000 <= address < 0x80200000:
                return match.group(0)
            total += 1
            return match.group(1) + "0x" + match.group(2)

        updated = pattern.sub(replace, source)
        if updated != source:
            path.write_text(updated, encoding="utf-8")
    info(f"rodata absolutize: {total} label refs -> literal")


def assemble_bin(unit: dict[str, Any], tools: Toolchain) -> None:
    """Wrap a raw `bin` span (splat asset) in a .incbin unit so it links verbatim."""
    source = ROOT / unit["source"]
    if not source.is_file():
        raise BuildError(f"missing split bin span {unit['source']}")
    wrapper = ROOT / Path(unit["object"]).with_suffix(".s")
    wrapper.write_text(
        "/* Generated by disc1_build.py: raw span, bytes come from the split. */\n"
        f"\t.section {unit['primary_section']}\n"
        f'\t.incbin "{source}"\n',
        encoding="utf-8",
    )
    run(
        tools.command(
            tools.assembler,
            *AS_FLAGS,
            "-o",
            unit["object"],
            str(wrapper),
        )
    )


def assemble_all(plan: dict[str, Any], tools: Toolchain) -> None:
    target = plan_target(plan)
    (ROOT / f"build/{target.asm_dir}/data").mkdir(parents=True, exist_ok=True)
    (ROOT / f"build/{target.src_dir}").mkdir(parents=True, exist_ok=True)
    header = plan.get("header")
    if header:
        run(
            tools.command(
                tools.assembler,
                *AS_FLAGS,
                "-I",
                str(ROOT / "include"),
                "-o",
                header["object"],
                header["source"],
            )
        )
    absolutize_rodata(plan)
    for unit in plan["units"]:
        if unit["kind"] == "c":
            continue
        Path(unit["object"]).parent.mkdir(parents=True, exist_ok=True)
        if unit["kind"] == "bin":
            assemble_bin(unit, tools)
            continue
        run(
            tools.command(
                tools.assembler,
                *AS_FLAGS,
                "-I",
                str(ROOT / "include"),
                "-o",
                unit["object"],
                unit["source"],
            )
        )
    assembled = sum(1 for unit in plan["units"] if unit["kind"] != "c") + (1 if header else 0)
    info(f"assembled {assembled} units")


def force_absolute_symbols(assembly: Path, specification: str) -> None:
    text = assembly.read_text(encoding="utf-8")
    for symbol in specification.split(","):
        text = re.sub(
            rf"\t\.extern\t{re.escape(symbol)}, \d+\n",
            "",
            text,
        )
    assembly.write_text(text, encoding="utf-8")


def dispatch_rodata_pad_ok(rodata_size: int, expected_size: int, tail: bytes) -> bool:
    """Decide whether a compiled dispatch table's `.rodata` is the pool table.

    cc1 emits the switch table with `.align 3`, so a table with an ODD word
    count carries four bytes of zero alignment padding after it: a 0x5C pool
    table lands as a 0x60 section.  The strip zeroes `sh_size`, so those bytes
    never reach the image; refusing them merely blocks the odd-word tables in
    `asm/disc1/data/*.rodata.s`.

    The tolerance is deliberately exact rather than a range.  Identity is
    already independently proven by the `.rel.rodata` relocation count
    (`words != len(literals)` is checked separately) and ultimately by the
    whole-image SHA-1, so accepting only "exact size" or "exactly 4 trailing
    zero bytes, and only where alignment actually requires them" cannot hide a
    differently shaped table.

    Diagnoses from the merge that motivated this (2026-09-22):
      jtbl_80010080: pool 7 literals (0x1C), object 0x20 -- 8th word is the
        symbol-referenced null default `.L00000000_main`, i.e. zero padding.
      jtbl_800C2128: pool 5 literals (0x14), object 0x18 -- 5-case switch with
        no default; the 4 extra bytes are `.align 3` padding.
    """
    if rodata_size == expected_size:
        return True
    # `.align 3` only adds padding when the table is not already 8-byte sized.
    if expected_size % 8 == 0:
        return False
    return (
        rodata_size == expected_size + 4
        and len(tail) == 4
        and tail == b"\x00\x00\x00\x00"
    )


def dispatch_multi_layout(
    offsets: list[int], block_counts: list[int], rodata: bytes, symbol: str
) -> list[int]:
    """Expected .rel.rodata offsets for a multi-table dispatch fold.

    cc1 emits one jump table per switch, back to back in .rodata; the i-th
    table holds exactly the i-th pool block's code-address entries. Between
    tables (and after the last) the assembler may insert one zero word of
    `.align 3` padding when the preceding table ends 4 mod 8 -- the same
    padding retail's pool shows as a `.word .L00000000_main` null entry
    (jtbl_800120FC/jtbl_8001211C: 7 entries + one zero word each). Any other
    gap, a non-zero pad, or a size mismatch refuses the strip. Limit: two
    unpadded tables carry no boundary in .rodata, so their split (and their
    order, when the sizes agree) is proven by the link/SHA-1 check -- each
    dispatch's `sltiu` bound and folded pool symbol are fixed in .text.
    """
    expected: list[int] = []
    cursor = 0
    for block, count in enumerate(block_counts):
        first = len(expected)
        start = offsets[first] if first < len(offsets) else cursor
        gap = start - cursor
        if block == 0 and gap != 0:
            raise BuildError(f"first dispatch table not at .rodata 0 for {symbol}")
        if gap not in (0, 4) or (gap == 4 and cursor % 8 != 4):
            raise BuildError(
                f"dispatch table {block} at .rodata 0x{start:X} does not follow "
                f"table {block - 1} (end 0x{cursor:X}) for {symbol}"
            )
        if rodata[cursor:start] != bytes(gap):
            raise BuildError(f"non-zero pad before dispatch table {block} for {symbol}")
        expected.extend(start + 4 * index for index in range(count))
        cursor = start + 4 * count
    tail = rodata[cursor:]
    if not dispatch_rodata_pad_ok(len(rodata), cursor, tail):
        raise BuildError(
            f".rodata 0x{len(rodata):X} != pool tables 0x{cursor:X} "
            f"for {symbol}; refusing to strip"
        )
    return expected


def pool_block_literals(pool_source: str, name: str) -> list[str]:
    """Return the jump-table entries of pool block ``name`` (``dlabel``..``enddlabel``).

    EXE pools are hex literals; an overlay pool whose targets sit in the same
    segment is emitted by splat as ``.word .L<vram>`` labels.  A trailing zero
    word is the ``.align 3`` pad splat folds into the preceding dlabel (seen in
    PE.IMG room headers); a jump-table entry is a code address and is never
    zero, so it cannot be an entry.  Interior zeros are kept.
    """
    block = re.search(
        rf"dlabel {re.escape(name)}\n(.*?)enddlabel {re.escape(name)}",
        pool_source,
        re.DOTALL,
    )
    if not block:
        raise BuildError(f"pool source has no {name} block")
    block_literals = re.findall(
        r"\.word\s+(0x[0-9A-Fa-f]+|\.L[0-9A-Fa-f]{8})\b", block.group(1)
    )
    while block_literals and block_literals[-1] == "0x00000000":
        block_literals.pop()
    if len(block_literals) < 2:
        raise BuildError(f"{name} pool block has no literal words")
    return block_literals


def rodata_fold_blocks(pool_source: str, symbols: list[str]) -> list[bytes]:
    """Bytes of the pool blocks named by MASPSX_RODATA_FOLD, in list order."""
    blocks: list[bytes] = []
    for name in symbols:
        block = re.search(
            rf"dlabel {re.escape(name)}\n(.*?)enddlabel {re.escape(name)}",
            pool_source,
            re.DOTALL,
        )
        if not block:
            raise BuildError(f"pool source has no {name} block")
        words = re.findall(r"\.word\s+(0x[0-9A-Fa-f]+)\b", block.group(1))
        if not words or len(words) != block.group(1).count(".word"):
            raise BuildError(f"{name} pool block is not plain .word literals")
        blocks.append(b"".join(struct.pack("<I", int(w, 16)) for w in words))
    return blocks


def fold_region_matches(region: bytes, blocks: list[bytes]) -> bool:
    """True when ``region`` (an object's cc1 literal bytes) is the pool image.

    cc1 pads each literal to 4 bytes like the pool blocks; the only slack is
    trailing zero pad -- in the last pool block (the object may end first), or
    up to 4 zero bytes after the literals when an `.align 3` table follows.
    """
    expected = b"".join(blocks)
    if not blocks or not region:
        return False
    if len(region) >= len(expected):
        return (
            region[: len(expected)] == expected
            and not any(region[len(expected):])
            and len(region) - len(expected) < 8
        )
    return (
        region == expected[: len(region)]
        and not any(expected[len(region):])
        and len(region) > len(expected) - len(blocks[-1])
    )


def strip_dispatch_rodata(
    object_path: Path,
    symbol: str,
    pool_dir: Path = ROOT / "asm/disc1/data",
    fold_spec: str | None = None,
) -> None:
    # MASPSX_DISPATCH_FOLD may name several pool tables (comma list, in the
    # order cc1 emits its switch tables into .rodata); the object's .rodata
    # must then be exactly those blocks back to back.
    symbols = [part.strip() for part in symbol.split(",") if part.strip()]
    if not symbols:
        raise BuildError(f"MASPSX_DISPATCH_FOLD {symbol!r} names no table")
    for name in symbols:
        if not re.fullmatch(r"jtbl_[0-9A-Fa-f]{8}", name):
            raise BuildError(
                f"MASPSX_DISPATCH_FOLD {name!r} is not jtbl_<vram-hex>"
            )
    pool_source = "".join(
        path.read_text(encoding="utf-8")
        for path in sorted(pool_dir.glob("*.rodata.s"))
    )
    literals: list[str] = []
    block_counts: list[int] = []
    for name in symbols:
        block_literals = pool_block_literals(pool_source, name)
        literals.extend(block_literals)
        block_counts.append(len(block_literals))
    expected_size = len(literals) * 4

    data = bytearray(object_path.read_bytes())
    if data[:4] != b"\x7fELF":
        raise BuildError(f"{object_path}: not ELF")
    e_shoff = struct.unpack_from("<I", data, 32)[0]
    e_shentsize = struct.unpack_from("<H", data, 46)[0]
    e_shnum = struct.unpack_from("<H", data, 48)[0]
    e_shstrndx = struct.unpack_from("<H", data, 50)[0]
    shstr_off = struct.unpack_from(
        "<I", data, e_shoff + e_shstrndx * e_shentsize + 16
    )[0]

    def header(index: int) -> int:
        return e_shoff + index * e_shentsize

    def section_name(index: int) -> str:
        offset = struct.unpack_from("<I", data, header(index))[0]
        end = data.index(b"\x00", shstr_off + offset)
        return data[shstr_off + offset : end].decode()

    rodata_index = next(
        (index for index in range(e_shnum) if section_name(index) == ".rodata"),
        None,
    )
    if rodata_index is None:
        raise BuildError("compiled switch leaf has no .rodata")
    rodata_header = header(rodata_index)
    rodata_size = struct.unpack_from("<I", data, rodata_header + 20)[0]
    rodata_offset = struct.unpack_from("<I", data, rodata_header + 16)[0]
    # Combined MASPSX_DISPATCH_FOLD + MASPSX_RODATA_FOLD: the object's .rodata
    # is cc1's `$LC` literals and switch tables in emission order. Only the two
    # shapes cc1 produces are accepted -- literals, zero pad to 8, tables; or
    # tables, zero pad to 4, literals -- the literal part must be the pool
    # image of the fold list, and the table part must pass every dispatch
    # check below on its own (viewed as if it started at .rodata 0).
    table_base = 0
    table_size = rodata_size
    fold_region = b""
    if fold_spec is not None:
        fold_symbols = [x.strip() for x in fold_spec.split(",") if x.strip()]
        if not fold_symbols:
            raise BuildError(f"MASPSX_RODATA_FOLD {fold_spec!r} names no symbol")
        fold_blocks = rodata_fold_blocks(pool_source, fold_symbols)
        whole = bytes(data[rodata_offset : rodata_offset + rodata_size])
        relocation_probe = next(
            (i for i in range(e_shnum) if section_name(i) == ".rel.rodata"), None
        )
        if relocation_probe is None:
            raise BuildError("no .rel.rodata; cannot place the dispatch table")
        probe_offset, probe_size = struct.unpack_from(
            "<II", data, header(relocation_probe) + 16
        )
        reloc_at = [
            struct.unpack_from("<I", data, probe_offset + 8 * k)[0]
            for k in range(probe_size // 8)
        ]
        if not reloc_at:
            raise BuildError("dispatch fold object has no table relocations")
        first, last = min(reloc_at), max(reloc_at) + 4
        layouts = []
        if first > 0 and first % 8 == 0:
            layouts.append(("literals-first", first, rodata_size - first, whole[:first]))
        if first == 0:
            fold_start = (last + 3) & ~3
            layouts.append(("tables-first", 0, fold_start, whole[fold_start:]))
        chosen = None
        for label, base, size, region in layouts:
            if fold_region_matches(region, fold_blocks):
                chosen = (label, base, size, region)
                break
        if chosen is None:
            raise BuildError(
                f".rodata 0x{rodata_size:X} is not dispatch tables {symbol} + "
                f"fold literals {fold_spec} in either order; refusing to strip"
            )
        _, table_base, table_size, fold_region = chosen
    tail = bytes(
        data[rodata_offset + table_base + expected_size :
             rodata_offset + table_base + table_size]
    )
    if len(symbols) == 1 and not dispatch_rodata_pad_ok(
        table_size, expected_size, tail
    ):
        raise BuildError(
            f".rodata 0x{table_size:X} != pool table 0x{expected_size:X} "
            f"for {symbol}; refusing to strip"
        )

    text_index = next(
        (index for index in range(e_shnum) if section_name(index) == ".text"),
        None,
    )
    if text_index is None:
        raise BuildError("compiled switch leaf has no .text")
    text_size = struct.unpack_from("<I", data, header(text_index) + 20)[0]
    relocation_index = next(
        (
            index
            for index in range(e_shnum)
            if section_name(index) == ".rel.rodata"
        ),
        None,
    )
    if relocation_index is None:
        raise BuildError("no .rel.rodata; cannot prove dispatch table identity")
    relocation_header = header(relocation_index)
    relocation_offset = struct.unpack_from("<I", data, relocation_header + 16)[0]
    relocation_size = struct.unpack_from("<I", data, relocation_header + 20)[0]
    symbol_table_index = struct.unpack_from("<I", data, relocation_header + 24)[0]
    symbol_offset = struct.unpack_from(
        "<I", data, header(symbol_table_index) + 16
    )[0]
    words = relocation_size // 8
    if words != len(literals):
        raise BuildError(
            f"{words} dispatch relocs != {len(literals)} pool words for {symbol}"
        )
    if len(symbols) == 1:
        expected_offsets = [table_base + index * 4 for index in range(words)]
    else:
        expected_offsets = [
            table_base + offset
            for offset in dispatch_multi_layout(
                [
                    struct.unpack_from("<I", data, relocation_offset + index * 8)[0]
                    - table_base
                    for index in range(words)
                ],
                block_counts,
                bytes(
                    data[rodata_offset + table_base :
                         rodata_offset + table_base + table_size]
                ),
                symbol,
            )
        ]
    for index in range(words):
        relocation_at, relocation_info = struct.unpack_from(
            "<II", data, relocation_offset + index * 8
        )
        if relocation_at != expected_offsets[index] or relocation_info & 0xFF != 2:
            raise BuildError(f"dispatch relocation {index} has unexpected shape")
        symbol_index = relocation_info >> 8
        symbol_value = struct.unpack_from(
            "<I", data, symbol_offset + symbol_index * 16 + 4
        )[0]
        symbol_section = struct.unpack_from(
            "<H", data, symbol_offset + symbol_index * 16 + 14
        )[0]
        if symbol_section != text_index or symbol_value >= text_size:
            raise BuildError(
                f"dispatch relocation {index} does not target local .text"
            )

    struct.pack_into("<I", data, rodata_header + 20, 0)
    struct.pack_into("<I", data, relocation_header + 20, 0)

    file_length = len(data)
    for index in range(e_shnum):
        section_header = header(index)
        offset, size = struct.unpack_from("<II", data, section_header + 16)
        section_type = struct.unpack_from("<I", data, section_header + 4)[0]
        if section_type != 8 and offset + size > file_length:
            raise BuildError(
                f"post-strip section {index} ({section_name(index)}) is out of bounds"
            )
        if section_type == 9:
            target = struct.unpack_from("<I", data, section_header + 28)[0]
            if target == rodata_index and size:
                raise BuildError("post-strip relocation still targets .rodata")

    if fold_spec is not None:
        # every $LC operand must have been retargeted to its pool symbol
        for index in range(e_shnum):
            section_header = header(index)
            if struct.unpack_from("<I", data, section_header + 4)[0] != 9:
                continue
            rel_offset, rel_size = struct.unpack_from("<II", data, section_header + 16)
            rel_symtab = struct.unpack_from("<I", data, section_header + 24)[0]
            rel_symbols = struct.unpack_from("<I", data, header(rel_symtab) + 16)[0]
            for k in range(rel_size // 8):
                rel_info = struct.unpack_from("<I", data, rel_offset + 8 * k + 4)[0]
                shndx = struct.unpack_from(
                    "<H", data, rel_symbols + (rel_info >> 8) * 16 + 14
                )[0]
                if shndx == rodata_index:
                    raise BuildError(
                        f"{section_name(index)} still references .rodata; "
                        "MASPSX_RODATA_FOLD did not retarget every $LC operand"
                    )

    symbol_table_header = header(symbol_table_index)
    symbol_offset_2 = struct.unpack_from("<I", data, symbol_table_header + 16)[0]
    symbol_size = struct.unpack_from("<I", data, symbol_table_header + 20)[0]
    if symbol_offset_2 + symbol_size > file_length:
        raise BuildError("post-strip symbol table is out of bounds")
    for index in range(symbol_size // 16):
        section_index = struct.unpack_from(
            "<H", data, symbol_offset_2 + index * 16 + 14
        )[0]
        if section_index >= e_shnum and section_index not in (0, 0xFFF1, 0xFFF2):
            raise BuildError(
                f"post-strip symbol {index} has bad shndx 0x{section_index:X}"
            )
    object_path.write_bytes(data)
    info(
        f"dispatch dedup: stripped {rodata_size}-byte duplicate table"
        + (f" + fold literals ({fold_spec})" if fold_spec is not None else "")
        + f" from {object_path.relative_to(ROOT)}"
    )


def strip_rodata_fold(
    object_path: Path, symbols_spec: str, pool_dir: Path = ROOT / "asm/disc1/data"
) -> None:
    """Drop a leaf's duplicate .rodata literals after MASPSX_RODATA_FOLD.

    maspsx (patch 20) retargets every `$LC<n>` operand to the retail pool
    symbol, so nothing in .text refers to the object's own .rodata any more.
    The section is stripped only after proving it byte-identical to the pool
    blocks named by the fold list (in order, each block padded to 4 bytes the
    way cc1's `.align 2` pads the literal), and that no relocation targets
    it -- a wrong list or a mistyped initializer fails the build loudly.
    """
    symbols = [part.strip() for part in symbols_spec.split(",") if part.strip()]
    if not symbols:
        raise BuildError(f"MASPSX_RODATA_FOLD {symbols_spec!r} names no symbol")
    pool_source = "".join(
        path.read_text(encoding="utf-8")
        for path in sorted(pool_dir.glob("*.rodata.s"))
    )
    blocks: list[bytes] = []
    for name in symbols:
        block = re.search(
            rf"dlabel {re.escape(name)}\n(.*?)enddlabel {re.escape(name)}",
            pool_source,
            re.DOTALL,
        )
        if not block:
            raise BuildError(f"pool source has no {name} block")
        words = re.findall(r"\.word\s+(0x[0-9A-Fa-f]+)\b", block.group(1))
        if not words or len(words) != block.group(1).count(".word"):
            raise BuildError(f"{name} pool block is not plain .word literals")
        blocks.append(b"".join(struct.pack("<I", int(w, 16)) for w in words))

    data = bytearray(object_path.read_bytes())
    if data[:4] != b"\x7fELF":
        raise BuildError(f"{object_path}: not ELF")
    e_shoff = struct.unpack_from("<I", data, 32)[0]
    e_shentsize = struct.unpack_from("<H", data, 46)[0]
    e_shnum = struct.unpack_from("<H", data, 48)[0]
    e_shstrndx = struct.unpack_from("<H", data, 50)[0]
    shstr_off = struct.unpack_from(
        "<I", data, e_shoff + e_shstrndx * e_shentsize + 16
    )[0]

    def header(index: int) -> int:
        return e_shoff + index * e_shentsize

    def section_name(index: int) -> str:
        offset = struct.unpack_from("<I", data, header(index))[0]
        end = data.index(b"\x00", shstr_off + offset)
        return data[shstr_off + offset : end].decode()

    names = {section_name(index): index for index in range(e_shnum)}
    if ".rodata" not in names:
        raise BuildError("MASPSX_RODATA_FOLD leaf has no .rodata")
    if ".rel.rodata" in names:
        raise BuildError("MASPSX_RODATA_FOLD .rodata carries relocations; refusing")
    rodata_index = names[".rodata"]
    rodata_header = header(rodata_index)
    rodata_offset, rodata_size = struct.unpack_from("<II", data, rodata_header + 16)
    rodata = bytes(data[rodata_offset : rodata_offset + rodata_size])
    # cc1 pads each literal to 4 bytes (`.align 2`), exactly like the pool
    # blocks; the only slack allowed is trailing zero pad in the last block.
    expected = b"".join(blocks)
    if (
        rodata_size == 0
        or rodata != expected[:rodata_size]
        or any(expected[rodata_size:])
        or rodata_size < len(expected) - len(blocks[-1]) + 1
    ):
        raise BuildError(
            f".rodata (0x{rodata_size:X} bytes) is not the pool image of "
            f"{symbols_spec} (0x{len(expected):X} bytes); refusing to strip"
        )
    for index in range(e_shnum):
        section_header = header(index)
        if struct.unpack_from("<I", data, section_header + 4)[0] != 9:
            continue
        rel_offset, rel_size = struct.unpack_from("<II", data, section_header + 16)
        applies_to = struct.unpack_from("<I", data, section_header + 28)[0]
        if applies_to == rodata_index and rel_size:
            raise BuildError("a relocation section still applies to .rodata")
        # every relocation (e.g. .rel.text) must no longer point AT .rodata:
        # an un-folded `$LC` reference would be left dangling by the strip
        symtab = struct.unpack_from("<I", data, section_header + 24)[0]
        sym_offset = struct.unpack_from("<I", data, header(symtab) + 16)[0]
        for k in range(rel_size // 8):
            info_word = struct.unpack_from("<I", data, rel_offset + k * 8 + 4)[0]
            shndx = struct.unpack_from(
                "<H", data, sym_offset + (info_word >> 8) * 16 + 14
            )[0]
            if shndx == rodata_index:
                raise BuildError(
                    f"{section_name(index)} still references .rodata; "
                    "MASPSX_RODATA_FOLD did not retarget every $LC operand"
                )
    struct.pack_into("<I", data, rodata_header + 20, 0)
    object_path.write_bytes(data)
    info(
        f"rodata fold: stripped {rodata_size}-byte duplicate literals "
        f"({symbols_spec}) from {object_path.relative_to(ROOT)}"
    )


def compile_era(
    unit: dict[str, Any], tools: Toolchain, pool_dir: Path = ROOT / "asm/disc1/data"
) -> None:
    source = ROOT / unit["source"]
    output = ROOT / unit["object"]
    leaf_environment = os.environ.copy()
    leaf_environment.update(unit["environment"])
    era_cpp, era_cc1 = era_compiler(unit["environment"])
    with tempfile.TemporaryDirectory(prefix="pe-era-") as temporary:
        temp = Path(temporary)
        preprocessed = temp / "x.i"
        assembly = temp / "x.s"
        expanded = temp / "xm.s"
        with preprocessed.open("wb") as stream:
            run(
                [str(ROOT / era_cpp), str(source)],
                env=leaf_environment,
                stdout=stream,
                stderr=subprocess.DEVNULL,
            )
        run(
            [
                str(ROOT / era_cc1),
                "-quiet",
                *unit["flags"],
                str(preprocessed),
                "-o",
                str(assembly),
            ],
            env=leaf_environment,
        )
        forced = unit["environment"].get("MASPSX_FORCE_ABSOLUTE_SYMBOLS")
        if forced:
            force_absolute_symbols(assembly, forced)
        aspsx_version = unit["environment"].get("ERA_ASPSX_VER", ERA_ASPSX_VER_DEFAULT)
        maspsx_command = [
            sys.executable,
            str(ROOT / MASPSX),
            f"--aspsx-version={aspsx_version}",
            # cc1's small-data limit, so maspsx classifies sized `.extern`
            # symbols the way GNU as will place them (must precede the input
            # path, which maspsx pops last).
            maspsx_sdata_flag(list(unit["flags"])),
        ]
        # Most retail leaves want cc1's `li` left alone (GNU as expands it to
        # addiu). A few were compiled with the immediate materialised as
        # `ori $r,$zero,imm` instead; those leaves opt in per-leaf with
        # MASPSX_EXPAND_LI=1 in their build profile, exactly like
        # MASPSX_EXPAND_DIV below. Default is unchanged.
        if unit["environment"].get("MASPSX_EXPAND_LI") != "1":
            maspsx_command.append("--dont-expand-li")
        if unit["environment"].get("MASPSX_EXPAND_DIV") == "1":
            maspsx_command.append("--expand-div")
        if unit["environment"].get("MASPSX_USE_COMM_SECTION") == "1":
            maspsx_command.append("--use-comm-section")
        maspsx_command.append(str(assembly))
        with expanded.open("wb") as stream:
            run(
                maspsx_command,
                env=leaf_environment,
                stdin=subprocess.DEVNULL,
                stdout=stream,
            )
        run(
            tools.command(
                tools.assembler,
                *AS_FLAGS,
                "-I",
                str(ROOT / "include"),
                "-o",
                str(output),
                str(expanded),
            ),
            env=leaf_environment,
        )
    dispatch = unit["environment"].get("MASPSX_DISPATCH_FOLD")
    rodata_fold = unit["environment"].get("MASPSX_RODATA_FOLD")
    if dispatch and rodata_fold:
        # one .rodata holds both the switch table(s) and the fold literals
        strip_dispatch_rodata(output, dispatch, pool_dir, fold_spec=rodata_fold)
    elif dispatch:
        strip_dispatch_rodata(output, dispatch, pool_dir)
    elif rodata_fold:
        strip_rodata_fold(output, rodata_fold, pool_dir)


def compile_all(plan: dict[str, Any], tools: Toolchain) -> None:
    counts: dict[str, int] = {"era": 0, "modern": 0}
    pool_dir = ROOT / plan_target(plan).asm_dir / "data"
    for unit in plan["units"]:
        if unit["kind"] != "c":
            continue
        Path(unit["object"]).parent.mkdir(parents=True, exist_ok=True)
        if unit["toolchain"] == "era":
            compile_era(unit, tools, pool_dir)
        elif unit["toolchain"] == "modern":
            run(
                tools.command(
                    tools.compiler,
                    *MODERN_C_FLAGS,
                    *unit["flags"],
                    "-c",
                    "-o",
                    unit["object"],
                    unit["source"],
                )
            )
        else:
            raise BuildError(f"unknown toolchain for {unit['name']}")
        counts[unit["toolchain"]] += 1
    info(
        f"compiled {sum(counts.values())} YAML C leaves "
        f"({counts['era']} era, {counts['modern']} modern)"
    )


def trim_all(plan: dict[str, Any]) -> None:
    for unit in plan["units"]:
        run(
            [
                sys.executable,
                str(ROOT / TRIM),
                unit["object"],
                unit["primary_section"],
                f"0x{unit['size']:X}",
            ]
        )
    header = plan.get("header")
    if header:
        result = run(
            [
                sys.executable,
                str(ROOT / TRIM),
                header["object"],
                header["section"],
                f"0x{header['size']:X}",
            ],
            check=False,
        )
        if result.returncode:
            info("header pad trim was not required")
    info(f"trimmed {len(plan['units'])} YAML-derived object spans")


def prepare_absolute_symbols(target: Target = MAIN_TARGET) -> Path:
    output = ROOT / target.build_dir / "abs_syms.ld"
    output.parent.mkdir(parents=True, exist_ok=True)
    pieces: list[str] = []
    for name in target.auto_symbol_files:
        path = ROOT / name
        if path.is_file():
            pieces.append(path.read_text(encoding="utf-8"))
    # Library-named callees (cc1's soft-float helpers, memset) bind to their
    # retail addresses; unreferenced entries have no effect on the image.
    aliases = ROOT / "configs" / "USA" / "libcall_aliases.ld"
    if target.is_main and aliases.is_file():
        pieces.append(aliases.read_text(encoding="utf-8"))
    pieces.extend([".L00000000_main = 0;\n", "_gp = 0x8009CD70;\n"])
    output.write_text("".join(pieces), encoding="utf-8")
    return output


def link(plan: dict[str, Any], tools: Toolchain) -> None:
    target = plan_target(plan)
    build_dir = Path(target.build_dir)
    elf_stem = "disc1" if target.is_main else target.id
    linker_script = ROOT / target.generated_dir / "disc1_romorder.ld"
    absolute_symbols = prepare_absolute_symbols(target)
    probe_error = ROOT / build_dir / "link_probe.err"
    probe_command = tools.command(
        tools.linker,
        "-EL",
        "-m",
        "elf32ltsmip",
        "-nostdlib",
        "--no-check-sections",
        "-T",
        str(linker_script),
        "-T",
        str(absolute_symbols),
        "-o",
        str(build_dir / f"{elf_stem}_probe.elf"),
    )
    with probe_error.open("wb") as stream:
        probe = run(probe_command, stderr=stream, check=False)
    if probe.returncode:
        errors = probe_error.read_text(encoding="utf-8", errors="replace")
        # Address-named symbols (D_/jtbl_/func_ + 8 hex digits) bind to the
        # address their name encodes; which prefixes may be derived is a
        # per-target decision (the EXE keeps its historical D_-only rule).
        prefixes = "|".join(re.escape(p) for p in target.derive_symbol_prefixes)
        symbols = sorted(
            set(
                re.findall(
                    rf"undefined reference to `?((?:{prefixes})[0-9A-Fa-f]{{8}})\b",
                    errors,
                )
            )
        )
        if symbols:
            with absolute_symbols.open("a", encoding="utf-8") as stream:
                for symbol in symbols:
                    stream.write(f"{symbol} = 0x{symbol[-8:]};\n")
            info(f"probe link added {len(symbols)} derived absolute symbols")

    link_error = ROOT / build_dir / "link.err"
    link_command = tools.command(
        tools.linker,
        "-EL",
        "-m",
        "elf32ltsmip",
        "-nostdlib",
        "--no-check-sections",
        "-T",
        str(linker_script),
        "-T",
        str(absolute_symbols),
        "-Map",
        str(build_dir / f"{elf_stem}.map"),
        "-o",
        str(build_dir / f"{elf_stem}.elf"),
    )
    with link_error.open("wb") as stream:
        linked = run(link_command, stderr=stream, check=False)
    if linked.returncode:
        excerpt = "\n".join(
            link_error.read_text(encoding="utf-8", errors="replace").splitlines()[:40]
        )
        raise BuildError(f"link failed ({linked.returncode})\n{excerpt}")
    info(f"OK {build_dir / f'{elf_stem}.elf'}")


def pack_and_compare(plan: dict[str, Any], tools: Toolchain) -> None:
    target = plan_target(plan)
    build_dir = Path(target.build_dir)
    elf_stem = "disc1" if target.is_main else target.id
    elf = str(build_dir / f"{elf_stem}.elf")
    header_size = plan["file"]["header_size"]
    load_size = plan["file"]["load_size"]
    header = b""
    if plan.get("header"):
        run(
            tools.command(
                tools.objcopy,
                "-O",
                "binary",
                "-j",
                ".header",
                elf,
                str(build_dir / f"{elf_stem}.header.bin"),
            )
        )
        header = (ROOT / build_dir / f"{elf_stem}.header.bin").read_bytes()[
            :header_size
        ].ljust(header_size, b"\x00")
    run(
        tools.command(
            tools.objcopy,
            "-O",
            "binary",
            "-j",
            ".main",
            elf,
            str(build_dir / f"{elf_stem}.main.bin"),
        )
    )
    main_raw = (ROOT / build_dir / f"{elf_stem}.main.bin").read_bytes()
    body = main_raw[:load_size].ljust(load_size, b"\x00")
    candidate = header + body
    candidate_path = ROOT / target.candidate
    candidate_path.parent.mkdir(parents=True, exist_ok=True)
    candidate_path.write_bytes(candidate)
    original = (ROOT / target.retail).read_bytes()
    original_sha1 = hashlib.sha1(original).hexdigest()
    candidate_sha1 = hashlib.sha1(candidate).hexdigest()
    if plan.get("header"):
        info(f"header: {len(header)} bytes (match={header == original[:header_size]})")
    info(f"main raw: {len(main_raw)} (0x{len(main_raw):X}); packed body: 0x{load_size:X}")
    info(f"candidate: {len(candidate)} (0x{len(candidate):X})")
    info(f"orig SHA-1: {original_sha1}")
    info(f"cand SHA-1: {candidate_sha1}")
    if candidate == original:
        info("RESULT: EXACT MATCH")
        return

    mismatch = next(
        (index for index, pair in enumerate(zip(candidate, original)) if pair[0] != pair[1]),
        min(len(candidate), len(original)),
    )
    owner = None
    for unit in plan["units"]:
        if unit["start"] <= mismatch < unit["end"]:
            owner = unit
            break
    detail = "header/outside YAML plan"
    if owner:
        detail = (
            f"{owner['kind']} {owner['name'] or owner['source']} "
            f"[0x{owner['start']:X},0x{owner['end']:X}) profile={owner['profile']}"
        )
    raise BuildError(
        f"NON-MATCH: first byte at 0x{mismatch:X}: "
        f"candidate=0x{candidate[mismatch]:02X} retail=0x{original[mismatch]:02X}; "
        f"owner={detail}; candidate SHA-1 {candidate_sha1}"
    )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--assemble-only", action="store_true")
    parser.add_argument(
        "--target",
        default=None,
        metavar="ID",
        help="rebuild target: disc1 (default, the EXE) or an overlay id from "
        "configs/USA/overlays/manifest.yaml",
    )
    args = parser.parse_args(argv)

    try:
        target = load_target(args.target, root=ROOT)
        candidate = ROOT / target.candidate
        candidate.unlink(missing_ok=True)

        step("Generate and validate YAML build authority")
        plan = build_plan(root=ROOT, require_generated=True, target=target)
        write_generated(plan, ROOT / target.generated_dir)
        require_inputs(plan)

        step("Toolchain")
        tools = find_toolchain()
        info(f"using: {tools.note}")
        info(f"as: {first_version_line(tools.command(tools.assembler, '--version'))}")
        info(f"ld: {first_version_line(tools.command(tools.linker, '--version'))}")
        info(f"cc: {first_version_line(tools.command(tools.compiler, '--version'))}")

        step("Assemble YAML asm/rodata spans")
        assemble_all(plan, tools)
        step("Compile YAML C spans")
        compile_all(plan, tools)
        step("Trim objects to YAML span geometry")
        trim_all(plan)
        if args.assemble_only:
            print("\nAssemble/compile-only complete; no matching claim.")
            return 0

        step("Link generated ROM-order plan")
        link(plan, tools)
        step("Pack and compare retail executable")
        pack_and_compare(plan, tools)
    except (BuildError, PlanError, TargetError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1

    print("\n=== Summary ===")
    if not target.is_main:
        print(f"Target:    {target.id} ({target.description})")
    print(f"Plan:      OK ({plan['counts']['units']} YAML spans; no manual span lists)")
    print(f"Compile:   OK ({plan['counts']['c']} generated C entries)")
    print("Trim/link: OK (sizes/order generated from YAML edges)")
    print(f"Compare:   EXACT SHA-1 {plan['expected_sha1']}")
    if plan["counts"]["c"]:
        print(f"Matching claim: YES ({plan['counts']['c']} registered C leaves)")
    else:
        print("Matching claim: NONE (0 registered C leaves; this is the split/reassemble round-trip proof only)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
