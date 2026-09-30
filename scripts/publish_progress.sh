#!/usr/bin/env bash
# Generate and publish Parasite Eve progress dashboard to GitHub Pages.
#
# Target: Blizz127/parasite-eve-progress (gh-pages branch)
# Live URL: https://blizz127.github.io/parasite-eve-progress/
#
# Usage:
#   scripts/publish_progress.sh              # generate + publish
#   scripts/publish_progress.sh --dry-run    # generate only, print paths
#
# Requires: python3, PyYAML, git, gh (authenticated), network for push.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PROGRESS_REPO="${PROGRESS_REPO:-parasite-eve-progress}"
PROGRESS_REMOTE="${PROGRESS_REMOTE:-https://github.com/Blizz127/${PROGRESS_REPO}.git}"
PROGRESS_DIR="${PROGRESS_DIR:-$HOME/Projects/${PROGRESS_REPO}}"
DRY_RUN=0

if [[ "${1:-}" == "--dry-run" ]]; then
    DRY_RUN=1
fi

die() { echo "ERROR: $*" >&2; exit 1; }
info() { echo "  $*"; }

cd "$ROOT"

command -v python3 >/dev/null || die "python3 not found"
command -v git >/dev/null || die "git not found"

if [[ "$DRY_RUN" -eq 0 ]]; then
    command -v gh >/dev/null || die "gh CLI not found (needed for publish)"
fi

echo "=== Generate progress.json ==="
python3 "$ROOT/tools/progress/generate_progress.py" \
    -o "$ROOT/build/progress.json"

echo "=== Render dashboard ==="
python3 "$ROOT/tools/progress/render_dashboard.py" \
    "$ROOT/build/progress.json" \
    -o "$ROOT/build/progress.html"

# Standalone progress.json copy for xenogears-style dual-file publish.
cp "$ROOT/build/progress.json" "$ROOT/build/progress.publish.json"

if [[ "$DRY_RUN" -eq 1 ]]; then
    echo "=== Dry run complete ==="
    info "progress.json: $ROOT/build/progress.json"
    info "index.html:      $ROOT/build/progress.html"
    info "Would publish to: $PROGRESS_DIR (branch gh-pages)"
    exit 0
fi

echo "=== Prepare progress repo checkout ==="
if [[ ! -d "$PROGRESS_DIR/.git" ]]; then
    info "Cloning $PROGRESS_REMOTE → $PROGRESS_DIR"
    git clone "$PROGRESS_REMOTE" "$PROGRESS_DIR"
fi

cd "$PROGRESS_DIR"

# Ensure gh-pages branch exists and is checked out.
if git show-ref --verify --quiet refs/heads/gh-pages; then
    git checkout gh-pages
elif git show-ref --verify --quiet refs/remotes/origin/gh-pages; then
    git checkout -b gh-pages origin/gh-pages
else
    git checkout --orphan gh-pages 2>/dev/null || git checkout -B gh-pages
    git rm -rf . 2>/dev/null || true
fi

# Copy publish artifacts.
cp "$ROOT/build/progress.html" "$PROGRESS_DIR/index.html"
cp "$ROOT/build/progress.publish.json" "$PROGRESS_DIR/progress.json"
touch "$PROGRESS_DIR/.nojekyll"

git add index.html progress.json .nojekyll

if git diff --cached --quiet; then
    info "No changes to publish."
else
    GIT_AUTHOR_NAME="$(cd "$ROOT" && git log -1 --format='%an')"
    GIT_AUTHOR_EMAIL="$(cd "$ROOT" && git log -1 --format='%ae')"
    export GIT_AUTHOR_NAME GIT_AUTHOR_EMAIL
    export GIT_COMMITTER_NAME="$GIT_AUTHOR_NAME"
    export GIT_COMMITTER_EMAIL="$GIT_AUTHOR_EMAIL"

    DEcomp_HEAD="$(cd "$ROOT" && git rev-parse --short HEAD)"
    LEAVES="$(python3 -c "import json; m=json.load(open('$ROOT/build/progress.json'))['measures']; print(m['matched_functions'])")"
    CODE_PCT="$(python3 -c "import json; m=json.load(open('$ROOT/build/progress.json'))['measures']; print(f\"{float(m['matched_code_percent']):.2f}\")")"

    git commit -m "$(cat <<EOF
Update progress dashboard (${LEAVES} C leaves, ${CODE_PCT}% code)

Source: Parasite-Eve-Decompilation @ ${DEcomp_HEAD}
EOF
)"
    git push origin gh-pages
    info "Pushed to origin/gh-pages"
fi

echo "=== Enable GitHub Pages (idempotent) ==="
gh api -X POST "repos/Blizz127/${PROGRESS_REPO}/pages" \
    -f source[branch]=gh-pages \
    -f source[path]=/ 2>/dev/null || \
gh api -X PUT "repos/Blizz127/${PROGRESS_REPO}/pages" \
    -f source[branch]=gh-pages \
    -f source[path]=/ 2>/dev/null || \
info "Pages already configured or API call skipped."

echo
echo "=== Published ==="
echo "  URL: https://blizz127.github.io/${PROGRESS_REPO}/"
echo "  Repo: https://github.com/Blizz127/${PROGRESS_REPO}"
