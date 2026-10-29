#!/usr/bin/env bash
# Warp sweep (dev harness; room-ENTRY evidence only, never play evidence).
#
# For each ROOM:STORY[:DISC] it boots New Game headless with --route-pad, warps
# at frame 1000 with the dev env warp (PE_DEV_MENU=1, PE_WARP_ROOM/STORY, and
# PE_WARP_DISC=2 when DISC is 2), presses Cross 20 frames on / 20 off and stops
# at FRAMES. Same probe as warp_probe.sh, but every room gets a fresh HOME,
# XDG_DATA/CONFIG/STATE_HOME, TMPDIR and cwd under OUT/<room>-<story>/, the
# environment is cleared (env -i), and the port gets --screenshot under that
# dir (without it the port writes its shutdown frame to /tmp/pe-port-black.ppm).
#
# usage: DISC=<Disc 1 .bin> [DISC2=<Disc 2 .bin>] [SWEEP_EXTRA="VAR=VAL ..."] \
#        laterdays_warp_sweep.sh OUT FRAMES BIN ROOM:STORY[:DISC] [...]
#   SWEEP_EXTRA  extra variables for every room, e.g. a persist block:
#                SWEEP_EXTRA="PE_WARP_PERSIST_FILE=/path/persist_2D6.bin"
# The caller owns locking and niceness (e.g. flock <run-slot lock> nice -n 19 ...);
# each room also has its own timeout (SWEEP_ROOM_TIMEOUT seconds, default 300).
# OUT/summary.txt gets one line per room; laterdays_sweep_table.py OUT makes the table.
set -euo pipefail
OUT=${1:?usage: laterdays_warp_sweep.sh OUT FRAMES BIN ROOM:STORY[:DISC] ...}
FRAMES=${2:?frames}
BIN=${3:?port binary}
shift 3
: "${DISC:?set DISC to the Disc 1 image}"
[ -x "$BIN" ] || { echo "not executable: $BIN" >&2; exit 2; }
[ ! -e "$OUT" ] || { echo "$OUT exists (fresh dir required)" >&2; exit 1; }
mkdir -p "$OUT"
read -r -a EXTRA <<< "${SWEEP_EXTRA:-}"
PAD=$(python3 -I -c "print(','.join(f'{i*20}:{\"BFFF\" if i%2==0 else \"FFFF\"}' for i in range(400)))")
echo "START $(date -u +%FT%TZ) frames=$FRAMES bin=$BIN rooms=$#" | tee "$OUT/start.txt"
for spec in "$@"; do
  IFS=: read -r room story disc <<< "$spec"
  R=$OUT/$room-$story
  mkdir -p "$R/cwd" "$R/home/data" "$R/home/config" "$R/home/state" "$R/tmp"
  cd "$R/cwd"
  set +e
  timeout -k 10 "${SWEEP_ROOM_TIMEOUT:-300}" env -i PATH=/usr/bin:/bin HOME="$R/home" \
    XDG_DATA_HOME="$R/home/data" XDG_CONFIG_HOME="$R/home/config" XDG_STATE_HOME="$R/home/state" \
    TMPDIR="$R/tmp" PE_PAD_SCRIPT="$PAD" PE_DEV_MENU=1 PE_WARP_ROOM="$room" PE_WARP_STORY="$story" \
    PE_WARP_AT=1000 ${disc:+PE_WARP_DISC=$disc} "${EXTRA[@]}" \
    "$BIN" --headless --route-pad --hand-over-at 1000 --max-frames "$FRAMES" \
    --disc-image "$DISC" ${DISC2:+--disc2-image "$DISC2"} --screenshot "$R/end.ppm" > "$R/run.log" 2>&1
  rc=$?
  set -e
  L=$R/run.log
  w=$(grep -c '^\[WARP\] env transfer' "$L" || true)
  sr=$(grep -a 'stop_reason' "$L" | tail -1 | sed 's/.*stop_reason=//' || true); [ -n "$sr" ] || sr="none(rc=$rc)"
  b=$(grep -a -m2 -E 'CPU_BOUNDARY|^\[STOP\]|FATAL|DAY1_SHIM|not ported|STUB:BOOT' "$L" | tr '\n' '|' | cut -c1-300 || true)
  echo "$room story=$story disc=${disc:-1} warp=$w rc=$rc stop=$sr :: $b" | tee -a "$OUT/summary.txt"
done
echo "END $(date -u +%FT%TZ)" | tee "$OUT/end.txt"
