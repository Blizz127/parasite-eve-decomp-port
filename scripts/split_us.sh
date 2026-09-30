#!/usr/bin/env bash
# Run splat against configs/USA/disc1.yaml to produce the first disassembly
# of SLUS_006.62 locally. All output (asm/disc1/, linkers/disc1.ld,
# assets/disc1/, .splache, *_auto.txt) is git-ignored and must NEVER be
# committed; this script verifies that mechanically before running.
#
# The canonical target is disc 1's SLUS_006.62 only: disc 2's SLUS_006.68 is
# byte-identical (see configs/USA/disc2.yaml and docs/disc_info.md).
#
# Usage:
#   scripts/split_us.sh --check   dry-run: verify every prerequisite and
#                                 print what a real run would generate,
#                                 without invoking splat
#   scripts/split_us.sh           run the split
#
# Prerequisites, checked below, in order:
#   1. Running from a checkout of this repo (root guard).
#   2. configs/USA/disc1.yaml exists (committed; Phase 2).
#   3. build/extracted/disc1/SLUS_006.62 exists — produced locally by
#      `scripts/extract_us.sh 1` from a user-supplied disc image.
#   4. The extracted EXE's SHA-1 matches the recorded value; a mismatch
#      means the wrong or corrupt input and the split must not run.
#   5. splat is available — prefer .venv/bin/splat (installed pinned by
#      `scripts/setup_env.sh`), else PATH.
#   6. Every output path is covered by .gitignore (git check-ignore), so a
#      split can never produce committable files.
#
# Running a split makes NO matching or rebuild claims — it only generates a
# first-pass disassembly to study. See docs/splitting.md.
set -euo pipefail

CHECK_ONLY=0
if [[ $# -gt 0 ]]; then
    case "$1" in
        --check) CHECK_ONLY=1 ;;
        *)
            echo "Usage: $0 [--check]" >&2
            exit 2
            ;;
    esac
fi

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# 1. Root guard: refuse to run from a stray copy of this script, and make
# sure git commands below operate on this repo.
if [[ ! -f "$ROOT/CLAUDE.md" || ! -d "$ROOT/configs/USA" ]]; then
    echo "ERROR: $ROOT does not look like the Parasite-Eve-Decompilation root." >&2
    exit 1
fi
if ! git -C "$ROOT" rev-parse --show-toplevel >/dev/null 2>&1; then
    echo "ERROR: $ROOT is not inside a git repository." >&2
    exit 1
fi

CONFIG="$ROOT/configs/USA/disc1.yaml"
EXE="$ROOT/build/extracted/disc1/SLUS_006.62"
EXPECTED_SHA1="452fb033f2eaa4b18aa20a5bca60b8125af3a37b"

# Everything a split writes (paths relative to repo root; see the config
# and splat defaults). Each must be git-ignored before we run anything.
OUTPUT_PATHS=(
    "asm/disc1"
    "linkers/disc1.ld"
    "assets/disc1"
    ".splache"
    "undefined_funcs_auto.txt"
    "undefined_syms_auto.txt"
    "include/gte_macros.inc"
    "include/include_asm.h"
    "include/labels.inc"
    "include/macro.inc"
)

# 2. Config present.
if [[ ! -f "$CONFIG" ]]; then
    echo "ERROR: missing $CONFIG" >&2
    exit 1
fi

# 3. Extracted EXE present.
if [[ ! -f "$EXE" ]]; then
    echo "ERROR: extracted executable not found: $EXE" >&2
    echo "Run 'scripts/extract_us.sh 1' first (needs your disc 1 image under rom/image/)." >&2
    exit 1
fi

# 4. EXE hash matches the recorded Phase 1 value.
actual_sha1="$(sha1sum "$EXE" | cut -d' ' -f1)"
if [[ "$actual_sha1" != "$EXPECTED_SHA1" ]]; then
    echo "ERROR: SHA-1 mismatch for $EXE" >&2
    echo "  expected: $EXPECTED_SHA1 (docs/disc_info.md)" >&2
    echo "  actual:   $actual_sha1" >&2
    echo "Refusing to split an unverified input. Re-run scripts/extract_us.sh 1." >&2
    exit 1
fi

# 5. splat available: pinned venv install first, PATH as fallback.
if [[ -x "$ROOT/.venv/bin/splat" ]]; then
    SPLAT="$ROOT/.venv/bin/splat"
elif command -v splat >/dev/null; then
    SPLAT="$(command -v splat)"
else
    echo "ERROR: splat not found (.venv/bin/splat missing and not on PATH)." >&2
    echo "Run 'scripts/setup_env.sh' to install the pinned toolchain." >&2
    exit 1
fi

# 6. Output paths must be git-ignored, so split output can never be staged.
ignore_violations=0
for path in "${OUTPUT_PATHS[@]}"; do
    if ! git -C "$ROOT" check-ignore -q "$path"; then
        echo "ERROR: output path not covered by .gitignore: $path" >&2
        ignore_violations=1
    fi
done
if [[ "$ignore_violations" -ne 0 ]]; then
    echo "Refusing to run: fix .gitignore first (generated output must never be committable)." >&2
    exit 1
fi

echo "Prerequisites OK:"
echo "  root:   $ROOT"
echo "  config: $CONFIG"
echo "  input:  $EXE (SHA-1 verified: $actual_sha1)"
echo "  splat:  $SPLAT"
echo "  ignore: all ${#OUTPUT_PATHS[@]} output paths covered by .gitignore"
echo
echo "A split run generates (locally, ALL git-ignored — never commit):"
for path in "${OUTPUT_PATHS[@]}"; do
    echo "  $path"
done

if [[ "$CHECK_ONLY" -eq 1 ]]; then
    echo
    echo "--check: dry run only, splat was NOT invoked and nothing was generated."
    echo "Would run: $SPLAT split $CONFIG"
    exit 0
fi

# Snapshot git status so we can detect any file a split adds OUTSIDE the
# ignore rules (fail loudly instead of leaving committable output around).
status_before="$(git -C "$ROOT" status --porcelain)"

echo
echo "Running: $SPLAT split $CONFIG"
"$SPLAT" split "$CONFIG"

# A YAML asm-to-C carve can leave an older splat unit behind. Remove only
# hex-named, git-ignored generated units that are no longer current (or whose
# embedded file offsets now land in a C span). The build-time stale-overlap
# gate remains the backstop if generation ever produces an invalid unit.
python3 "$ROOT/tools/build/disc1_plan.py" --root "$ROOT" --cleanup-stale-asm

status_after="$(git -C "$ROOT" status --porcelain)"
# LC_ALL=C is required: comm compares byte-wise while sort honours LC_COLLATE,
# so under a non-C locale (e.g. en_US.UTF-8) comm reports "not in sorted order"
# and exits 1, which `set -e` turns into a spurious split failure.
new_entries="$(LC_ALL=C comm -13 <(LC_ALL=C sort <<<"$status_before") <(LC_ALL=C sort <<<"$status_after"))"
if [[ -n "$new_entries" ]]; then
    echo "ERROR: the split created files git does not ignore:" >&2
    echo "$new_entries" >&2
    echo "Do NOT commit these. Extend .gitignore (and OUTPUT_PATHS above), then re-check." >&2
    exit 1
fi

echo
echo "Split complete. Output is local-only and git-ignored (verified: no new"
echo "tracked/untracked entries in git status)."
echo "This is a study artifact only — no matching or rebuild claims."
