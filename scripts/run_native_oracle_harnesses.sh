#!/usr/bin/env bash
# Retail-oracle differentials for three native ports. The tested executables
# hold only the port's C and a test transport; the retained Python MIPS
# oracle (tools/analysis/e21_diff.py, test-only) executes the retail EXE
# bytes for the other side.
#   font:      func_80038D74 font-page initializer, five callees mocked
#   animation: func_80039B74/D24/ED4 with the real rotation func_800794C4
#   field VM:  E21-generated func_800133E8 / func_80019810, math mocked
#
# usage: scripts/run_native_oracle_harnesses.sh [EXE] [OUT_DIR]
#   EXE      retail SLUS_006.62 (default build/extracted/disc1/SLUS_006.62)
#   OUT_DIR  results directory (default: a fresh mktemp dir under TMPDIR)
# PYTHON must have pycparser (tools/analysis/guest_lower.py). The VM TUs are
# generated: run scripts/build_pc_port.sh or gen_decomp_ports.py --e21 first.
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
EXE=${1:-$ROOT/build/extracted/disc1/SLUS_006.62}
OUT=${2:-$(mktemp -d "${TMPDIR:-/tmp}/pe-native-oracle.XXXXXX")}
PYTHON=${PYTHON:-python3}
PC=$ROOT/pc_port
TA=$ROOT/tools/analysis
mkdir -p "$OUT"
[ -f "$EXE" ] || { echo "missing retail EXE: $EXE" >&2; exit 1; }
for f in func_800133E8 func_80019810; do
    [ -f "$PC/game/decomp/${f}_port.c" ] ||
        { echo "missing generated $f TU (gen_decomp_ports.py --e21)" >&2; exit 1; }
done
INC=(-I"$PC/include" -I"$PC/platform" -I"$PC/bootstrap" -I"$PC")
# -Wno-comment: pe_port_compat.h has "game/boot/*.c" inside a comment.
CFLAGS=(-O2 -std=gnu11 -Wall -Wextra -Werror -Wno-comment -funsigned-char)
run() { (ulimit -v 2097152; nice -n 19 timeout 900 "$@"); }

echo "== font initializer"
run cc "${CFLAGS[@]}" "${INC[@]}" "$PC/game/decomp_hand/font_page_init_port.c" \
    "$TA/font_native/harness.c" -o "$OUT/font-native-tests"
run "$PYTHON" -I "$TA/font_native/validate.py" --base "$ROOT" --exe "$EXE" \
    --native "$OUT/font-native-tests" \
    --body-sha256 450c39072acb73b703e0a3810d97963608e73aa33b14baa1aa11d83f2ce85005 \
    --output "$OUT/font-results.json"

echo "== animation decoders + rotation"
run cc "${CFLAGS[@]}" -ffunction-sections -fdata-sections -Wl,--gc-sections "${INC[@]}" \
    "$PC/game/decomp_hand/animation_decode_port.c" "$PC/game/boot/func_800794C4_port.c" \
    "$TA/animation_native/harness.c" -o "$OUT/animation-native-tests"
run "$PYTHON" -I "$TA/animation_native/validate.py" --base "$ROOT" --exe "$EXE" \
    --native "$OUT/animation-native-tests" --pins "$TA/animation_native/retail-pins.json" \
    --output "$OUT/animation-results.json"

echo "== field-VM handlers (E21-generated)"
# -Wno-unused-variable: the generated TUs keep the matched leaf's locals.
run cc "${CFLAGS[@]}" -Wno-unused-variable "${INC[@]}" \
    "$PC/game/decomp/func_800133E8_port.c" "$PC/game/decomp/func_80019810_port.c" \
    "$TA/field_vm_native/harness.c" "$TA/field_vm_native/trap_shim.c" \
    -o "$OUT/field-vm-native-tests"
run "$PYTHON" -I "$TA/field_vm_native/validate.py" --base "$ROOT" --exe "$EXE" \
    --native "$OUT/field-vm-native-tests" --output "$OUT/vm-results.json"

echo "NATIVE_ORACLE_HARNESSES=PASS out=$OUT"
