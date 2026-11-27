#!/usr/bin/env bash
# Build every sw-vec-gpu2 checker with EACH build line from its own header (verbatim apart from the output name and the
# optional -DGPU2_MUTANT=n), run it against the vectors (forward and reversed), run the mutants, and prove a build
# without the provider files fails to link.  Usage (repo root):
#   pc_port/tools/pe_gpu2_run_checks.sh OUTDIR VECTOR_DIR [EXPAND_VECTORS EXPAND_BASE]
set -uo pipefail
declare -A ENT=([gpu2d]='8010C0D8 8010BE3C')
TOOLS=pc_port/tools
OUT=${1:?outdir}; VEC=${2:?vector dir}; EXPV=${3:-}; EXPB=${4:-}
mkdir -p "$OUT"; export TMPDIR=${TMPDIR:-$OUT}
OVL=build/extracted/disc1/peimg/ovl_039F.bin
for b in ${G2_BATCHES:-gpu2a gpu2b gpu2c gpu2d gpu2e}; do
  src=pc_port/tools/pe_gpu2_${b}_check.c
  vf="$VEC/$b/$b.txt"
  tac "$vf" > "$OUT/${b}_reversed.txt"
  i=0
  grep -E '^ \*   cc ' "$src" | sed -E 's/^ \*   //' | while IFS= read -r line; do
    i=$((i+1))
    for m in 0 1 2 3; do
      exe="$OUT/check_${b}_l${i}_m$m"
      flags=""; [ "$m" != 0 ] && flags="-DGPU2_MUTANT=$m"
      cmd=$(printf '%s' "$line" | sed -E "s# -o [^ ]+\$# $flags -o $exe#")
      echo "== $b line$i mutant=$m"; echo "BUILD: $cmd"
      eval "$cmd" 2>"$OUT/build_${b}_l${i}_m$m.err"; rc=$?; echo "build exit $rc"
      [ $rc = 0 ] || { head -3 "$OUT/build_${b}_l${i}_m$m.err"; continue; }
      G2_ENTRIES=${ENT[$b]:-} G2_VERBOSE=1 timeout 900 "$exe" "$vf" >"$OUT/run_${b}_l${i}_m$m.out" 2>"$OUT/run_${b}_l${i}_m$m.err"; echo "run exit $?"
      cat "$OUT/run_${b}_l${i}_m$m.out"
    done
    # reversed order (state carried between cases in the current providers would show as a count change)
    echo "== $b line$i mutant=0 REVERSED order"
    G2_ENTRIES=${ENT[$b]:-} G2_VERBOSE=1 timeout 900 "$OUT/check_${b}_l${i}_m0" "$OUT/${b}_reversed.txt" >"$OUT/run_${b}_l${i}_rev.out" 2>"$OUT/run_${b}_l${i}_rev.err"; echo "run exit $?"; cat "$OUT/run_${b}_l${i}_rev.out"
    # informational: the same smoke with reads outside the original's tolerated (shows what else differs), then tables
    echo "== $b line$i mutant=0 LENIENT (extra RAM reads tolerated; informational)"
    G2_LENIENT=1 G2_ENTRIES=${ENT[$b]:-} G2_VERBOSE=1 timeout 900 "$OUT/check_${b}_l${i}_m0" "$vf" >"$OUT/run_${b}_l${i}_len.out" 2>"$OUT/run_${b}_l${i}_len.err"; echo "run exit $?"; cat "$OUT/run_${b}_l${i}_len.out"
    python3 $TOOLS/pe_gpu2_categorize.py "$OUT/run_${b}_l${i}_m0.err" > "$OUT/categories_${b}_l${i}_strict.txt"
    python3 $TOOLS/pe_gpu2_categorize.py "$OUT/run_${b}_l${i}_len.err" > "$OUT/categories_${b}_l${i}_lenient.txt"
    python3 $TOOLS/pe_gpu2_mutant_table.py "$vf" "$OUT/run_${b}_l${i}" > "$OUT/mutant_table_${b}_l${i}.md"
    echo "-- first-failure categories (strict contract) $b line$i"; cat "$OUT/categories_${b}_l${i}_strict.txt"
    echo "-- first-failure categories (lenient reads) $b line$i"; cat "$OUT/categories_${b}_l${i}_lenient.txt"
    echo "-- mutant control table $b line$i"; cat "$OUT/mutant_table_${b}_l${i}.md"
    # clean-room shape without any provider file and without the smoke layer must fail to link
    nopv=$(printf '%s' "$line" | sed -E 's# pc_port/(game|platform)/[^ ]+\.c##g; s# -DG2_SMOKE##; s# -o [^ ]+$# -o '"$OUT"'/check_noprov#')
    echo "== $b line$i NO PROVIDER"; echo "BUILD: $nopv"
    eval "$nopv" 2>"$OUT/build_${b}_l${i}_noprov.err"; echo "build exit $? (must be nonzero)"; grep -c "undefined reference" "$OUT/build_${b}_l${i}_noprov.err" | sed 's/^/undefined references: /'
  done
  if [ "$b" = gpu2d ] && [ -n "$EXPV" ]; then
    for m in 0 1 2 3; do
      echo "== gpu2d expand mutant=$m"
      G2_ENTRIES=8010BD4C G2_VERBOSE=1 timeout 1500 "$OUT/check_gpu2d_l1_m$m" "$EXPV" "$OVL" "$EXPB" >"$OUT/run_gpu2d_expand_m$m.out" 2>"$OUT/run_gpu2d_expand_m$m.err"; echo "run exit $?"; cat "$OUT/run_gpu2d_expand_m$m.out"
    done
  fi
done
