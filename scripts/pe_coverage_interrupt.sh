#!/usr/bin/env bash
# Player-experience coverage: interruption recovery (see
# docs/evidence/player-experience-coverage-20261101/COVERAGE.md).
#
# Phase A is the save phase of scripts/pe_coverage_saveload.sh, but the
# process is SIGKILLed (no clean shutdown) by pc_port/tools/pe_run_kill_on.py:
#   MODE=after-save  kill two output lines after the "[MEMCARD] ... saved"
#                    line: the save completed, the process never exits
#                    cleanly. Phase B (a fresh process, title -> Continue ->
#                    load) must restore A80663C8 / story 0x5E.
#   MODE=mid-save    kill at the "[MEMCARD] ... created" line, between the
#                    directory-entry write and the data write. Phase B
#                    records what the next launch shows; the run reports the
#                    outcome and does not assert one.
# Both modes check for a leftover "<card>.tmp" and whether the card file is
# still a full 128 KiB image. Both kill points come after the port's
# tmp + rename persist has returned, so they do not exercise a kill inside
# that write. mid-save classifies the card by its first file block (block 1:
# all zero = the data write never landed).
#
# Frames and the card are rendered or derived from the user's disc, so keep
# them out of git.
#
# Usage: scripts/pe_coverage_interrupt.sh BINARY OUTDIR [after-save|mid-save]
#   DISC=<path to Disc 1 .bin>, PE_RUN_LOCK=<file> as in pe_coverage_route.sh.
set -euo pipefail
if [ -n "${PE_RUN_LOCK:-}" ] && [ -z "${PE_RUN_LOCK_HELD:-}" ]; then
    exec env PE_RUN_LOCK_HELD=1 flock -w 900 "$PE_RUN_LOCK" "$0" "$@"
fi
BIN=$(realpath "$1"); OUT=$(realpath -m "$2"); MODE=${3:-after-save}
ROOT=$(cd "$(dirname "$0")/.." && pwd)
DISC=${DISC:-"$ROOT/rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin"}
CARD="$OUT/memcard1.mcd"
case "$MODE" in
    after-save) PAT='^\[MEMCARD\] .*\bsaved\b'; AFTER=2 ;;
    mid-save)   PAT='^\[MEMCARD\] port 0: created BASLUS'; AFTER=0 ;;
    *) echo "unknown mode $MODE" >&2; exit 2 ;;
esac
run_env() {   # $1 = phase dir; the rest is the command
    local P=$1; shift
    mkdir -p "$P"/{cwd,tmp,home,data,config,state,cache,shots}
    (cd "$P/cwd" && env -i PATH=/usr/bin:/bin HOME="$P/home" TMPDIR="$P/tmp" XDG_DATA_HOME="$P/data" \
        XDG_CONFIG_HOME="$P/config" XDG_STATE_HOME="$P/state" XDG_CACHE_HOME="$P/cache" \
        PE_MEMCARD1="$CARD" PE_SHOT_DIR="$P/shots" "$@")
}
card_state() {
    if [ -e "$CARD.tmp" ]; then echo "leftover $CARD.tmp: yes"; else echo "leftover tmp: no"; fi
    if [ -f "$CARD" ]; then
        echo "card bytes $(stat -c %s "$CARD") sha256 $(sha256sum "$CARD" | cut -c1-64)"
    else
        echo "card missing"
    fi
}
mkdir -p "$OUT/save" "$OUT/continue"
rc=0

# Phase A: the save-phase run of pe_coverage_saveload.sh, killed on PAT.
run_env "$OUT/save" PE_SHOT_EVERY=100 PE_ROUTE_SHOTS=1 PE_ROUTE_M34_NO_SHIM=1 PE_ROUTE_AIDS_PAD_ONLY=1 \
    PE_PADONLY_FHEAL=0 PE_DEBUG_SAVE_MENU_AT=44500 \
    PE_PAD_SCRIPT='0:FFFF,300:BFFF,302:FFFF,600:BFFF,602:FFFF,800:FF7F,802:FFFF,900:BFFF,902:FFFF,1200:BFFF,1202:FFFF,1600:BFFF,1602:FFFF' \
    python3 "$ROOT/pc_port/tools/pe_run_kill_on.py" --log "$OUT/save/run.log" --pattern "$PAT" \
    --after-lines "$AFTER" -- "$BIN" --headless --route-pad --hand-over-at 44400 --max-frames 46400 \
    --disc-image "$DISC" > "$OUT/save/kill.out" 2>&1 || true
cat "$OUT/save/kill.out"
grep -E '^\[MEMCARD\]|^\[HOST\] (stop_reason|final room)' "$OUT/save/run.log" || true
if ! grep -q 'killed=1' "$OUT/save/kill.out"; then echo "INTERRUPT_FAIL: the kill was not sent"; exit 1; fi
if grep -q '^\[HOST\] stop_reason' "$OUT/save/run.log"; then echo "INTERRUPT_FAIL: the process shut down cleanly"; exit 1; fi
echo "after kill:"; card_state
before=$(sha256sum "$CARD" 2>/dev/null | cut -c1-64 || echo none)

# Phase B: a fresh process, title -> Continue -> load (as pe_coverage_saveload.sh).
CONT="9300:FFF7,9330:FFFF,9600:BFFF,9630:FFFF,9900:BFFF,9930:FFFF,10200:BFFF,10230:FFFF,10500:BFFF,10530:FFFF"
run_env "$OUT/continue" PE_SHOT_EVERY=150 PE_PAD_SCRIPT="$CONT" PE_PAD_SCRIPT_CLOCK=vblank \
    "$BIN" --headless --max-frames 14000 --disc-image "$DISC" \
    --screenshot "$OUT/continue/final.ppm" > "$OUT/continue/run.log" 2>&1 || rc=$?
echo "continue phase rc=$rc"
grep -E '^\[MEMCARD\]|^\[HOST\] (stop_reason|final room)' "$OUT/continue/run.log" || true
echo "after continue:"; card_state
after=$(sha256sum "$CARD" 2>/dev/null | cut -c1-64 || echo none)
restored=0
grep -qE '^\[HOST\] final room token=A80663C8 story persist\[74\]=0000005E' "$OUT/continue/run.log" && restored=1
python3 "$ROOT/pc_port/tools/pe_frame_coverage.py" "$OUT/save/shots" --write "$OUT/save.manifest" || true
python3 "$ROOT/pc_port/tools/pe_frame_coverage.py" "$OUT/continue/shots" --write "$OUT/continue.manifest" || true
if [ "$MODE" = after-save ]; then
    if [ "$restored" = 1 ] && [ "$before" = "$after" ] && [ ! -e "$CARD.tmp" ]; then
        echo "INTERRUPT_PASS mode=after-save: Continue restored A80663C8 / story 0x5E after a SIGKILL; card unchanged by loading; no tmp left"
    else
        echo "INTERRUPT_FAIL mode=after-save restored=$restored"; rc=$((rc ? rc : 1))
    fi
else
    # Classify by card content: the data is persisted inside the write call,
    # before the "closed file" line, so a log line count would mislabel a
    # kill between the write and the close.
    data=$(python3 -c 'import sys; d=open(sys.argv[1],"rb").read(); print("zero" if not any(d[0x2000:0x4000]) else "present")' "$CARD")
    echo "INTERRUPT_OUTCOME mode=mid-save first_file_block=$data restored=$restored card_changed_by_continue=$([ "$before" = "$after" ] && echo 0 || echo 1)"
fi
exit "$rc"
