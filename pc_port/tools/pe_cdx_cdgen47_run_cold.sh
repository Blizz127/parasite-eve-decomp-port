#!/usr/bin/env bash
# usage: run_cold.sh FRAMES TAG  (cold boot, isolated env, capture binary BIN)
set -u
ROOT=${ROOT:-$(cd "$(dirname "$0")/../.." && pwd)}
D=${DISC:?set DISC to the Disc 1 .bin}
W=${CDX_WORK:?set CDX_WORK to a scratch dir}; B=${BIN:-$ROOT/build/cap/pe-cap2}
n=$1; tag=$2; O=$W/run-$tag; rm -rf $O; mkdir -p $O/{cwd,tmp,home,data,config,state,cache,shots}; cd $O/cwd
env -i PATH=/usr/bin:/bin HOME=$O/home TMPDIR=$O/tmp XDG_DATA_HOME=$O/data XDG_CONFIG_HOME=$O/config XDG_STATE_HOME=$O/state XDG_CACHE_HOME=$O/cache PE_MEMCARD1=$O/card.mcd PE_SHOT_DIR=$O/shots PE_SHOT_EVERY=150 PE_AUDIO_WAV=$O/cold.wav PE_CDX_CAPTURE=$W/$tag.cap timeout 900 $B --headless --max-frames $n --disc-image "$D" --screenshot $O/final.ppm > $O/cold.log 2>&1
echo "$tag rc=$?"
