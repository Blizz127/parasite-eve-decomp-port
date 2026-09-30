#!/usr/bin/env bash
# Day-1 route regression: run the headless --route-pad autopilot from cold boot
# and check an ordered list of checkpoints. Exits non-zero on the first missed
# checkpoint (or on a game-over/reset), printing frame/token/story context.
#
# Usage:
#   pc_port/tools/day1_route_regress.sh [--bin PATH] [--until CHECKPOINT]
#                                       [--max-frames N] [--log PATH] [--list]
# Defaults: --bin build/pcbuild-port/parasite-eve-port, --until m34_entry,
#           --max-frames 90000, --log build/lanes/day1/regress-<until>.log
# Env: DISC (disc image), PE_ROUTE_* pass through to the port.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"
export TMPDIR="${TMPDIR:-$ROOT/build/tmp}"
mkdir -p "$TMPDIR" build/lanes/day1
exec python3 "$ROOT/pc_port/tools/day1_route_regress.py" "$@"
