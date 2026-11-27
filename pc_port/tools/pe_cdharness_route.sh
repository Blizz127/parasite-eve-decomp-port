#!/usr/bin/env bash
# usage: pe_cdharness_route.sh UNTIL TAG [KEY=VALUE ...]   (Day-1 route regress through the wrapped binary; extra KEY=VALUE go to the game env)
# Env: DISC (Disc 1 .bin), CDX_WORK, BIN.  PE_ROUTE_PLAY_MOVIES=1 and PE_ROUTE_AIDS_PAD_ONLY=1 are the cdgen47 settings; pass them explicitly.
set -u
ROOT=${ROOT:-$(cd "$(dirname "$0")/../.." && pwd)}
D=${DISC:?}; W=${CDX_WORK:?}; B=${BIN:-$ROOT/build/cap/pe-cdh}; until=$1; tag=$2; shift 2
O=$W/run-$tag; rm -rf "$O"; mkdir -p "$O"/{cwd,tmp,home,data,config,state,cache,shots}; cd "$O/cwd"
echo "start $(date -u +%FT%TZ)" > "$O/times.txt"
env -i PATH=/usr/bin:/bin HOME=$O/home TMPDIR=$O/tmp XDG_DATA_HOME=$O/data XDG_CONFIG_HOME=$O/config XDG_STATE_HOME=$O/state XDG_CACHE_HOME=$O/cache \
  DISC="$D" PE_DISC1_BIN="$D" PE_SHOT_DIR=$O/shots PE_SHOT_EVERY=300 PE_ROUTE_SHOTS=1 PE_AUDIO_WAV=$O/route.wav PE_CDX_CAPTURE=$O/capture.cap PE_MEMCARD1=$O/card.mcd "$@" \
  timeout ${WALL:-1500} python3 $ROOT/pc_port/tools/day1_route_regress.py --bin "$B" --until "$until" --max-frames 90000 --log $O/route.log --screenshot $O/route.ppm > $O/regress.out 2>&1
rc=$?; echo "end $(date -u +%FT%TZ) rc=$rc" >> "$O/times.txt"; echo "$tag rc=$rc"; tail -2 "$O/regress.out" | cut -c1-200
