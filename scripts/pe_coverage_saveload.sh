#!/usr/bin/env bash
# Player-experience coverage: save, then relaunch and Continue (see
# docs/evidence/player-experience-coverage-20261101/COVERAGE.md). Based on
# docs/evidence/native-boss-save-20261027/{save,continue}.sh.
#
# Phase A (save): the pad-only Day-1 route to the first victory (route frame
# 44344); input is handed over at 44400; the existing diagnostic save-menu
# entry opens the native save menu at 44500 (PE_DEBUG_SAVE_MENU_AT: an
# assisted UI entry, disclosed; no story/position/HP/inventory injection);
# a pad script saves to slot 1 of a fresh card under OUTDIR.
# Phase B (continue): a cold boot with no route aids plays the opening; a
# vblank-clocked pad script presses Start on the title, Cross on Continue,
# then Cross to pick and load the save. The run passes when the final room
# token and story match the save point (A80663C8 / 0x5E) and the card is
# unchanged by loading.
#
# Both phases capture frames (PE_SHOT_DIR); frames and the card are
# rendered or derived from the user's disc, so keep them out of git.
#
# Usage: scripts/pe_coverage_saveload.sh BINARY OUTDIR
#   DISC=<path to Disc 1 .bin>, PE_RUN_LOCK=<file> as in pe_coverage_route.sh.
set -euo pipefail
if [ -n "${PE_RUN_LOCK:-}" ] && [ -z "${PE_RUN_LOCK_HELD:-}" ]; then
    exec env PE_RUN_LOCK_HELD=1 flock -w 900 "$PE_RUN_LOCK" "$0" "$@"
fi
BIN=$(realpath "$1"); OUT=$(realpath -m "$2")
ROOT=$(cd "$(dirname "$0")/.." && pwd)
DISC=${DISC:-"$ROOT/rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin"}
CARD="$OUT/memcard1.mcd"
run_env() {   # $1 = phase dir; the rest is the command
    local P=$1; shift
    mkdir -p "$P"/{cwd,tmp,home,data,config,state,cache,shots}
    (cd "$P/cwd" && env -i PATH=/usr/bin:/bin HOME="$P/home" TMPDIR="$P/tmp" XDG_DATA_HOME="$P/data" \
        XDG_CONFIG_HOME="$P/config" XDG_STATE_HOME="$P/state" XDG_CACHE_HOME="$P/cache" \
        PE_MEMCARD1="$CARD" PE_SHOT_DIR="$P/shots" "$@")
}
mkdir -p "$OUT/save" "$OUT/continue"   # before the redirections below
# Phase A saves to a fresh card: a card left in OUTDIR by an earlier run of
# this script would turn the save into an overwrite (different menu frames
# and save timing).
rm -f "$CARD" "$CARD.tmp"
rc=0

# Phase A: save after the first victory (pad script as in the 10-27 run).
run_env "$OUT/save" PE_SHOT_EVERY=100 PE_ROUTE_SHOTS=1 PE_ROUTE_M34_NO_SHIM=1 PE_ROUTE_AIDS_PAD_ONLY=1 \
    PE_PADONLY_FHEAL=0 PE_DEBUG_SAVE_MENU_AT=44500 \
    PE_PAD_SCRIPT='0:FFFF,300:BFFF,302:FFFF,600:BFFF,602:FFFF,800:FF7F,802:FFFF,900:BFFF,902:FFFF,1200:BFFF,1202:FFFF,1600:BFFF,1602:FFFF' \
    "$BIN" --headless --route-pad --hand-over-at 44400 --max-frames 46400 --disc-image "$DISC" \
    --screenshot "$OUT/save/final.ppm" > "$OUT/save/run.log" 2>&1 || rc=$?
echo "save phase rc=$rc"
grep -E '^\[MEMCARD\]|^\[HOST\] (stop_reason|final room)' "$OUT/save/run.log" || true
if ! grep -qE '^\[MEMCARD\] .*\bsaved\b' "$OUT/save/run.log"; then echo "SAVE_FAIL: no [MEMCARD] ... saved line"; exit 1; fi
before=$(sha256sum "$CARD" | cut -c1-64); echo "card after save sha256 $before"

# Phase B: cold boot, title -> Continue -> load.
CONT="9300:FFF7,9330:FFFF,9600:BFFF,9630:FFFF,9900:BFFF,9930:FFFF,10200:BFFF,10230:FFFF,10500:BFFF,10530:FFFF"
run_env "$OUT/continue" PE_SHOT_EVERY=150 PE_PAD_SCRIPT="$CONT" PE_PAD_SCRIPT_CLOCK=vblank \
    "$BIN" --headless --max-frames 14000 --disc-image "$DISC" \
    --screenshot "$OUT/continue/final.ppm" > "$OUT/continue/run.log" 2>&1 || rc=$?
echo "continue phase rc=$rc"
grep -E '^\[MEMCARD\]|^\[HOST\] (stop_reason|final room)' "$OUT/continue/run.log" || true
after=$(sha256sum "$CARD" | cut -c1-64); echo "card after continue sha256 $after"
if grep -qE '^\[HOST\] final room token=A80663C8 story persist\[74\]=0000005E' "$OUT/continue/run.log" \
   && [ "$before" = "$after" ]; then
    echo "SAVELOAD_PASS: Continue restored A80663C8 / story 0x5E; card unchanged by loading"
else
    echo "SAVELOAD_FAIL"; rc=$((rc ? rc : 1))
fi
python3 "$ROOT/pc_port/tools/pe_frame_coverage.py" "$OUT/save/shots" --write "$OUT/save.manifest" || true
python3 "$ROOT/pc_port/tools/pe_frame_coverage.py" "$OUT/continue/shots" --write "$OUT/continue.manifest" || true
exit "$rc"
