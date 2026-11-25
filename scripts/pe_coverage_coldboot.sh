#!/usr/bin/env bash
# Player-experience coverage: cold boot with no pad input (see
# docs/evidence/player-experience-coverage-20261101/COVERAGE.md).
#
# Runs the native port headless from power-on for MAX_PRESENTS presents
# (default 9000: publisher card, logo, the whole opening movie and the title
# screen), with a screenshot every EVERY presents and a WAV dump, inside an
# isolated HOME / XDG tree with its own memory-card file. Then hashes the
# frames with pc_port/tools/pe_frame_coverage.py.
#
# Frames and WAV are rendered from the user's disc: keep them out of git.
#
# Usage: scripts/pe_coverage_coldboot.sh BINARY OUTDIR [EVERY=150] [MAX_PRESENTS=9000]
#   DISC=<path to Disc 1 .bin> overrides the default rom/image location.
#   BASELINE=<manifest> compares the frames against an earlier run.
#   BASELINE_WAV_SHA=<sha256> fails the run when the WAV differs from it.
#   PE_RUN_LOCK=<file> holds that flock for the whole run.
set -euo pipefail
if [ -n "${PE_RUN_LOCK:-}" ] && [ -z "${PE_RUN_LOCK_HELD:-}" ]; then
    exec env PE_RUN_LOCK_HELD=1 flock -w 900 "$PE_RUN_LOCK" "$0" "$@"
fi
BIN=$(realpath "$1"); OUT=$(realpath -m "$2"); EVERY=${3:-150}; MAXF=${4:-9000}
ROOT=$(cd "$(dirname "$0")/.." && pwd)
DISC=${DISC:-"$ROOT/rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin"}
mkdir -p "$OUT"/{cwd,tmp,home,data,config,state,cache,shots}
cd "$OUT/cwd"
rc=0
env -i PATH=/usr/bin:/bin HOME="$OUT/home" TMPDIR="$OUT/tmp" XDG_DATA_HOME="$OUT/data" \
  XDG_CONFIG_HOME="$OUT/config" XDG_STATE_HOME="$OUT/state" XDG_CACHE_HOME="$OUT/cache" \
  PE_MEMCARD1="$OUT/card.mcd" PE_SHOT_DIR="$OUT/shots" PE_SHOT_EVERY="$EVERY" \
  PE_AUDIO_WAV="$OUT/cold.wav" \
  "$BIN" --headless --max-frames "$MAXF" --disc-image "$DISC" --screenshot "$OUT/final.ppm" \
  > "$OUT/cold.log" 2>&1 || rc=$?
echo "port rc=$rc"
grep -E '^\[HOST\] (stop_reason|final room)' "$OUT/cold.log" || true
args=("$OUT/shots" --write "$OUT/frames.manifest")
[ -n "${BASELINE:-}" ] && args+=(--baseline "$BASELINE")
python3 "$ROOT/pc_port/tools/pe_frame_coverage.py" "${args[@]}" || rc=$((rc ? rc : 1))
if [ -f "$OUT/cold.wav" ]; then
    wsha=$(sha256sum "$OUT/cold.wav" | cut -c1-64)
    echo "cold.wav sha256 $wsha"
    if [ -n "${BASELINE_WAV_SHA:-}" ]; then
        if [ "$wsha" = "$BASELINE_WAV_SHA" ]; then echo "AUDIO_BASELINE_EQUAL"; else echo "AUDIO_BASELINE_DIFF"; rc=$((rc ? rc : 1)); fi
    fi
fi
exit "$rc"
