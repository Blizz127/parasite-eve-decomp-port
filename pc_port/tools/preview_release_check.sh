#!/usr/bin/env bash
# Preview release check: build the current tree, compare the Day-1 route
# against the owner-confirmed baseline, run the normal-start aid check and a
# no-skip FMV boot smoke, then package the preview (PE_PREVIEW_DEFAULT_SKIP=0).
#
# Exit codes: 0 all good (and packaged), 1 new build misses a checkpoint the
# baseline reached, 3 route aids active on a normal start, 4 FMV boot smoke
# failed, 5 build failed, 6 packaging failed.
#
# Usage: pc_port/tools/preview_release_check.sh [--skip-build | --bin PATH] [--until CP]
#            [--max-frames N] [--no-package] [--force-package]
# Env: CMAKE (default build/lanes/port/venv/bin/cmake), DISC, BASELINE_BIN.
# Outputs: build/lanes/day1/release-check/ (logs, table.txt), package in
#          build/lanes/day1/release-check/dist. No git ops, no deploy.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"
export TMPDIR="${TMPDIR:-$ROOT/build/tmp}"
mkdir -p "$TMPDIR"

NEW_BIN_OVERRIDE=""; SKIP_BUILD=0; UNTIL=day1_end_80; MAX_FRAMES=120000; PACKAGE=1; FORCE_PACKAGE=0
while [[ $# -gt 0 ]]; do
    case "$1" in
        --skip-build) SKIP_BUILD=1; shift ;;
        --until) UNTIL="$2"; shift 2 ;;
        --max-frames) MAX_FRAMES="$2"; shift 2 ;;
        --no-package) PACKAGE=0; shift ;;
        --bin) NEW_BIN_OVERRIDE="$2"; SKIP_BUILD=1; shift 2 ;;
        --force-package) FORCE_PACKAGE=1; shift ;;
        *) echo "unknown option: $1" >&2; exit 2 ;;
    esac
done

OUT=build/lanes/day1/release-check
BUILD_DIR=build/pcbuild-release
NEW_BIN=${NEW_BIN_OVERRIDE:-$BUILD_DIR/parasite-eve-port}
BASELINE_BIN="${BASELINE_BIN:-build/baseline/owner-2026-09-28/parasite-eve-port}"
DISC="${DISC:-rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin}"
export CMAKE="${CMAKE:-$ROOT/build/lanes/port/venv/bin/cmake}"
export JOBS="${JOBS:-2}"   # memory-limited box: cap build parallelism
REGRESS=pc_port/tools/day1_route_regress.sh
mkdir -p "$OUT"
echo "== preview_release_check $(date -u +%FT%TZ) until=$UNTIL max_frames=$MAX_FRAMES"

# 1. Build.
if [[ $SKIP_BUILD -eq 0 ]]; then
    echo "== build -> $BUILD_DIR (log $OUT/build.log)"
    if ! scripts/build_pc_port.sh --no-tests --build-dir "$BUILD_DIR" > "$OUT/build.log" 2>&1; then
        echo "RELEASE_CHECK FAIL build (see $OUT/build.log)"; tail -5 "$OUT/build.log"; exit 5
    fi
fi
[[ -x "$NEW_BIN" ]] || { echo "RELEASE_CHECK FAIL no binary $NEW_BIN"; exit 5; }
[[ -x "$BASELINE_BIN" ]] || { echo "RELEASE_CHECK FAIL no baseline $BASELINE_BIN"; exit 5; }

# 2+3. Route regress (new build includes the normal-start aid check; exit 3).
set +e
echo "== route: new build"
"$REGRESS" --bin "$NEW_BIN" --until "$UNTIL" --max-frames "$MAX_FRAMES" \
    --log "$OUT/route-new.log" > "$OUT/regress-new.txt" 2>&1
NEW_RC=$?
echo "== route: baseline"
"$REGRESS" --bin "$BASELINE_BIN" --skip-normal-start-check --until "$UNTIL" \
    --max-frames "$MAX_FRAMES" --log "$OUT/route-baseline.log" > "$OUT/regress-baseline.txt" 2>&1
BASE_RC=$?
set -e
cat "$OUT/regress-new.txt" "$OUT/regress-baseline.txt" | grep -E "REGRESS (FAIL|PASS)" || true

if [[ $NEW_RC -eq 3 ]]; then
    echo "RELEASE_CHECK FAIL normal-start aid check: route aids active without --route-pad"; exit 3
fi

# Side-by-side table; fail if the new build misses a checkpoint the baseline reached.
set +e
python3 - "$OUT/regress-baseline.txt" "$OUT/regress-new.txt" "$OUT/table.txt" <<'EOF'
import re, subprocess, sys
base, new, table = sys.argv[1:4]
names = [l.split()[0] for l in subprocess.run(
    ["python3", "pc_port/tools/day1_route_regress.py", "--list"],
    capture_output=True, text=True, check=True).stdout.splitlines()]
def parse(path):
    got, fail = {}, ""
    for l in open(path, errors="replace"):
        m = re.match(r"REGRESS ok\s+(\S+)\s+frame=(\d+) token=(\S+) story=(\S+)", l)
        if m: got[m.group(1)] = (int(m.group(2)), m.group(3), m.group(4))
        if l.startswith("REGRESS FAIL"): fail = l.strip()
    return got, fail
b, bf = parse(base); n, nf = parse(new)
rows = ["%-16s %-28s %-28s %s" % ("checkpoint", "baseline", "new", "")]
bad = []
for c in names:
    if c not in b and c not in n: continue
    fmt = lambda d: ("frame=%d %s/%s" % d[c]) if c in d else "-"
    flag = ""
    if c in b and c not in n: flag = "REGRESSION"; bad.append(c)
    elif c in n and c not in b: flag = "new"
    rows.append("%-16s %-28s %-28s %s" % (c, fmt(b), fmt(n), flag))
rows.append("baseline: " + (bf or "PASS"))
rows.append("new:      " + (nf or "PASS"))
out = "\n".join(rows)
open(table, "w").write(out + "\n")
print(out)
sys.exit(1 if bad else 0)
EOF
TABLE_RC=$?
set -e

# 4. No-skip FMV boot smoke: New Game from the title with the real movie path.
echo "== fmv boot smoke (no skip)"
set +e
PE_PAD_SCRIPT="1000:FFF7,1010:FFFF,1300:FFF7,1310:FFFF,1450:FFEF,1460:FFFF,1500:BFFF,1510:FFFF" \
PE_FMV_LOG=1 timeout 900 "$NEW_BIN" --headless --max-frames 4000 \
    --screenshot "$OUT/fmv-smoke.ppm" --disc-image "$DISC" > /dev/null 2> "$OUT/fmv-smoke.log"
FMV_RC=$?
set -e
FMV_OK=1
grep -aq "\[FMV\] stream open" "$OUT/fmv-smoke.log" || FMV_OK=0
grep -aq "stop_reason=frame-limit" "$OUT/fmv-smoke.log" || FMV_OK=0
grep -aqi "unresolved" "$OUT/fmv-smoke.log" && FMV_OK=0
echo "fmv smoke: rc=$FMV_RC ok=$FMV_OK $(grep -a 'stop_reason' "$OUT/fmv-smoke.log" | tail -1) $(grep -ac '\[FMV\] stream' "$OUT/fmv-smoke.log") stream lines"

STATUS=0
[[ $TABLE_RC -ne 0 ]] && STATUS=1
[[ $FMV_OK -eq 0 && $STATUS -eq 0 ]] && STATUS=4

# 5. Package (only when every check passed, unless forced).
if [[ $PACKAGE -eq 1 && ( $STATUS -eq 0 || $FORCE_PACKAGE -eq 1 ) ]]; then
    echo "== package (PE_PREVIEW_DEFAULT_SKIP=0) -> $OUT/dist"
    if ! PE_BINARY="$NEW_BIN" PE_PREVIEW_DEFAULT_SKIP=0 PE_OUTPUT_DIR="$ROOT/$OUT/dist" \
         PE_SOURCE_NOTE="preview_release_check status=$STATUS" \
         pc_port/package_preview.sh > "$OUT/package.log" 2>&1; then
        echo "RELEASE_CHECK FAIL packaging (see $OUT/package.log)"; exit 6
    fi
    tail -3 "$OUT/package.log"
else
    echo "== package skipped (status=$STATUS; --force-package to override)"
fi
echo "RELEASE_CHECK status=$STATUS table=$OUT/table.txt"
exit $STATUS
