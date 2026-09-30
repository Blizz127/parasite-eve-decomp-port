#!/usr/bin/env bash
# Durable registration guard for a newly matched era/modern C leaf.
#
# Runs the two checks that must both pass BEFORE a leaf is declared done:
#   1. link-level exactness at the retail VMA (era_link_check.py), and
#   2. the *deep* span-size check for exactly this leaf
#      (disc1_preflight.py --deep --only <name>), which catches the
#      "declared span exceeds compiled .text" defect class (a `c` span that
#      silently swallowed the next real function).
#
# The deep check is the authoritative size gate; the fast preflight does NOT
# compile leaves and therefore cannot catch it. Run this for every leaf you
# carve, and run the full `disc1_preflight.py --deep` before declaring a batch
# done.
#
# Usage: tools/analysis/check_leaf.sh <func_name> <vram_hex> <size_hex> [cc1 flags...]
# Example:
#   tools/analysis/check_leaf.sh func_800906E4 0x800906E4 0x38 -O2 -G0
#
# NOTE: maspsx knobs are selected per-leaf by the build-profile JSON, not by
# this script. Export the same env var the profile sets (e.g.
# MASPSX_SYMBOL_AT_TEMP=1) or the link check will compile the wrong shape.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"

NAME="${1:?func_name}"
VRAM="${2:?vram}"
SIZE="${3:?size}"
shift 3
FLAGS=("${@:-"-O2" "-G0"}")

SRC="src/$NAME.c"
[[ -f "$SRC" ]] || { echo "missing $SRC (never leave unmatched C in src/)"; exit 2; }

echo "== [$NAME] link-level check =="
python3 tools/analysis/era_link_check.py "$SRC" "$VRAM" "$SIZE" "${FLAGS[@]}"

echo "== [$NAME] deep span-size check =="
python3 tools/build/disc1_preflight.py --deep --only "$NAME"

echo "== [$NAME] OK (link exact + span size exact) =="
