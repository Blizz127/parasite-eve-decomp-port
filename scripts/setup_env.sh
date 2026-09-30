#!/usr/bin/env bash
# Set up the local Python toolchain for Phase 2+ splitting: a git-ignored
# virtualenv at .venv/ with a pinned splat (PyPI: splat64) install.
#
# Idempotent: safe to re-run; it reuses .venv/ and (re)installs the exact
# pinned version. Touches nothing git-tracked and no game data.
#
# Pin rationale: 0.41.0 was the latest splat64 release on PyPI as of
# 2026-07-05 (evidence: `curl https://pypi.org/pypi/splat64/json`, recorded
# in docs/ai_context/ACTIVE_HANDOFF.md). Bump deliberately and record the
# bump + reason in the handoff changelog.
set -euo pipefail

SPLAT_PIN="0.41.0"

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# Repo-root sanity: refuse to run from a stray copy of this script — .venv/
# must land at the real repo root, where .gitignore covers it.
if [[ ! -f "$ROOT/CLAUDE.md" || ! -d "$ROOT/configs/USA" ]]; then
    echo "ERROR: $ROOT does not look like the Parasite-Eve-Decompilation root." >&2
    exit 1
fi
if ! git -C "$ROOT" rev-parse --show-toplevel >/dev/null 2>&1; then
    echo "ERROR: $ROOT is not inside a git repository." >&2
    exit 1
fi

VENV="$ROOT/.venv"

if [[ ! -d "$VENV" ]]; then
    echo "Creating virtualenv: $VENV"
    python3 -m venv "$VENV" || {
        echo "ERROR: 'python3 -m venv' failed (missing python3-venv/ensurepip?)." >&2
        exit 1
    }
else
    echo "Reusing existing virtualenv: $VENV"
fi

echo "Installing splat64[mips]==$SPLAT_PIN (pinned) ..."
"$VENV/bin/pip" install --quiet --upgrade pip
"$VENV/bin/pip" install --quiet "splat64[mips]==$SPLAT_PIN"

if [[ ! -x "$VENV/bin/splat" ]]; then
    echo "ERROR: install finished but $VENV/bin/splat is missing." >&2
    exit 1
fi
installed="$("$VENV/bin/pip" show splat64 | awk '/^Version:/{print $2}')"
echo "OK: splat64 $installed at $VENV/bin/splat"
echo
echo "Next steps:"
echo "  scripts/split_us.sh --check   # dry-run: verify prerequisites, generate nothing"
echo "  scripts/split_us.sh           # run the first split (all output git-ignored)"
