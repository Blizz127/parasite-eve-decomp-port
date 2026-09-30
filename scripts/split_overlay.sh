#!/usr/bin/env bash
# scripts/split_overlay.sh <id> — run splat on one PE.IMG overlay.
#
# The overlay twin of scripts/split_us.sh: same prerequisite guards, same
# "every output path must be git-ignored" rule, same before/after git-status
# check. Output (asm/overlays/<id>/, assets/overlays/<id>/,
# linkers/overlays/<id>.ld, the overlay's own undefined_*_auto.txt and
# .splache under asm/overlays/<id>/, include/*.inc) is git-ignored and must
# NEVER be committed.
#
# Usage:
#   scripts/split_overlay.sh <id> --check   dry-run: verify prerequisites, print
#                                           what a real run would generate
#   scripts/split_overlay.sh <id>           run the split
#
# <id> is an overlay id with a `config` entry in
# configs/USA/overlays/manifest.yaml (e.g. ovl_0700).
#
# Prerequisites, checked below, in order:
#   1. Running from a checkout of this repo (root guard).
#   2. The manifest names <id> and its splat config exists (committed).
#   3. The extracted blob exists — produced locally by
#      `scripts/extract_overlays.sh` from the user's PE.IMG.
#   4. The blob's SHA-1 matches the manifest; a mismatch means the wrong or a
#      corrupt input and the split must not run.
#   5. splat is available — prefer .venv/bin/splat (installed pinned by
#      `scripts/setup_env.sh`), else PATH.
#   6. Every output path is covered by .gitignore (git check-ignore).
#
# Running a split makes NO matching or rebuild claims — it generates a
# disassembly to study, and the reassembly proof is
# scripts/exact_rebuild_overlay.sh <id>. See docs/ai_context/OVERLAY_LANE.md.
set -euo pipefail

if [[ $# -lt 1 || $# -gt 2 ]]; then
    echo "Usage: $0 <overlay-id> [--check]" >&2
    exit 2
fi
OVL_ID="$1"
CHECK_ONLY=0
if [[ $# -eq 2 ]]; then
    case "$2" in
        --check) CHECK_ONLY=1 ;;
        *) echo "Usage: $0 <overlay-id> [--check]" >&2; exit 2 ;;
    esac
fi
if [[ ! "$OVL_ID" =~ ^[a-z][A-Za-z0-9_]*$ ]]; then
    echo "ERROR: overlay id must match ^[a-z][A-Za-z0-9_]*$, got '$OVL_ID'" >&2
    exit 2
fi

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# 1. Root guard.
if [[ ! -f "$ROOT/CLAUDE.md" || ! -d "$ROOT/configs/USA/overlays" ]]; then
    echo "ERROR: $ROOT does not look like the Parasite-Eve-Decompilation root." >&2
    exit 1
fi
if ! git -C "$ROOT" rev-parse --show-toplevel >/dev/null 2>&1; then
    echo "ERROR: $ROOT is not inside a git repository." >&2
    exit 1
fi
command -v python3 >/dev/null || { echo "ERROR: python3 not found on PATH" >&2; exit 1; }

# 2. Resolve the target from the manifest (config path, blob path, SHA-1).
if ! target_line="$(python3 - "$ROOT" "$OVL_ID" <<'PY'
import sys
from pathlib import Path
root, ovl_id = Path(sys.argv[1]), sys.argv[2]
sys.path.insert(0, str(root / "tools/build"))
from overlay_targets import TargetError, load_target
try:
    t = load_target(ovl_id, root=root)
except TargetError as exc:
    print(f"ERROR: {exc}", file=sys.stderr)
    raise SystemExit(1)
if t.is_main:
    print("ERROR: use scripts/split_us.sh for the executable", file=sys.stderr)
    raise SystemExit(1)
print(t.config, t.retail, t.expected_sha1, t.asm_dir, t.asset_dir, f"0x{t.vram:08X}", f"0x{t.load_size:X}")
PY
)"; then
    exit 1
fi
read -r CONFIG_REL BLOB_REL EXPECTED_SHA1 ASM_DIR ASSET_DIR VRAM SIZE <<<"$target_line"
CONFIG="$ROOT/$CONFIG_REL"
BLOB="$ROOT/$BLOB_REL"

# Everything a split writes (paths relative to repo root; see the config).
# Each must be git-ignored before we run anything.
OUTPUT_PATHS=(
    "$ASM_DIR"
    "$ASSET_DIR"
    "linkers/overlays/$OVL_ID.ld"
    "$ASM_DIR/.splache"
    "$ASM_DIR/undefined_funcs_auto.txt"
    "$ASM_DIR/undefined_syms_auto.txt"
    "include/gte_macros.inc"
    "include/include_asm.h"
    "include/labels.inc"
    "include/macro.inc"
)

if [[ ! -f "$CONFIG" ]]; then
    echo "ERROR: missing $CONFIG" >&2
    exit 1
fi
# The config must point splat's side outputs into the overlay's own directory.
for key in cache_path undefined_funcs_auto_path undefined_syms_auto_path; do
    if ! grep -qE "^\s*$key:\s*$ASM_DIR/" "$CONFIG"; then
        echo "ERROR: $CONFIG must set options.$key under $ASM_DIR/ (an overlay split must not overwrite the EXE's splat side outputs)." >&2
        exit 1
    fi
done

# 3. Extracted blob present.
if [[ ! -f "$BLOB" ]]; then
    echo "ERROR: extracted overlay not found: $BLOB" >&2
    echo "Run 'scripts/extract_overlays.sh' first (needs build/extracted/disc1/PE.IMG or your disc 1 image under rom/image/)." >&2
    exit 1
fi

# 4. Blob hash matches the manifest.
actual_sha1="$(sha1sum "$BLOB" | cut -d' ' -f1)"
if [[ "$actual_sha1" != "$EXPECTED_SHA1" ]]; then
    echo "ERROR: SHA-1 mismatch for $BLOB" >&2
    echo "  expected: $EXPECTED_SHA1 (configs/USA/overlays/manifest.yaml)" >&2
    echo "  actual:   $actual_sha1" >&2
    echo "Refusing to split an unverified input. Re-run scripts/extract_overlays.sh." >&2
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
echo "  target: $OVL_ID (vram $VRAM, size $SIZE)"
echo "  config: $CONFIG"
echo "  input:  $BLOB (SHA-1 verified: $actual_sha1)"
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
# hex-named, git-ignored generated units that are no longer current.
python3 "$ROOT/tools/build/disc1_plan.py" --root "$ROOT" --target "$OVL_ID" --cleanup-stale-asm

status_after="$(git -C "$ROOT" status --porcelain)"
# LC_ALL=C: comm compares byte-wise while sort honours LC_COLLATE.
new_entries="$(LC_ALL=C comm -13 <(LC_ALL=C sort <<<"$status_before") <(LC_ALL=C sort <<<"$status_after"))"
if [[ -n "$new_entries" ]]; then
    echo "ERROR: the split created files git does not ignore:" >&2
    echo "$new_entries" >&2
    echo "Do NOT commit these. Extend .gitignore (and OUTPUT_PATHS above), then re-check." >&2
    exit 1
fi

echo
echo "Split complete for $OVL_ID. Output is local-only and git-ignored (verified: no new"
echo "tracked/untracked entries in git status)."
echo "This is a study artifact only — no matching or rebuild claims."
