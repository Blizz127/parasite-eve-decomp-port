#!/usr/bin/env bash
# Player-experience coverage: the player's own path from power-on through the
# title menu to New Game (see
# docs/evidence/player-experience-coverage-20261101/COVERAGE.md).
#
# No --route-pad: the opening movie plays, then a scripted pad (vblank clock)
# presses Start on the title, Up to "New Game", Cross to start, and then
# Cross every 300 vblanks: it advances the first dialogue and types into the
# Enter Name screen. A screenshot is taken every EVERY presents, and the
# frames are hashed with pc_port/tools/pe_frame_coverage.py.
#
# PE_PAD_SCRIPT masks are the raw PSX pad word, which is active-low: 0xFFFF
# is no button, Start 0xFFF7, Up 0xFFEF, Cross 0xBFFF.
#
# Frames are rendered from the user's disc: keep them out of git.
#
# Usage: scripts/pe_coverage_newgame.sh BINARY OUTDIR [EVERY=150] [MAX_PRESENTS=21000]
#   DISC=<path to Disc 1 .bin>, BASELINE=<manifest>, PE_RUN_LOCK=<file> as in
#   scripts/pe_coverage_route.sh.
set -euo pipefail
if [ -n "${PE_RUN_LOCK:-}" ] && [ -z "${PE_RUN_LOCK_HELD:-}" ]; then
    exec env PE_RUN_LOCK_HELD=1 flock -w 900 "$PE_RUN_LOCK" "$0" "$@"
fi
BIN=$(realpath "$1"); OUT=$(realpath -m "$2"); EVERY=${3:-150}; MAXF=${4:-21000}
ROOT=$(cd "$(dirname "$0")/.." && pwd)
DISC=${DISC:-"$ROOT/rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin"}
SCRIPT="9300:FFF7,9330:FFFF,9700:FFEF,9730:FFFF,9900:BFFF,9930:FFFF"
for v in $(seq 14600 300 19400); do SCRIPT="$SCRIPT,$v:BFFF,$((v + 20)):FFFF"; done
mkdir -p "$OUT"/{cwd,tmp,home,data,config,state,cache,shots}
cd "$OUT/cwd"
rc=0
env -i PATH=/usr/bin:/bin HOME="$OUT/home" TMPDIR="$OUT/tmp" XDG_DATA_HOME="$OUT/data" \
  XDG_CONFIG_HOME="$OUT/config" XDG_STATE_HOME="$OUT/state" XDG_CACHE_HOME="$OUT/cache" \
  PE_MEMCARD1="$OUT/card.mcd" PE_SHOT_DIR="$OUT/shots" PE_SHOT_EVERY="$EVERY" \
  PE_PAD_SCRIPT="$SCRIPT" PE_PAD_SCRIPT_CLOCK=vblank \
  "$BIN" --headless --max-frames "$MAXF" --disc-image "$DISC" --screenshot "$OUT/final.ppm" \
  > "$OUT/newgame.log" 2>&1 || rc=$?
echo "port rc=$rc"
grep -E '^\[HOST\] (stop_reason|final room)' "$OUT/newgame.log" || true
args=("$OUT/shots" --write "$OUT/frames.manifest")
[ -n "${BASELINE:-}" ] && args+=(--baseline "$BASELINE")
python3 "$ROOT/pc_port/tools/pe_frame_coverage.py" "${args[@]}" || rc=$((rc ? rc : 1))
exit "$rc"
