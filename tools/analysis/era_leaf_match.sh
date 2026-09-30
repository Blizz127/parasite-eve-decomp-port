#!/usr/bin/env bash
# Compile a C leaf with era gcc-2.7.2-psx + maspsx and diff .text vs ROM.
# Usage: tools/analysis/era_leaf_match.sh <src.c> <vram_hex> <size_hex> [cc1 flags...]
# Example: tools/analysis/era_leaf_match.sh src/func_800293F4.c 0x800293F4 0x1F0 -O2 -G8
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"

SRC="${1:?src.c}"
VRAM="${2:?vram}"
SIZE="${3:?size}"
shift 3
FLAGS=("${@:-"-O2" "-G8"}")

# ERA_CC1_VER=2.8.1 selects the stock gcc-2.8.1-psx pair (default 2.7.2), the
# same per-leaf knob era_link_check.py / disc1_build.py honour.
ERA_CC1_VER="${ERA_CC1_VER:-2.7.2}"
ERA_CPP="$ROOT/tools/era/gcc-$ERA_CC1_VER-psx/cpp"
ERA_CC1="$ROOT/tools/era/gcc-$ERA_CC1_VER-psx/cc1"
MASPSX="$ROOT/tools/era/maspsx/maspsx.py"
AS="${AS:-mips-linux-gnu-as}"
OBJDUMP="${OBJDUMP:-mips-linux-gnu-objdump}"
OBJCOPY="${OBJCOPY:-mips-linux-gnu-objcopy}"
EXE="$ROOT/build/extracted/disc1/SLUS_006.62"

[[ -x "$ERA_CC1" && -x "$ERA_CPP" && -f "$MASPSX" ]] || { echo "era toolchain missing"; exit 2; }
[[ -f "$EXE" ]] || { echo "missing $EXE"; exit 2; }
command -v "$AS" >/dev/null || { echo "assembler $AS not found"; exit 2; }

d="$(mktemp -d)"
trap 'rm -rf "$d"' EXIT

"$ERA_CPP" "$SRC" > "$d/x.i" 2>/dev/null
"$ERA_CC1" -quiet "${FLAGS[@]}" "$d/x.i" -o "$d/x.s"
# ASPSX >= 2.30 is the retail baseline (docs/ai_context/TOOLCHAIN_REBUILD.md);
# ERA_ASPSX_VER overrides per invocation.  cc1's -G<n> is forwarded so maspsx
# classifies sized `.extern` symbols the way GNU as will place them.
G_FLAG="-G0"
for f in "${FLAGS[@]}"; do case "$f" in -G[0-9]*) G_FLAG="$f";; esac; done
python3 "$MASPSX" --aspsx-version="${ERA_ASPSX_VER:-2.30}" --dont-expand-li "$G_FLAG" "$d/x.s" > "$d/xm.s" </dev/null
"$AS" -EL -mips1 -mabi=32 -I "$ROOT/include" -o "$d/x.o" "$d/xm.s"

echo "==== cc1 flags: ${FLAGS[*]} ===="
echo "==== cc1 asm ===="
cat "$d/x.s"
echo "==== objdump .text ===="
"$OBJDUMP" -d "$d/x.o" || true

python3 - "$EXE" "$d/x.o" "$VRAM" "$SIZE" << 'PY'
import struct, sys
from pathlib import Path

exe = Path(sys.argv[1]).read_bytes()
obj = Path(sys.argv[2]).read_bytes()
vram = int(sys.argv[3], 0)
size = int(sys.argv[4], 0)

off = vram - 0x80010000 + 0x800
rom = exe[off:off+size]

# ELF32 LE: extract .text
assert obj[:4] == b'\x7fELF'
e_shoff = struct.unpack_from('<I', obj, 32)[0]
e_shentsize = struct.unpack_from('<H', obj, 46)[0]
e_shnum = struct.unpack_from('<H', obj, 48)[0]
e_shstrndx = struct.unpack_from('<H', obj, 50)[0]
shstr_off = struct.unpack_from('<I', obj, e_shoff + e_shstrndx * e_shentsize + 16)[0]

def sec_name(i):
    name_off = struct.unpack_from('<I', obj, e_shoff + i * e_shentsize)[0]
    start = shstr_off + name_off
    end = obj.index(b'\x00', start)
    return obj[start:end].decode('ascii', errors='replace')

text = None
for i in range(e_shnum):
    if sec_name(i) == '.text':
        sh = e_shoff + i * e_shentsize
        sh_offset = struct.unpack_from('<I', obj, sh + 16)[0]
        sh_size = struct.unpack_from('<I', obj, sh + 20)[0]
        text = obj[sh_offset:sh_offset+sh_size]
        break
if text is None:
    print('NO .text'); sys.exit(1)

# trim trailing zeros for size compare, but report raw
print(f'ROM  .text {len(rom)} bytes  C .text {len(text)} bytes  target {size}')
n = min(len(rom), len(text), size)
first = None
mismatch = 0
for i in range(0, n, 4):
    rw = struct.unpack_from('<I', rom, i)[0]
    cw = struct.unpack_from('<I', text, i)[0] if i+4 <= len(text) else None
    if cw is None or rw != cw:
        mismatch += 1
        if first is None:
            first = i
if len(text) != size:
    print(f'SIZE_MISMATCH C={len(text):#x} ROM={size:#x}')
if first is None and len(text) >= size:
    # allow trailing pad in C
    extra = text[size:]
    if extra and any(b != 0 for b in extra):
        print(f'EXTRA_NONZERO pad {len(extra)} bytes')
    else:
        print('BYTE_EXACT (ignoring gas align pad)')
else:
    print(f'MISMATCHES={mismatch} first_off={first} vram={vram+first:#x}' if first is not None else 'LEN_ONLY_MISMATCH')
    # show first 12 mismatched words
    shown = 0
    for i in range(0, n, 4):
        rw = struct.unpack_from('<I', rom, i)[0]
        cw = struct.unpack_from('<I', text, i)[0] if i+4 <= len(text) else None
        if cw is None or rw != cw:
            cws = f'{cw:08x}' if cw is not None else '--------'
            print(f'  {vram+i:#x}: ROM {rw:08x}  C {cws}')
            shown += 1
            if shown >= 16:
                break
PY
echo "==== done $SRC ===="
