#!/usr/bin/env bash
# Player-experience coverage run for the native port (see
# docs/evidence/player-experience-coverage-20261101/COVERAGE.md).
#
# Runs the Day-1 pad-only route to eve1_victory with field movies playing
# (PE_ROUTE_PLAY_MOVIES=1 holds the route clock while a movie owns the
# display), a screenshot every EVERY presents (PE_SHOT_DIR / PE_SHOT_EVERY /
# PE_ROUTE_SHOTS) and a route-length WAV (PE_AUDIO_WAV), inside an isolated
# HOME / XDG tree so no real save or config is touched. Then hashes the
# frames with pc_port/tools/pe_frame_coverage.py.
#
# The frames and the WAV are rendered from the user's disc: keep them out of
# git (only the hash manifest is evidence).
#
# Usage: scripts/pe_coverage_route.sh BINARY OUTDIR [EVERY=300] [MAX_PRESENTS=90000]
#   DISC=<path to Disc 1 .bin> overrides the default rom/image location.
#   BASELINE=<manifest> compares the frames against an earlier run.
#   BASELINE_WAV_SHA=<sha256> fails the run when the WAV differs from it.
#   UNTIL=<checkpoint> stops at another day1_route_regress.py checkpoint
#   (default eve1_victory, which the committed baselines use).
#   FIXED_END=1 waits for a clean MAX_PRESENTS exit after the checkpoint;
#   use the same explicit limit for baseline and candidate audio captures.
#   PE_RUN_LOCK=<file> holds that flock (e.g. a gate's run-slot lock) for the
#   whole run, so a concurrent gate route waits instead of competing.
set -euo pipefail
if [ -n "${PE_RUN_LOCK:-}" ] && [ -z "${PE_RUN_LOCK_HELD:-}" ]; then
    exec env PE_RUN_LOCK_HELD=1 flock -w 900 "$PE_RUN_LOCK" "$0" "$@"
fi
BIN=$(realpath "$1"); OUT=$(realpath -m "$2"); EVERY=${3:-300}; MAXF=${4:-90000}
ROOT=$(cd "$(dirname "$0")/.." && pwd)
DISC=${DISC:-"$ROOT/rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin"}
mkdir -p "$OUT"/{cwd,tmp,home,data,config,state,cache,shots}
finish_args=()
if [ "${FIXED_END:-0}" = 1 ]; then finish_args+=(--finish-at-frame-limit); fi
cd "$OUT/cwd"
rc=0
env -i PATH=/usr/bin:/bin HOME="$OUT/home" TMPDIR="$OUT/tmp" XDG_DATA_HOME="$OUT/data" \
  XDG_CONFIG_HOME="$OUT/config" XDG_STATE_HOME="$OUT/state" XDG_CACHE_HOME="$OUT/cache" \
  PE_ROUTE_AIDS_PAD_ONLY=1 DISC="$DISC" PE_DISC1_BIN="$DISC" \
  PE_SHOT_DIR="$OUT/shots" PE_SHOT_EVERY="$EVERY" PE_ROUTE_SHOTS=1 PE_ROUTE_PLAY_MOVIES=1 \
  PE_AUDIO_WAV="$OUT/route.wav" \
  python3 "$ROOT/pc_port/tools/day1_route_regress.py" --bin "$BIN" --until "${UNTIL:-eve1_victory}" \
  --max-frames "$MAXF" "${finish_args[@]}" --log "$OUT/route.log" --screenshot "$OUT/route.ppm" \
  > "$OUT/regress.out" 2>&1 || rc=$?
echo "route rc=$rc"
tail -1 "$OUT/regress.out"
args=("$OUT/shots" --write "$OUT/frames.manifest")
[ -n "${BASELINE:-}" ] && args+=(--baseline "$BASELINE")
python3 "$ROOT/pc_port/tools/pe_frame_coverage.py" "${args[@]}" || rc=$((rc ? rc : 1))
if [ -f "$OUT/route.wav" ]; then
    wsha=$(sha256sum "$OUT/route.wav" | cut -c1-64)
    echo "route.wav sha256 $wsha"
    if [ -n "${BASELINE_WAV_SHA:-}" ]; then
        if [ "$wsha" = "$BASELINE_WAV_SHA" ]; then echo "AUDIO_BASELINE_EQUAL"; else echo "AUDIO_BASELINE_DIFF"; rc=$((rc ? rc : 1)); fi
    fi
fi
exit "$rc"
