#!/usr/bin/env bash
# scripts/fetch_psycross.sh — fetch PsyCross (OpenDriver2, MIT) at a pinned
# commit into the git-ignored pc_port/third_party/psycross/.
#
# PsyCross is the planned replacement layer for the Psy-Q-derived pieces of
# the native port (docs/ai_context/PSYQ_PORT_REPLACEMENT.md, step P0).  It is
# fetched, never committed, and NOT linked into the build yet.  Licence and
# exclusions: pc_port/THIRD_PARTY.md.  In particular PsyCross's embedded libgte
# tables (src/gte/sqrt_tbl.h, rcossin_tbl.h, ratan_tbl.h) are byte-identical to
# Sony libgte and must never be compiled into the port.
#
# Idempotent: an existing checkout at the pin is left alone; a checkout at a
# different commit is moved to the pin; anything else fails loudly.
# Override the destination with PSYCROSS_DIR=... (must stay git-ignored).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PSYCROSS_REPO="https://github.com/OpenDriver2/PsyCross.git"
# Pin verified 2026-09-28 with `git ls-remote`: upstream master HEAD, and the
# same commit Brave Fencer Musashi pins.
PSYCROSS_PIN="e56e4cde1c2b8a15e0d4e38b26cdd9202e0d17e6"
DEST="${PSYCROSS_DIR:-$ROOT/pc_port/third_party/psycross}"
MARKER="$DEST/.pe_psycross_pin"

command -v git >/dev/null || { echo "ERROR: git is required" >&2; exit 1; }

if [[ -e "$DEST" && ! -d "$DEST/.git" ]]; then
    echo "ERROR: $DEST exists but is not a git checkout; remove it first" >&2
    exit 1
fi

if [[ ! -d "$DEST/.git" ]]; then
    echo "Fetching PsyCross @ $PSYCROSS_PIN ..."
    mkdir -p "$(dirname "$DEST")"
    tmp="$DEST.partial.$$"
    rm -rf "$tmp"
    trap 'rm -rf "$tmp"' EXIT
    git init -q "$tmp"
    git -C "$tmp" remote add origin "$PSYCROSS_REPO"
    git -C "$tmp" fetch -q --depth 1 origin "$PSYCROSS_PIN"
    git -C "$tmp" -c advice.detachedHead=false checkout -q FETCH_HEAD
    mv "$tmp" "$DEST"
    trap - EXIT
fi

head="$(git -C "$DEST" rev-parse HEAD)"
if [[ "$head" != "$PSYCROSS_PIN" ]]; then
    if [[ -n "$(git -C "$DEST" status --porcelain)" ]]; then
        echo "ERROR: $DEST has local edits at $head; PsyCross patches belong in build-dir copies, never in the checkout" >&2
        exit 1
    fi
    echo "Moving PsyCross checkout $head -> $PSYCROSS_PIN ..."
    git -C "$DEST" fetch -q --depth 1 origin "$PSYCROSS_PIN"
    git -C "$DEST" -c advice.detachedHead=false checkout -q FETCH_HEAD
    head="$(git -C "$DEST" rev-parse HEAD)"
fi
[[ "$head" == "$PSYCROSS_PIN" ]] || { echo "ERROR: PsyCross at $head, expected $PSYCROSS_PIN" >&2; exit 1; }

[[ -f "$DEST/LICENSE" || -f "$DEST/LICENSE.md" || -f "$DEST/LICENSE.txt" ]] || {
    echo "ERROR: PsyCross checkout has no LICENSE file" >&2; exit 1; }

if [[ -n "$(git -C "$ROOT" ls-files -- "$DEST" 2>/dev/null)" ]] ||
   ! git -C "$ROOT" check-ignore -q "$DEST/LICENSE" 2>/dev/null; then
    case "$DEST" in
        "$ROOT"/*) echo "ERROR: $DEST is not git-ignored in this repository" >&2; exit 1 ;;
    esac
fi

echo "$PSYCROSS_PIN" > "$MARKER"
echo "OK  PsyCross $PSYCROSS_PIN at $DEST"
