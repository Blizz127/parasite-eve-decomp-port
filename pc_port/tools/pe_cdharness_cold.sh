#!/usr/bin/env bash
# usage: pe_cdharness_cold.sh FRAMES TAG [KEY=VALUE ...]
# Cold boot of the (wrapped) runtime, isolated env, extra KEY=VALUE pairs (PE_TEST_CD_* fixtures) passed to the game.
# Env: DISC (Disc 1 .bin), CDX_WORK (scratch dir), BIN (binary), WALL (seconds, default 900).  Coverage harness, not play evidence.
set -u
ROOT=${ROOT:-$(cd "$(dirname "$0")/../.." && pwd)}
D=${DISC:?set DISC to the Disc 1 .bin}
W=${CDX_WORK:?set CDX_WORK to a scratch dir}; B=${BIN:-$ROOT/build/cap/pe-cdh}
n=$1; tag=$2; shift 2
O=$W/run-$tag; rm -rf "$O"; mkdir -p "$O"/{cwd,tmp,home,data,config,state,cache,shots}; cd "$O/cwd"
echo "start $(date -u +%FT%TZ)" > "$O/times.txt"
env -i PATH=/usr/bin:/bin HOME=$O/home TMPDIR=$O/tmp XDG_DATA_HOME=$O/data XDG_CONFIG_HOME=$O/config XDG_STATE_HOME=$O/state XDG_CACHE_HOME=$O/cache \
  PE_MEMCARD1=$O/card.mcd PE_SHOT_DIR=$O/shots PE_SHOT_EVERY=${SHOT_EVERY:-150} PE_AUDIO_WAV=$O/cold.wav PE_CDX_CAPTURE=$O/capture.cap "$@" \
  timeout ${WALL:-900} "$B" --headless --max-frames "$n" --disc-image "$D" --screenshot "$O/final.ppm" > "$O/cold.log" 2>&1
rc=$?; echo "end $(date -u +%FT%TZ) rc=$rc" >> "$O/times.txt"; echo "$tag rc=$rc"
