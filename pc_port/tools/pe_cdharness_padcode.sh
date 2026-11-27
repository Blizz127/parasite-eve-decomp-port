#!/usr/bin/env bash
# usage: pe_cdharness_padcode.sh TAG ROOM STORY [KEY=VALUE ...]
# Pad-code credits attempt: dev warp into a field room (Disc 1 only), then the nine-step input sequence of func_8003EB04
# (Square held; Triangle, Cross, Circle, Left, Up, Right, Down, Select, Start; masks from func_8003E974's pad->logical table and the table at
# 0x80092200) as a PE_PAD_SCRIPT after the hand-over.  Pad input only; coverage harness, not play evidence.
set -u
ROOT=${ROOT:-$(cd "$(dirname "$0")/../.." && pwd)}
D=${DISC:?}; W=${CDX_WORK:?}; B=${BIN:-$ROOT/build/cap/pe-cdh}; tag=$1; room=$2; story=$3; shift 3
O=$W/run-$tag; rm -rf "$O"; mkdir -p "$O"/{cwd,tmp,home,data,config,state,cache,shots}; cd "$O/cwd"
seq=(6FFF 3FFF 5FFF 7F7F 7FEF 7FDF 7FBF 7FFE 7FF7)
script=""; f=${PADCODE_START:-600}
for m in "${seq[@]}"; do script+="$f:$m,"; f=$((f+8)); script+="$f:7FFF,"; f=$((f+8)); done
script+="$f:FFFF"
echo "start $(date -u +%FT%TZ) script=$script" > "$O/times.txt"
env -i PATH=/usr/bin:/bin HOME=$O/home TMPDIR=$O/tmp XDG_DATA_HOME=$O/data XDG_CONFIG_HOME=$O/config XDG_STATE_HOME=$O/state XDG_CACHE_HOME=$O/cache \
  PE_MEMCARD1=$O/card.mcd PE_CDX_CAPTURE=$O/capture.cap PE_PAD_SCRIPT="$script" PE_DEV_MENU=1 PE_WARP_ROOM=$room PE_WARP_STORY=$story PE_WARP_AT=1000 "$@" \
  timeout ${WALL:-300} "$B" --headless --route-pad --hand-over-at 1000 --max-frames ${FRAMES:-4000} --disc-image "$D" > "$O/run.log" 2>&1
rc=$?; echo "end $(date -u +%FT%TZ) rc=$rc" >> "$O/times.txt"; echo "$tag rc=$rc"
