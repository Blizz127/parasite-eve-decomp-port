#!/usr/bin/env bash
# scripts/exact_rebuild.sh — one-command exact packed-rebuild gate.
#
# Runs the documented disc-1 matching sequence end to end and proves the packed
# candidate equals the retail executable byte-for-byte:
#
#   scripts/split_us.sh   -> scripts/build_us.sh -> scripts/verify_us.sh
#
# It is safe to run while sibling matching workers are editing the tree:
#
#   * The split's post-run git-status guard trips whenever a sibling creates a
#     file (src/func_*.c, docs/evidence/…) between its before/after snapshots.
#     That is classified here as a BENIGN RACE and does not need an opt-in flag.
#   * A real ignore-rule violation (a *split output path* appearing in git
#     status, i.e. generated output that is no longer git-ignored) still FAILS
#     hard, as does any change to the YAML or to the compiled sources during the
#     run — the proof can never straddle two tree states.
#
# On success it prints a self-describing summary line carrying the plan
# SHA-256, the YAML SHA-256, the span counts, and BOTH SHA-1s, so a PASS can
# never be misread as covering a later state. See
# docs/ai_context/EXACT_REBUILD_GATE.md.
#
# Usage:
#   scripts/exact_rebuild.sh              run the gate (auto-classify a split race)
#   scripts/exact_rebuild.sh --strict-split
#                                         fail on ANY split git-status trip
#   scripts/exact_rebuild.sh --allow-split-race
#                                         legacy alias; benign races already continue
#   scripts/exact_rebuild.sh --preflight-fast
#                                         skip the deep span-size compile check
#   scripts/exact_rebuild.sh --no-sweep
#                                         EXPLICIT, LOUD opt-out of the strong
#                                         full-image re-verification (default ON)
#   scripts/exact_rebuild.sh --sweep
#                                         request the strong sweep explicitly
#                                         (it is already the default)
#   scripts/exact_rebuild.sh --help
#
# The preflight validator (tools/build/disc1_preflight.py) runs before the
# expensive stages and can only add an early FAIL, never turn one into a PASS.
# It parses the YAML exactly as splat does and, by default, compiles every C
# leaf (~7s) to compare its emitted span size with the declared span.
#
# By default the gate ALSO runs the strong full-image sweep
# (tools/analysis/verify_matched_leaves.py) after its own PASS: it re-verifies
# every matched c span for size-exactness, link-exactness at the retail VMA,
# a real function terminator, and no interior `jr $ra` (a swallowed neighbour).
# That check is what the weak per-leaf helper `era_link_check.py` (used by
# check_leaf.sh) cannot do: it compares min(linked, size) words, so an inflated
# span prints LINK_EXACT. The sweep only ever adds a FAIL. `--no-sweep` (or
# EXACT_REBUILD_GATE_SWEEP=off) turns it off with a loud warning, so relying on
# the weak check is a deliberate, visible choice.
#
# Leaked per-leaf build knobs (a persistent shell that once ran `export
# MASPSX_*`) are stripped before any child runs: the build layers each leaf's
# own knobs over `os.environ`, so a stray export would silently change every
# leaf and could poison the proof. The strip is reported loudly.
#
# Exit codes:
#   0  PASS — split+build produced the retail SHA-1 and verify passed
#   1  FAIL — a step failed, a real ignore violation, or an input changed mid-run
#   2  ENV  — a prerequisite is missing (toolchain / splat / retail image)
set -euo pipefail

EXPECTED_SHA1="452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
CONFIG="configs/USA/disc1.yaml"
PROFILES="configs/USA/disc1_build_profiles.json"
EXE="build/extracted/disc1/SLUS_006.62"
ROUTE_TOOL="tools/analysis/route_coverage.py"
PREFLIGHT="tools/build/disc1_preflight.py"
SWEEP="tools/analysis/verify_matched_leaves.py"

STRICT_SPLIT=0
# Default ON: the strong full-image sweep runs after the gate's own PASS and
# can only make the gate stricter. --no-sweep / EXACT_REBUILD_GATE_SWEEP=off
# opts out loudly.
RUN_SWEEP=1
# Default deep is ON: it costs ~7s at ~500 leaves and catches the span-size
# defect class that otherwise only trips trim_elf_section_pad.py after a
# ~30-minute split. EXACT_REBUILD_GATE_PREFLIGHT=fast|off disables it.
PREFLIGHT_DEEP=1
case "${1:-}" in
    --help|-h)
        # Print the leading comment block only (stop at first non-comment line).
        awk 'NR == 1 { next } /^#/ { sub(/^# ?/, ""); print; next } { exit }' \
            "${BASH_SOURCE[0]}"
        exit 0
        ;;
    --strict-split) STRICT_SPLIT=1 ;;
    --allow-split-race) : ;;  # legacy: benign races now continue by default
    --preflight-fast) PREFLIGHT_DEEP=0 ;;
    --no-sweep) RUN_SWEEP=0 ;;
    --sweep) RUN_SWEEP=1 ;;
    "") ;;
    *) printf 'usage: %s [--strict-split|--allow-split-race|--preflight-fast|--no-sweep|--sweep]\n' "$0" >&2; exit 2 ;;
esac
# Env override lets CI/wrappers request deep without changing the CLI.
case "${EXACT_REBUILD_GATE_PREFLIGHT:-}" in
    deep) PREFLIGHT_DEEP=1 ;;
    fast|"") ;;
    off) PREFLIGHT_DEEP=-1 ;;
    *) printf 'EXACT_REBUILD_GATE_PREFLIGHT must be fast|deep|off\n' >&2; exit 2 ;;
esac
case "${EXACT_REBUILD_GATE_SWEEP:-}" in
    on|"") ;;
    off) RUN_SWEEP=0 ;;
    *) printf 'EXACT_REBUILD_GATE_SWEEP must be on|off\n' >&2; exit 2 ;;
esac

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

say()  { printf '%s\n' "$*"; }
fail() { printf 'EXACT_REBUILD_GATE=FAIL %s\n' "$*" >&2; exit 1; }
envfail() { printf 'EXACT_REBUILD_GATE=ENV %s\n' "$*" >&2; exit 2; }

# ---------------------------------------------------------------------------
# Environment guard. disc1_build.compile_era copies os.environ and layers each
# leaf's own knobs on top, so a leaked `export MASPSX_*` / `ERA_*` from a
# persistent shell would silently change unrelated leaves (a real hazard the
# matching worker hit) and could poison this proof. Strip them loudly. The
# toolchain itself is left to each child's own resolver (`find_toolchain`),
# so a genuinely missing toolchain still fails loudly as ENV.
# ---------------------------------------------------------------------------
if env | grep -qE '^(MASPSX_|ERA_)'; then
    leaked_knobs="$(env | awk -F= '/^(MASPSX_|ERA_)/{printf "%s ", $1}')"
    say "  NOTE stripping leaked build knobs from this shell (they would poison per-leaf build): $leaked_knobs"
    while IFS= read -r _knob; do
        unset "$_knob"
    done < <(env | awk -F= '/^(MASPSX_|ERA_)/{print $1}')
fi

# ---------------------------------------------------------------------------
# Preflight: fail loudly, before any expensive work, with the exact remedy.
# ---------------------------------------------------------------------------

# 1. Repo root guard.
if [[ ! -f "$ROOT/CLAUDE.md" || ! -f "$ROOT/$CONFIG" ]]; then
    envfail "not a Parasite-Eve-Decompilation root: $ROOT"
fi

# 2. Python driver present.
command -v python3 >/dev/null || envfail "python3 not found on PATH"

# 3. Toolchain present — reuse the build's own resolver so there is a single
#    source of truth for PATH -> repo-local -> distrobox.
if ! toolchain_note="$(
    python3 -c '
import sys
sys.path.insert(0, "tools/build")
from disc1_build import find_toolchain, BuildError
try:
    print(find_toolchain().note)
except BuildError as exc:
    print(str(exc), file=sys.stderr)
    raise SystemExit(1)
' 2>&1
)"; then
    envfail "mipsel toolchain unavailable: $toolchain_note
  remedy: scripts/setup_mipsel_host.sh   (rootless, pinned Debian trixie)"
fi

# 4. Fast input validation — seconds, not minutes. Catches a syntactically
#    invalid YAML (splat would abort at split) and C-span/profile mismatches
#    (trim_elf_section_pad.py would abort at build) before the expensive
#    stages run. This can only ADD an early FAIL: it never relaxes a check.
if [[ "$PREFLIGHT_DEEP" -ne -1 ]]; then
    preflight_args=()
    [[ "$PREFLIGHT_DEEP" -eq 1 ]] && preflight_args+=(--deep)
    if ! preflight_out="$(python3 "$PREFLIGHT" "${preflight_args[@]}" 2>&1)"; then
        printf '%s\n' "$preflight_out" >&2
        fail "preflight: invalid build inputs (fix the findings above before split/build)"
    fi
    say "  preflight: ok ($(printf '%s\n' "$preflight_out" | tail -n1))"
fi

# 5. Retail input + splat + all split prerequisites (splat, EXE hash, ignore
#    coverage) via the split script's own --check mode.
if ! split_check="$(scripts/split_us.sh --check 2>&1)"; then
    envfail "split prerequisites failed:
$split_check"
fi

# 6. YAML config and expected retail SHA-1 must agree with this gate.
plan_sha1="$(
    python3 -c '
import sys
sys.path.insert(0, "tools/build")
from disc1_plan import build_plan
from pathlib import Path
print(build_plan(root=Path("."))["expected_sha1"])
'
)"
if [[ "$plan_sha1" != "$EXPECTED_SHA1" ]]; then
    envfail "plan expected_sha1=$plan_sha1 != gate EXPECTED_SHA1=$EXPECTED_SHA1"
fi

# ---------------------------------------------------------------------------
# Pinned inputs.
#
# * yaml_sha    — the YAML alone. It must be identical before/after the split
#                 (splat reads it) and pinned at every later checkpoint.
# * inputs_sha  — YAML + build-profile manifest + exactly the compiled sources
#                 the plan references (its `c` spans), in plan order. It must be
#                 identical before the build and after the verify: the proof may
#                 not straddle an edit to a compiled input. A sibling's new,
#                 not-yet-registered draft under src/ is deliberately excluded —
#                 it is not an input to this build and cannot invalidate it.
# ---------------------------------------------------------------------------
yaml_sha() { sha256sum "$CONFIG" | awk '{print $1}'; }

inputs_sha() {
    python3 - <<'PY'
import hashlib, sys
from pathlib import Path

sys.path.insert(0, "tools/build")
from disc1_plan import build_plan

plan = build_plan(root=Path("."))
h = hashlib.sha256()
for rel in (plan["authority"], plan["build_profiles"]):
    h.update(rel.encode("utf-8"))
    h.update(b"\0")
    h.update(Path(rel).read_bytes())
for unit in plan["units"]:
    if unit["kind"] != "c":
        continue
    rel = unit["source"]
    h.update(rel.encode("utf-8"))
    h.update(b"\0")
    h.update(Path(rel).read_bytes())
print(h.hexdigest())
PY
}

# Split output paths (single source of truth: split_us.sh). Used to tell a real
# ignore-rule violation from a benign sibling file appearing mid-run.
mapfile -t OUTPUT_PATHS < <(
    awk '/^OUTPUT_PATHS=\(/{f=1;next} f&&/^\)/{exit} f{gsub(/^[[:space:]]*"?|"?[[:space:]]*$/,""); if($0!="")print}' \
        scripts/split_us.sh
)

is_output_path() {
    local p="$1" o
    for o in "${OUTPUT_PATHS[@]}"; do
        [[ "$p" == "$o" || "$p" == "$o/"* ]] && return 0
    done
    return 1
}

say "== exact rebuild gate =="
say "  root:      $ROOT"
say "  toolchain: $toolchain_note"
say "  expected:  SHA-1 $EXPECTED_SHA1"
say ""

logs="$(mktemp -d)"
# Keep the logs when the gate fails, so a FAIL stays diagnosable.
trap 'rc=$?; if [[ $rc -eq 0 ]]; then rm -rf "$logs"; else echo "gate logs kept in $logs" >&2; fi' EXIT

H_START="$(yaml_sha)"

# ---------------------------------------------------------------------------
# Step 1/3: split
# ---------------------------------------------------------------------------
say "[1/3] scripts/split_us.sh"
split_rc=0
scripts/split_us.sh >"$logs/split.log" 2>&1 || split_rc=$?
if [[ "$split_rc" -ne 0 ]]; then
    if grep -q 'the split created files git does not ignore' "$logs/split.log"; then
        # git status --porcelain format is "XY PATH"; collect the reported paths.
        new_entries="$(
            awk '/^ERROR: the split created files git does not ignore:/{f=1;next}
                 f && /^Do NOT commit these/{exit}
                 f {print}' "$logs/split.log"
        )"
        # A real violation is an entry under a split OUTPUT path: generated
        # output that is no longer git-ignored. Sibling-authored src/*.c or
        # docs/evidence/* files are untracked-by-design and benign *for the
        # ignore rule*; whether they also changed a plan input is decided by the
        # YAML/inputs pins below, not here.
        real_violations=""
        while IFS= read -r entry; do
            [[ -n "$entry" ]] || continue
            path="${entry:3}"
            path="${path%\"}"; path="${path#\"}"
            if is_output_path "$path"; then
                real_violations+="$entry"$'\n'
            fi
        done <<<"$new_entries"

        if [[ -n "$real_violations" ]]; then
            printf 'ERROR: split output is not git-ignored:\n%s' "$real_violations" >&2
            fail "real ignore-rule violation: the split created files under an OUTPUT path; fix .gitignore"
        fi
        if [[ "$STRICT_SPLIT" -eq 1 ]]; then
            fail "split git-status trip (--strict-split): ${new_entries//$'\n'/; }"
        fi
        say "      WARN split git-status trip: sibling file(s) appeared mid-run"
        say "           ${new_entries//$'\n'/; }"
        say "           (no split output path involved -> ignore rule intact;"
        say "            checking the plan did not move under us)"
    else
        grep -E '^ERROR' "$logs/split.log" >&2 || true
        fail "split_us.sh exit $split_rc != 0; see $logs/split.log"
    fi
fi
H_AFTER_SPLIT="$(yaml_sha)"
if [[ "$H_START" != "$H_AFTER_SPLIT" ]]; then
    fail "race: YAML changed during the split (splat read a moving plan); re-run"
fi
say "      ok (YAML sha256 $H_AFTER_SPLIT)"

# Baseline the compiled inputs only now: the split does not compile src/, so a
# benign race during the split must not invalidate the build/verify proof.
I_BEFORE_BUILD="$(inputs_sha)"

# ---------------------------------------------------------------------------
# Step 2/3: build + packed SHA-1 comparison
# ---------------------------------------------------------------------------
say "[2/3] scripts/build_us.sh"
if ! scripts/build_us.sh >"$logs/build.log" 2>&1; then
    grep -E '^ERROR|NON-MATCH' "$logs/build.log" >&2 || true
    fail "build_us.sh exit != 0; see $logs/build.log"
fi
H_AFTER_BUILD="$(yaml_sha)"
orig_sha1="$(grep -m1 'orig SHA-1:' "$logs/build.log" | awk '{print $3}' || true)"
cand_sha1="$(grep -m1 'cand SHA-1:' "$logs/build.log" | awk '{print $3}' || true)"
if ! grep -q 'RESULT: EXACT MATCH' "$logs/build.log"; then
    fail "build did not report EXACT MATCH (orig=$orig_sha1 cand=$cand_sha1); see $logs/build.log"
fi
say "      ok orig=$orig_sha1 cand=$cand_sha1 RESULT=EXACT_MATCH"

# ---------------------------------------------------------------------------
# Step 3/3: verify (all gates)
# ---------------------------------------------------------------------------
say "[3/3] scripts/verify_us.sh"
if ! scripts/verify_us.sh >"$logs/verify.log" 2>&1; then
    grep -E '^ERROR' "$logs/verify.log" >&2 || true
    if grep -q 'missing split' "$logs/verify.log"; then
        fail "verify: concurrent split race — a sibling split added/removed generated asm mid-run, so this run's split output is incomplete; re-run"
    fi
    if grep -q 'YAML C sources are not tracked' "$logs/verify.log"; then
        fail "verify: staging gap — a YAML C source is not git-tracked yet; owner must 'git add' the leaf"
    fi
    fail "verify_us.sh exit != 0; see $logs/verify.log"
fi
H_AFTER_VERIFY="$(yaml_sha)"
if ! grep -q 'VERIFY_US=PASS' "$logs/verify.log"; then
    fail "verify did not report VERIFY_US=PASS; see $logs/verify.log"
fi
say "      ok VERIFY_US=PASS"

# ---------------------------------------------------------------------------
# Self-consistency: YAML at every checkpoint, compiled inputs from before the
# build through the end of verify. A change here means the proof straddled a
# tree state and must not be trusted.
# ---------------------------------------------------------------------------
I_AFTER_BUILD="$(inputs_sha)"
I_AFTER_VERIFY="$(inputs_sha)"

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
plan_line="$(python3 tools/build/disc1_plan.py --check)"
plan_sha_full="$(
    python3 -c '
import sys
sys.path.insert(0, "tools/build")
from disc1_plan import build_plan
from pathlib import Path
print(build_plan(root=Path("."))["plan_sha256"])
'
)"
counts="$(printf '%s\n' "$plan_line" | sed -n 's/.*(\(.*\)).*/\1/p')"

# Informational only: never changes PASS semantics. Route coverage for the
# persistent boot→end-of-day-2 objective, computed at the proven plan hash.
route_line=""
if [[ -f "$ROUTE_TOOL" ]]; then
    if route_line="$(python3 "$ROUTE_TOOL" --quiet --plan "$plan_sha_full" 2>"$logs/route.log")"; then
        route_line="$(printf '%s\n' "$route_line" | tail -n1)"
    else
        route_line="(unavailable: $(sed -n '1p' "$logs/route.log" 2>/dev/null))"
    fi
fi

# Final YAML checkpoint: the plan hash just printed must belong to the same
# pinned state the proof scored, even if a sibling edited the YAML after verify.
H_FINAL="$(yaml_sha)"

say ""
say "  plan:      $plan_sha_full"
say "  yaml:      $H_FINAL"
say "  spans:     $counts"
say "  checkpoints: yaml start=$H_START split=$H_AFTER_SPLIT build=$H_AFTER_BUILD verify=$H_AFTER_VERIFY final=$H_FINAL"
say "               inputs split=$I_BEFORE_BUILD build=$I_AFTER_BUILD verify=$I_AFTER_VERIFY"

if [[ "$H_START" != "$H_AFTER_SPLIT" || "$H_AFTER_SPLIT" != "$H_AFTER_BUILD" \
      || "$H_AFTER_BUILD" != "$H_AFTER_VERIFY" || "$H_AFTER_VERIFY" != "$H_FINAL" ]]; then
    fail "race: YAML changed during the run (result is for a mixed tree state); re-run"
fi
if [[ "$I_BEFORE_BUILD" != "$I_AFTER_BUILD" || "$I_AFTER_BUILD" != "$I_AFTER_VERIFY" ]]; then
    fail "race: compiled sources changed during build/verify (result is for a mixed tree state); re-run"
fi

if [[ -n "$route_line" ]]; then
    say "  route:     $route_line"
fi

# Strong full-image re-verification (DEFAULT ON). It runs only after the checks
# above pass, so it can never turn a FAIL into a PASS; a failure here exits 1
# *before* the PASS line is printed, so a PASS can never be misread. The sweep
# re-verifies every matched c span for size-exactness, link-exactness at the
# retail VMA, a real terminator, and no interior `jr $ra` — the checks the weak
# per-leaf `era_link_check.py` cannot make.
if [[ "$RUN_SWEEP" -eq 1 ]]; then
    say ""
    say "[sweep] tools/analysis/verify_matched_leaves.py (strong, default)"
    sweep_log="$(mktemp)"
    if python3 "$SWEEP" >"$sweep_log" 2>&1; then
        say "      ok $(grep -E '^VERIFY_SWEEP=' "$sweep_log" | tail -n1)"
    else
        cat "$sweep_log" >&2
        rm -f "$sweep_log"
        fail "matched-leaf sweep did not pass (strong check); see output above. If you are mid-carve this is a real FAIL, not a race."
    fi
    rm -f "$sweep_log"
else
    say ""
    say "  WARN strong full-image sweep DISABLED (--no-sweep / EXACT_REBUILD_GATE_SWEEP=off)."
    say "       The per-leaf check_leaf.sh/era_link_check.py path compares min(linked,"
    say "       size) words and prints LINK_EXACT for an OVERSIZED span; this PASS does"
    say "       NOT independently re-verify the matched spans. Re-run without --no-sweep."
fi

say ""
say "EXACT_REBUILD_GATE=PASS plan=$plan_sha_full yaml=$H_FINAL spans=[$counts] sha1_orig=$orig_sha1 sha1_cand=$cand_sha1 sha1=$EXPECTED_SHA1"
if [[ "$RUN_SWEEP" -eq 0 ]]; then
    say "EXACT_REBUILD_GATE_SWEEP=skipped"
fi
if [[ -n "$route_line" && "$route_line" != "(unavailable:"* ]]; then
    say "ROUTE_COVERAGE=$route_line"
fi
exit 0
