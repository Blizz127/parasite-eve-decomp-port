#!/usr/bin/env python3
"""Link-level exactness check for an era leaf.

Compiles a single `src/*.c` leaf with the era toolchain (cpp -> cc1 -> maspsx
-> as), links it at its retail VMA with every undefined symbol `--defsym`'d to
its retail address (the repo's address-named-symbol convention), then compares
the linked `.text` word-for-word against the extracted retail EXE.

Usage:
  tools/analysis/era_link_check.py [--target <ovl>] [--all-mismatches]
      <src.c> <vram_hex> <size_hex> [cc1 flags...]

`--all-mismatches` (or env ERA_LINK_CHECK_ALL_MISMATCHES=1) lists every
mismatched word instead of the first 12. The option may appear anywhere
before or after the positional arguments.

Prints `LINK_EXACT` and exits 0 when every word matches, else prints the
mismatch count / first offsets and exits 1.
"""
import os
import re
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
ERA = ROOT / "tools" / "era"
# ERA_CC1_VER selects the stock decompals/old-gcc cc1+cpp pair (2.7.2 default,
# 2.8.1 for the leaves whose delay slots only 2.8.1 fills). Same per-leaf
# environment contract as ERA_ASPSX_VER: the build profile's `environment`
# block sets it, and this check honours the same variable so the two agree.
CC1_VER = os.environ.get("ERA_CC1_VER", "2.7.2")
CC1 = ERA / f"gcc-{CC1_VER}-psx" / "cc1"
CPP = ERA / f"gcc-{CC1_VER}-psx" / "cpp"
# ERA_LINK_CHECK_MASPSX=<path/to/maspsx.py> runs a development copy instead
# of the live tools/era/maspsx (e.g. while the live tree is frozen for a gate).
MASPSX = Path(os.environ.get("ERA_LINK_CHECK_MASPSX") or ERA / "maspsx" / "maspsx.py").resolve()
HOST = ROOT / "tools" / "mipsel-host"
AS = HOST / "usr" / "bin" / "mipsel-linux-gnu-as"
LD = HOST / "usr" / "bin" / "mipsel-linux-gnu-ld"
OBJCOPY = HOST / "usr" / "bin" / "mipsel-linux-gnu-objcopy"
NM = HOST / "usr" / "bin" / "mipsel-linux-gnu-nm"
# The vendored host binutils are relocated Debian packages: their shared
# libraries (libopcodes/libbfd) live beside them, not on the system search
# path, so every host-tool call runs with that directory prefixed.
HOST_LIB = HOST / "usr" / "lib" / "x86_64-linux-gnu"
HOST_ENV = dict(os.environ)
HOST_ENV["LD_LIBRARY_PATH"] = str(HOST_LIB) + (
    ":" + os.environ["LD_LIBRARY_PATH"] if os.environ.get("LD_LIBRARY_PATH") else ""
)
EXE = ROOT / "build" / "extracted" / "disc1" / "SLUS_006.62"

SYM_TAIL_RE = re.compile(r"([0-9A-Fa-f]{6,8})$")

# Retail SLUS_006.62 was assembled with ASPSX >= 2.30 throughout (1246
# signature sites for 2.30 behaviour, 0 for 2.21 -- see
# docs/ai_context/TOOLCHAIN_REBUILD.md).  ERA_ASPSX_VER remains a per-leaf
# override.
ERA_ASPSX_VER_DEFAULT = "2.30"


def maspsx_sdata_flag(flags):
    """Return cc1's `-G<n>` small-data limit as the `-G<n>` maspsx expects.

    maspsx must know the same limit cc1 used: a `.extern SYM, size` whose
    size fits the limit resolves gp-relative (one instruction), and the
    MIPS-I load-delay nop before such a store is only emitted when maspsx can
    see that.  Missing `-G` means `-G0` (nothing gp-relative).
    """
    limit = "0"
    it = iter(flags)
    for flag in it:
        if flag == "-G":
            limit = next(it, "0")
        elif flag.startswith("-G") and flag[2:].isdigit():
            limit = flag[2:]
    return f"-G{limit}"


def elf_section(data: bytes, want: str):
    """Return the raw bytes of a named ELF32 section, or None."""
    if data[:4] != b"\x7fELF":
        return None
    e_shoff = struct.unpack_from("<I", data, 32)[0]
    e_shentsize = struct.unpack_from("<H", data, 46)[0]
    e_shnum = struct.unpack_from("<H", data, 48)[0]
    e_shstrndx = struct.unpack_from("<H", data, 50)[0]
    shstr_off = struct.unpack_from("<I", data, e_shoff + e_shstrndx * e_shentsize + 16)[0]
    for i in range(e_shnum):
        sh = e_shoff + i * e_shentsize
        name_off = struct.unpack_from("<I", data, sh)[0]
        start = shstr_off + name_off
        end = data.index(b"\x00", start)
        if data[start:end].decode("ascii", "replace") != want:
            continue
        off = struct.unpack_from("<I", data, sh + 16)[0]
        sz = struct.unpack_from("<I", data, sh + 20)[0]
        return data[off:off + sz]
    return None


LIBCALL_ALIASES = Path(__file__).resolve().parent.parent.parent / "configs" / "USA" / "libcall_aliases.ld"
_ALIAS_RE = re.compile(r"^\s*([A-Za-z_][A-Za-z0-9_.]*)\s*=\s*(0x[0-9A-Fa-f]+)\s*;", re.M)


def libcall_aliases() -> dict:
    """Library-named routines (`__divdf3`, `memset`, ...) -> retail VMA."""
    if not LIBCALL_ALIASES.is_file():
        return {}
    text = re.sub(r"/\*.*?\*/", "", LIBCALL_ALIASES.read_text(encoding="utf-8"), flags=re.S)
    return {name: int(value, 16) for name, value in _ALIAS_RE.findall(text)}


def sym_address(name: str):
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


def main():
    argv = sys.argv[1:]
    # `--all-mismatches` is removed wherever it appears so it never reaches
    # cc1's flag list; cc1 has no option of that name.
    show_all = os.environ.get("ERA_LINK_CHECK_ALL_MISMATCHES") == "1"
    if "--all-mismatches" in argv:
        show_all = True
        argv = [a for a in argv if a != "--all-mismatches"]
    # Retail bytes default to the EXE at its PS-X mapping. `--target <id>`
    # (first argument) points the comparison at a PE.IMG overlay blob from
    # configs/USA/overlays/manifest.yaml, whose file offset 0 is its VRAM.
    retail_path, load_vram, file_start = EXE, 0x80010000, 0x800
    if argv[:1] == ["--target"]:
        sys.path.insert(0, str(ROOT / "tools" / "build"))
        from overlay_targets import load_target  # noqa: PLC0415
        target = load_target(argv[1], root=ROOT)
        retail_path = ROOT / target.retail
        load_vram, file_start = target.vram, target.file_start
        argv = argv[2:]
    src, vram_s, size_s = argv[0], argv[1], argv[2]
    flags = argv[3:] or ["-O2", "-G0"]
    vram = int(vram_s, 0)
    size = int(size_s, 0)
    src_path = Path(src)
    if not src_path.is_absolute():
        src_path = ROOT / src_path
    if not (CC1.is_file() and CPP.is_file()):
        print(f"MISSING_ERA_CC1 {CC1_VER}: {CC1.parent}; run scripts/setup_era.sh")
        return 2

    with tempfile.TemporaryDirectory() as d:
        d = Path(d)
        with open(d / "x.i", "wb") as fi:
            r = subprocess.run([str(CPP), str(src_path)], stdout=fi,
                               stderr=subprocess.DEVNULL)
        assert r.returncode == 0, "cpp failed"
        r = subprocess.run([str(CC1), "-quiet", *flags, str(d / "x.i"),
                            "-o", str(d / "x.s")])
        assert r.returncode == 0, "cc1 failed"
        # Same knob as the in-tree build: remove the `.extern` for forced-absolute
        # symbols so maspsx/as macro-expand them absolutely instead of gp-relative.
        forced = os.environ.get("MASPSX_FORCE_ABSOLUTE_SYMBOLS")
        if forced:
            text = (d / "x.s").read_text()
            for symbol in forced.split(","):
                text = re.sub(
                    rf"\t\.extern\t{re.escape(symbol)}, \d+\n", "", text)
            (d / "x.s").write_text(text)
        with open(d / "xm.s", "w") as fo:
            maspsx_command = [sys.executable, str(MASPSX),
                              f"--aspsx-version={os.environ.get('ERA_ASPSX_VER', ERA_ASPSX_VER_DEFAULT)}",
                              "--dont-expand-li"]
            if os.environ.get("MASPSX_EXPAND_DIV") == "1":
                maspsx_command.append("--expand-div")
            if os.environ.get("MASPSX_USE_COMM_SECTION") == "1":
                maspsx_command.append("--use-comm-section")
            # Forward cc1's small-data limit (maspsx's own `-G<n>` parsing);
            # it must precede the input path, which maspsx pops last.
            maspsx_command.append(maspsx_sdata_flag(flags))
            maspsx_command.append(str(d / "x.s"))
            # maspsx reads its file argument only when stdin is a tty or hits EOF;
            # an inherited socket stdin (agent harness, CI) would block forever.
            r = subprocess.run(maspsx_command, stdin=subprocess.DEVNULL,
                               stdout=fo, stderr=subprocess.DEVNULL)
        assert r.returncode == 0, "maspsx failed"
        r = subprocess.run([str(AS), "-EL", "-mips1", "-mabi=32",
                            "-I", str(ROOT / "include"), "-o", str(d / "x.o"),
                            str(d / "xm.s")], env=HOST_ENV)
        assert r.returncode == 0, "as failed"

        nm = subprocess.run([str(NM), "-u", str(d / "x.o")],
                            capture_output=True, text=True, env=HOST_ENV).stdout
        undef = []
        for line in nm.splitlines():
            parts = line.split()
            if len(parts) >= 2 and parts[0] == "U":
                undef.append(parts[1])
        defsyms = []
        unresolved = []
        for s in undef:
            a = sym_address(s)
            if a is None:
                unresolved.append(s)
            else:
                defsyms += ["--defsym", f"{s}={a:#x}"]
        if unresolved:
            print(f"UNRESOLVED_SYMBOLS {unresolved}")
            return 1

        # Link with an explicit script so the object's .text lands exactly at the
        # retail VMA (the default link forces 16-byte section alignment, which
        # shifts absolute `j`/`jal` targets by the padding).
        leaf = src_path.stem
        script = d / "link.ld"
        script.write_text(
            "SECTIONS {\n"
            f"  . = {vram:#x};\n"
            "  .text : SUBALIGN(4) { *(.text) }\n"
            "  /DISCARD/ : { *(.MIPS.abiflags) *(.reginfo) *(.pdr) *(.comment) }\n"
            "}\n"
        )
        ldargs = [str(LD), "-T", str(script), "-o", str(d / "x.elf"),
                  str(d / "x.o"), "--defsym", "_gp=0x8009CD70", *defsyms]
        r = subprocess.run(ldargs, capture_output=True, text=True, env=HOST_ENV)
        if r.returncode != 0:
            print("LD FAILED\n" + r.stdout + r.stderr)
            return 1
        elf = (d / "x.elf").read_bytes()
        linked = elf_section(elf, ".text")
        nmall = subprocess.run([str(NM), str(d / "x.elf")],
                               capture_output=True, text=True, env=HOST_ENV).stdout
        func_addr = None
        for line in nmall.splitlines():
            parts = line.split()
            if len(parts) == 3 and parts[2] == leaf:
                func_addr = int(parts[0], 16)
                break
        if func_addr is None:
            print("NO_FUNC_SYMBOL_IN_LINKED_ELF")
            return 1
        lead = func_addr - vram
        if lead < 0 or lead > len(linked):
            print(f"BAD_LEAD {lead:#x}")
            return 1
        linked = linked[lead:]

    exe = retail_path.read_bytes()
    off = vram - load_vram + file_start
    rom = exe[off:off + size]
    # Optional: keep the linked .text for an alignment-aware diff.
    dump = os.environ.get("ERA_LINK_CHECK_DUMP")
    if dump:
        Path(dump).write_bytes(linked)

    # Compare the whole target span: a SHORT link counts every missing word
    # as a mismatch (it used to compare min(len) and could print LINK_EXACT).
    n = min(len(rom), size)
    mism = 0
    first = []
    for i in range(0, n, 4):
        rw = struct.unpack_from("<I", rom, i)[0]
        lw_ = struct.unpack_from("<I", linked, i)[0] if i + 4 <= len(linked) else None
        if lw_ is None or rw != lw_:
            mism += 1
            if show_all or len(first) < 12:
                first.append((vram + i, rw, lw_))
    # trailing linked bytes beyond target must be zero pad
    extra = linked[size:]
    extra_nonzero = sum(1 for i in range(0, len(extra) - 3, 4)
                        if struct.unpack_from("<I", extra, i)[0] != 0)
    print(f"linked .text {len(linked)} bytes, target {size:#x}, word mismatches={mism}, "
          f"nonzero_pad={extra_nonzero}")
    if mism > len(first):
        print(f"  (first {len(first)} of {mism}; --all-mismatches lists all)")
    for a, rw, lw_ in first:
        print(f"  {a:#x}: ROM {rw:08x}  LNK {lw_:08x}" if lw_ is not None
              else f"  {a:#x}: ROM {rw:08x}  LNK --------")
    if mism == 0 and extra_nonzero == 0:
        print("LINK_EXACT")
        return 0
    return 1


if __name__ == "__main__":
    sys.exit(main())
