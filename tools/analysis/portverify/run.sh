#!/usr/bin/env bash
# portverify: differential harness — matched-decomp generated TU vs the existing
# hand-written pc_port definition of the same leaf ("conflicts" reported by
# tools/analysis/gen_decomp_ports.py).
#
# For every conflict leaf: render its generated TU (without writing
# pc_port/game/decomp), rename the function to pv_<name>, compile it exactly like
# pc_port compiles decomp TUs (-std=gnu11 -O0 -funsigned-char), link it against
# the pc_port runtime library (which provides the hand definition), then run both
# in forked children on identical seeded guest RAM + scratchpad and arguments and
# compare full RAM, scratchpad and the return value (typed by the matched C).
#
# Verdicts (sweep.txt): EQUIV, DIVERGE, ABORTDIFF (exactly one side aborted on
# some trial; RAM/return agree elsewhere), INCONCL (<4 trials completed on both).
# A DIVERGE is only a *candidate*: confirm it against retail with
# tools/analysis/pe_dis.sh before changing anything (see
# docs/evidence/portverify/REPORT.md for the known false-positive classes).
#
# Usage: tools/analysis/portverify/run.sh [--lib] [TRIALS]
#   --lib    (re)build the pc_port library into $PORTVERIFY_WORK/pcbuild first
#   TRIALS   trials per leaf (default 192; 12 RAM-seeding modes)
# Env: PORTVERIFY_WORK (default build/portverify, git-ignored), TMPDIR.
set -euo pipefail
TOOL="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$TOOL/../../.." && pwd)"
W="${PORTVERIFY_WORK:-$ROOT/build/portverify}"
export PORTVERIFY_WORK="$W"
mkdir -p "$W"
if [[ "${1:-}" == "--lib" ]]; then
    shift
    "$ROOT/scripts/build_pc_port.sh" --build-dir "$W/pcbuild" --no-tests > "$W/lib.log" 2>&1 \
        || { echo "portverify: pc_port library build failed (see $W/lib.log)" >&2; exit 1; }
fi
LIB="$W/pcbuild/libpe_field_runtime.a"
[[ -f "$LIB" ]] || { echo "portverify: $LIB missing; run with --lib" >&2; exit 1; }
TRIALS="${1:-192}"
INC="-I$ROOT/pc_port -I$ROOT/pc_port/include -I$ROOT/pc_port/platform -I$ROOT/pc_port/bootstrap -I$ROOT/pc_port/src"
DEF="-DPE_PORT_FB_HEIGHT=240 -DPE_PORT_FB_WIDTH=320 -DPE_PORT_HEADLESS=1"

rm -rf "$W/gen" "$W/obj" "$W/ren" "$W/obj_x"; mkdir -p "$W/gen" "$W/obj" "$W/ren" "$W/obj_x"
python3 "$TOOL/dump_conflicts.py"
python3 "$TOOL/sigs.py"
python3 "$TOOL/audit_casts.py" "$W"/gen/*_port.c 2>/dev/null | awk '{print $1}' | sed 's/_port\.c$//' > "$W/unsafe.txt" || true

compile_one() {
    local f="$1" n; n="$(basename "$f" _port.c)"
    sed -E "s/\\b$n\\b/pv_$n/g" "$f" > "$W/ren/$n.c"
    if cc -std=gnu11 -O0 -w -funsigned-char $DEF $INC -c "$W/ren/$n.c" -o "$W/obj/$n.o" 2> "$W/ren/$n.err"; then
        rm -f "$W/ren/$n.err"
    else
        echo "compile-fail $n"
    fi
}
export -f compile_one; export W DEF INC
ls "$W"/gen/*_port.c | xargs -P"$(nproc)" -I{} bash -c 'compile_one {}' > "$W/compile_fail.txt" || true
echo "portverify: $(wc -l < "$W/compile_fail.txt") generated TU(s) do not compile renamed"

python3 "$TOOL/gen_driver.py" > /dev/null
cc -std=gnu11 -O1 -w -I"$TOOL" -c "$TOOL/harness.c" -o "$W/harness.o"
# Guest-code dispatch registry (PE_GuestCall targets): pe-native-tests and the
# game link it; without it, calls through guest code pointers become boundaries
# and the harness would compare a boundary against the hand port's real call.
REG="$ROOT/pc_port/game/decomp_ovl/_dispatch/zz_dispatch.c"
EXTRA=""
if [[ -f "$REG" ]]; then
    cc -std=gnu11 -O0 -w -funsigned-char $DEF $INC -c "$REG" -o "$W/zz_dispatch.o"
    EXTRA="$W/zz_dispatch.o"
fi
for _ in 1 2 3 4 5 6 7 8; do
    cc -std=gnu11 -O0 -w -I"$TOOL" -c "$W/driver_table.c" -o "$W/driver_table.o"
    if out="$(cc -o "$W/pvh" "$W/harness.o" "$W/driver_table.o" "$W"/obj/*.o $EXTRA "$LIB" -lm -ldl 2>&1)"; then break; fi
    bad="$(echo "$out" | grep -o 'obj/func_[0-9A-F]*\.o' | sort -u || true)"
    [[ -n "$bad" ]] || { echo "$out" | head -20 >&2; exit 1; }
    for b in $bad; do echo "portverify: link-excluded $(basename "$b" .o) (callee is file-static in pc_port)"; mv "$W/$b" "$W/obj_x/"; done
    python3 "$TOOL/gen_driver.py" > /dev/null
done

grep -o '"func_[0-9A-F]*"' "$W/driver_table.c" | tr -d '"' > "$W/names.txt"
rm -f "$W"/chunk_* "$W"/res_chunk_*
( cd "$W" && split -n l/8 names.txt chunk_ )
# Parallel chunks via xargs (no shell `&` job control; every chunk is waited on).
ls "$W"/chunk_* | xargs -P8 -I{} sh -c '"$0/pvh" "$1" "$(paste -sd, "$2")" > "$0/res_$(basename "$2").txt" 2>/dev/null || true' "$W" "$TRIALS" {}
cat "$W"/res_chunk_* | sort > "$W/sweep.txt"
awk '{print $1}' "$W/sweep.txt" | sort | uniq -c
