#!/bin/bash
# addleaf.sh <room> <draft.c> "<one-line description>" : link-check (default profile), install with header, carve.
set -uo pipefail
cd "$(dirname "$0")/../../.."; export TMPDIR=${TMPDIR:-$PWD/build/tmp}

export LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu
r=$1; d=$2; desc=$3; t=room_$r
f=$(grep -vE '^extern|^#' $d | grep -oE '\bfunc_[0-9A-F]{8}\(' | head -1 | grep -oE 'func_[0-9A-F]{8}')
line=$(grep -h "nonmatching $f," asm/overlays/$t/*.s) || { echo "$f not an asm function in $t"; exit 1; }
sz=$(echo "$line" | awk '{print $3}')
res=$(python3 tools/analysis/era_link_check.py --target $t $d 0x${f#func_} $sz -O2 -G0 | tail -1)
[ "$res" = LINK_EXACT ] || { echo "$f $res"; exit 1; }
off=$(printf '0x%X' $((0x${f#func_} - 0x8018EFE8)))
mkdir -p src/overlays/$t
{ echo "/* $t (PE.IMG room ${r} chunk 2, VRAM 0x8018EFE8)"; echo " * $f — blob offset $off, $sz bytes. Profile era_o2_g0 (default);"; echo " * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md)."; echo " * $desc */"; echo; grep -v '^/\*.*\*/$' $d; } > src/overlays/$t/$f.c
python3 tools/analysis/room_lane/carve.py $t $f >/dev/null && { for i in 1 2 3 4 5; do git add -N src/overlays/$t/$f.c 2>/dev/null && break; sleep 2; done; } && echo "ADDED $t $f $sz"
