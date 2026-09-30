#!/usr/bin/env bash
# scripts/exact_rebuild_overlay.sh <id> — one-command exact-rebuild gate for
# one PE.IMG overlay (the overlay twin of scripts/exact_rebuild.sh).
#
# Runs the overlay matching sequence end to end and proves the packed
# candidate equals the extracted retail overlay byte-for-byte:
#
#   preflight (fast + deep) -> scripts/split_overlay.sh <id>
#     -> tools/build/disc1_build.py --target <id>
#     -> tools/build/disc1_verify.py --target <id>
#
# For an all-`asm`/`bin` config this is the split -> reassemble -> SHA-1
# round-trip proof; every `c` span the config later gains is compiled with its
# per-leaf profile and must pack back to the retail bytes exactly like the EXE
# lane's leaves. On success it prints one self-describing line:
#
#   EXACT_REBUILD_GATE[<id>]=PASS plan=<sha256> yaml=<sha256> spans=[..] sha1_orig=<sha1> sha1_cand=<sha1> sha1=<manifest sha1>
#
# Exit codes:
#   0  PASS — split+build produced the manifest SHA-1 and verify passed
#   1  FAIL — a step failed, an ignore violation, or an input changed mid-run
#   2  ENV  — a prerequisite is missing (toolchain / splat / extracted blob)
#
# Usage:
#   scripts/exact_rebuild_overlay.sh <id> [--preflight-fast]
set -euo pipefail

if [[ $# -lt 1 || $# -gt 2 ]]; then
    printf 'usage: %s <overlay-id> [--preflight-fast]\n' "$0" >&2
    exit 2
fi
OVL_ID="$1"
PREFLIGHT_DEEP=1
case "${2:-}" in
    "") ;;
    --preflight-fast) PREFLIGHT_DEEP=0 ;;
    *) printf 'usage: %s <overlay-id> [--preflight-fast]\n' "$0" >&2; exit 2 ;;
esac
case "${EXACT_REBUILD_GATE_PREFLIGHT:-}" in
    deep) PREFLIGHT_DEEP=1 ;;
    fast) PREFLIGHT_DEEP=0 ;;
    "") ;;
    *) printf 'EXACT_REBUILD_GATE_PREFLIGHT must be fast|deep\n' >&2; exit 2 ;;
esac

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

say()  { printf '%s\n' "$*"; }
fail() { printf 'EXACT_REBUILD_GATE[%s]=FAIL %s\n' "$OVL_ID" "$*" >&2; exit 1; }
envfail() { printf 'EXACT_REBUILD_GATE[%s]=ENV %s\n' "$OVL_ID" "$*" >&2; exit 2; }

PREFLIGHT="tools/build/disc1_preflight.py"
BUILD="tools/build/disc1_build.py"
VERIFY="tools/build/disc1_verify.py"
PLAN="tools/build/disc1_plan.py"

# Leaked per-leaf build knobs would silently change every leaf; strip loudly
# (same guard as scripts/exact_rebuild.sh).
if env | grep -qE '^(MASPSX_|ERA_)'; then
    leaked_knobs="$(env | awk -F= '/^(MASPSX_|ERA_)/{printf "%s ", $1}')"
    say "  NOTE stripping leaked build knobs from this shell: $leaked_knobs"
    while IFS= read -r _knob; do
        unset "$_knob"
    done < <(env | awk -F= '/^(MASPSX_|ERA_)/{print $1}')
fi

# 1. Repo root + python.
[[ -f "$ROOT/CLAUDE.md" && -f "$ROOT/configs/USA/overlays/manifest.yaml" ]] \
    || envfail "not a Parasite-Eve-Decompilation root: $ROOT"
command -v python3 >/dev/null || envfail "python3 not found on PATH"

# 2. Resolve the target (config, blob, manifest SHA-1).
if ! target_line="$(python3 - "$ROOT" "$OVL_ID" <<'PY'
import sys
from pathlib import Path
root, ovl_id = Path(sys.argv[1]), sys.argv[2]
sys.path.insert(0, str(root / "tools/build"))
from overlay_targets import TargetError, load_target
try:
    t = load_target(ovl_id, root=root)
except TargetError as exc:
    print(str(exc), file=sys.stderr)
    raise SystemExit(1)
if t.is_main:
    print("use scripts/exact_rebuild.sh for the executable", file=sys.stderr)
    raise SystemExit(1)
print(t.config, t.profiles, t.retail, t.expected_sha1, t.candidate)
PY
2>&1)"; then
    envfail "cannot resolve target '$OVL_ID': $target_line"
fi
read -r CONFIG PROFILES BLOB EXPECTED_SHA1 CANDIDATE <<<"$target_line"

# 3. Toolchain present (single resolver shared with the build).
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

# 4. Fast/deep preflight for this target's inputs.
preflight_args=(--target "$OVL_ID")
[[ "$PREFLIGHT_DEEP" -eq 1 ]] && preflight_args+=(--deep)
if ! preflight_out="$(python3 "$PREFLIGHT" "${preflight_args[@]}" 2>&1)"; then
    printf '%s\n' "$preflight_out" >&2
    fail "preflight: invalid build inputs (fix the findings above before split/build)"
fi
say "  preflight: ok ($(printf '%s\n' "$preflight_out" | tail -n1))"

# 5. Extracted blob + splat + ignore coverage via the split's own --check.
if ! split_check="$(scripts/split_overlay.sh "$OVL_ID" --check 2>&1)"; then
    envfail "split prerequisites failed:
$split_check"
fi

# 6. The YAML's sha1 must equal the manifest's (the plan enforces this too).
plan_sha1="$(
    python3 -c '
import sys
sys.path.insert(0, "tools/build")
from pathlib import Path
from disc1_plan import build_plan
from overlay_targets import load_target
t = load_target(sys.argv[1], root=Path("."))
print(build_plan(root=Path("."), target=t)["expected_sha1"])
' "$OVL_ID"
)"
[[ "$plan_sha1" == "$EXPECTED_SHA1" ]] \
    || envfail "plan expected_sha1=$plan_sha1 != manifest sha1=$EXPECTED_SHA1"

yaml_sha() { sha256sum "$CONFIG" | awk '{print $1}'; }
inputs_sha() {
    python3 - "$OVL_ID" <<'PY'
import hashlib, sys
from pathlib import Path
sys.path.insert(0, "tools/build")
from disc1_plan import build_plan
from overlay_targets import load_target
t = load_target(sys.argv[1], root=Path("."))
plan = build_plan(root=Path("."), target=t)
h = hashlib.sha256()
for rel in (plan["authority"], plan["build_profiles"]):
    h.update(rel.encode()); h.update(b"\0"); h.update(Path(rel).read_bytes())
for unit in plan["units"]:
    if unit["kind"] != "c":
        continue
    h.update(unit["source"].encode()); h.update(b"\0"); h.update(Path(unit["source"]).read_bytes())
print(h.hexdigest())
PY
}

say "== exact rebuild gate [$OVL_ID] =="
say "  root:      $ROOT"
say "  toolchain: $toolchain_note"
say "  config:    $CONFIG"
say "  retail:    $BLOB"
say "  expected:  SHA-1 $EXPECTED_SHA1"
say ""

logs="$(mktemp -d)"
trap 'rm -rf "$logs"' EXIT

H_START="$(yaml_sha)"

say "[1/3] scripts/split_overlay.sh $OVL_ID"
if ! scripts/split_overlay.sh "$OVL_ID" >"$logs/split.log" 2>&1; then
    grep -E '^ERROR' "$logs/split.log" >&2 || true
    fail "split_overlay.sh exit != 0; see $logs/split.log"
fi
H_AFTER_SPLIT="$(yaml_sha)"
[[ "$H_START" == "$H_AFTER_SPLIT" ]] || fail "race: YAML changed during the split; re-run"
say "      ok (YAML sha256 $H_AFTER_SPLIT)"

I_BEFORE_BUILD="$(inputs_sha)"

say "[2/3] tools/build/disc1_build.py --target $OVL_ID"
if ! python3 "$BUILD" --target "$OVL_ID" >"$logs/build.log" 2>&1; then
    grep -E '^ERROR|NON-MATCH' "$logs/build.log" >&2 || true
    fail "disc1_build.py --target $OVL_ID exit != 0; see $logs/build.log"
fi
H_AFTER_BUILD="$(yaml_sha)"
orig_sha1="$(grep -m1 'orig SHA-1:' "$logs/build.log" | awk '{print $3}' || true)"
cand_sha1="$(grep -m1 'cand SHA-1:' "$logs/build.log" | awk '{print $3}' || true)"
grep -q 'RESULT: EXACT MATCH' "$logs/build.log" \
    || fail "build did not report EXACT MATCH (orig=$orig_sha1 cand=$cand_sha1); see $logs/build.log"
say "      ok orig=$orig_sha1 cand=$cand_sha1 RESULT=EXACT_MATCH"

say "[3/3] tools/build/disc1_verify.py --target $OVL_ID"
if ! python3 "$VERIFY" --target "$OVL_ID" >"$logs/verify.log" 2>&1; then
    grep -E '^ERROR' "$logs/verify.log" >&2 || true
    if grep -q 'YAML C sources are not tracked' "$logs/verify.log"; then
        fail "verify: staging gap — a YAML C source under src/overlays/$OVL_ID/ is not git-tracked yet; the owner must 'git add' the leaf (the split+build above already packed to the retail SHA-1)"
    fi
    fail "disc1_verify.py --target $OVL_ID exit != 0; see $logs/verify.log"
fi
H_AFTER_VERIFY="$(yaml_sha)"
grep -q "VERIFY_TARGET\[$OVL_ID\]=PASS" "$logs/verify.log" \
    || fail "verify did not report VERIFY_TARGET[$OVL_ID]=PASS; see $logs/verify.log"
say "      ok VERIFY_TARGET[$OVL_ID]=PASS"

I_AFTER_VERIFY="$(inputs_sha)"

plan_line="$(python3 "$PLAN" --check --target "$OVL_ID")"
plan_sha_full="$(
    python3 -c '
import sys
sys.path.insert(0, "tools/build")
from pathlib import Path
from disc1_plan import build_plan
from overlay_targets import load_target
print(build_plan(root=Path("."), target=load_target(sys.argv[1], root=Path(".")))["plan_sha256"])
' "$OVL_ID"
)"
counts="$(printf '%s\n' "$plan_line" | sed -n 's/.*(\(.*\)).*/\1/p')"
H_FINAL="$(yaml_sha)"

say ""
say "  plan:      $plan_sha_full"
say "  yaml:      $H_FINAL"
say "  spans:     $counts"
say "  checkpoints: yaml start=$H_START split=$H_AFTER_SPLIT build=$H_AFTER_BUILD verify=$H_AFTER_VERIFY final=$H_FINAL"
say "               inputs build=$I_BEFORE_BUILD verify=$I_AFTER_VERIFY"

if [[ "$H_START" != "$H_AFTER_SPLIT" || "$H_AFTER_SPLIT" != "$H_AFTER_BUILD" \
      || "$H_AFTER_BUILD" != "$H_AFTER_VERIFY" || "$H_AFTER_VERIFY" != "$H_FINAL" ]]; then
    fail "race: YAML changed during the run; re-run"
fi
[[ "$I_BEFORE_BUILD" == "$I_AFTER_VERIFY" ]] \
    || fail "race: compiled sources changed during build/verify; re-run"
[[ "$orig_sha1" == "$EXPECTED_SHA1" && "$cand_sha1" == "$EXPECTED_SHA1" ]] \
    || fail "SHA-1 lines do not both equal the manifest (orig=$orig_sha1 cand=$cand_sha1)"

say ""
say "EXACT_REBUILD_GATE[$OVL_ID]=PASS plan=$plan_sha_full yaml=$H_FINAL spans=[$counts] sha1_orig=$orig_sha1 sha1_cand=$cand_sha1 sha1=$EXPECTED_SHA1"
exit 0
