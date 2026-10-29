#!/usr/bin/env bash
# Warp probe: boot the native binary headless, warp into one room with the
# dev-only env warp, mash X, and print one summary line (stop / boundary).
#
# Usage: pc_port/tools/warp_probe.sh <room|token> <story-hex> [frames] [bin]
#   e.g. DISC=/path/disc1.bin pc_port/tools/warp_probe.sh m0049i 90
# Env: DISC (absolute disc image path, required), DISC2 (optional Disc 2 image:
#      --disc2-image, the tray's spare disc; with PE_WARP_DISC=2 it is in the
#      drive at the warp), PE_WARP_TIMEOUT (s, default 240), PE_WARP_PAD (PE_PAD_SCRIPT
#      after the hand-over; default: X pressed 20 frames on / 20 off).
# Log: build/lanes/warp/<room>-<story>.log (git-ignored).
set -uo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"
R=$1; S=$2; F=${3:-4000}; BIN=${4:-build/pcbuild-disc/parasite-eve-port}
: "${DISC:?set DISC to the absolute disc image path}"
mkdir -p build/lanes/warp; L=build/lanes/warp/$R-$S.log
PAD=${PE_WARP_PAD:-$(python3 -c "print(','.join(f'{i*20}:{\"BFFF\" if i%2==0 else \"FFFF\"}' for i in range(400)))")}
PE_PAD_SCRIPT="$PAD" PE_DEV_MENU=1 PE_WARP_ROOM=$R PE_WARP_STORY=$S PE_WARP_AT=1000 \
  timeout "${PE_WARP_TIMEOUT:-240}" "$BIN" --headless --route-pad --hand-over-at 1000 --max-frames "$F" \
  --disc-image "$DISC" ${DISC2:+--disc2-image "$DISC2"} > "$L" 2>&1
w=$(grep -c '^\[WARP\] env transfer' "$L")
fin=$(grep -a 'final room token' "$L" | grep -o 'token=[0-9A-F]*')
sr=$(grep -a 'stop_reason' "$L" | tail -1 | sed 's/.*stop_reason=//'); [ -z "$sr" ] && sr=hang-or-crash
b=$(grep -a -m2 -E 'CPU_BOUNDARY|^\[STOP\]|FATAL' "$L" | tr '\n' '|' | cut -c1-300)
echo "$R story=$S warp=$w final=$fin stop=$sr :: $b"
