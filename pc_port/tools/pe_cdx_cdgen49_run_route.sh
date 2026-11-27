#!/usr/bin/env bash
# usage: run_route.sh UNTIL TAG   (capture build, isolated env; adds PE_CDX_TRACE49 which the stock script's env -i would drop)
set -u
ROOT=${ROOT:-$(cd "$(dirname "$0")/../.." && pwd)}
D=${DISC:?set DISC to the Disc 1 .bin}
W=${CDX_WORK:?set CDX_WORK to a scratch dir}; B=${BIN:-$ROOT/build/vec/pe-wrap49}
until=$1; tag=$2; O=$W/run-route-$tag; rm -rf $O; mkdir -p $O/{cwd,tmp,home,data,config,state,cache,shots}; cd $O/cwd
env -i PATH=/usr/bin:/bin HOME=$O/home TMPDIR=$O/tmp XDG_DATA_HOME=$O/data XDG_CONFIG_HOME=$O/config XDG_STATE_HOME=$O/state XDG_CACHE_HOME=$O/cache PE_ROUTE_AIDS_PAD_ONLY=1 DISC="$D" PE_DISC1_BIN="$D" PE_SHOT_DIR=$O/shots PE_SHOT_EVERY=300 PE_ROUTE_SHOTS=1 PE_ROUTE_PLAY_MOVIES=1 PE_AUDIO_WAV=$O/route.wav PE_CDX_TRACE49=$W/route-$tag.cap PE_MEMCARD1=$O/card.mcd timeout 1500 python3 $ROOT/pc_port/tools/day1_route_regress.py --bin $B --until $until --max-frames 90000 --log $O/route.log --screenshot $O/route.ppm > $O/regress.out 2>&1
echo "route $tag rc=$?"; tail -2 $O/regress.out
