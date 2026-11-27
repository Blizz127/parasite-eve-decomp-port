#!/usr/bin/env bash
# Regression for the pe_cdx_common.h / pe_cdx_vectors.py changes made by cdgen47 (additive: 8-argument callee records, retflip / storeflip modes,
# CDX_LINES / CDX_TRACE / CDX_DEBUG output, extra native-frame window; `silent` and `budget` spec keys in pe_cdx_vectors.py):
#   1. regenerate the cdgen42..46 vector sets with the CURRENT generator and compare their sha256 with the recorded sums (docs/evidence/native-cdx-20261126/cdgen46/VECTORS_SHA256SUMS + ../VECTORS_SHA256SUMS);
#   2. build each existing checker (BUILD lines executed verbatim) against the SAME objects twice, once with the header of the base commit
#      (git show <BASE>:pc_port/tools/pe_cdx_common.h) and once with the current header, run native / noop / flip, and diff the output.
# The tip objects no longer link against checkers 42/44/45/46 (bindings renamed since: pe_plat_cdxetc_*, pe_plat_spub_*; old and new header fail
# identically), so the objects come from OBJROOT = a build tree of the commit the checkers were written against (default build/vec):
#   git worktree add --detach build/wt-obj 0e65ca280; (sync pc_port/game/decomp{,_ovl}); cmake -S pc_port -B build/vec ...; ninja the five objects
# usage (repo root): CDX_VECTORS=<dir> BASE=<commit> [OBJROOT=<dir>/build/vec] pe_cdx_cdgen47_hdrcheck.sh
set -uo pipefail
OUT=${CDX_VECTORS:?}/hdrcheck; BASE=${BASE:?}; mkdir -p "$OUT/old" "$OUT/vec"
LOG=$OUT/hdrcheck.log; : > "$LOG"
OBJROOT=${OBJROOT:-build/vec}
run() { echo "\$ $*" | tee -a "$LOG"; "$@" >> "$LOG" 2>&1; local rc=$?; echo "  exit=$rc" | tee -a "$LOG"; return $rc; }
T=CMakeFiles/pe_field_runtime.dir
[ -n "${OBJROOT_PREBUILT:-}" ] || run ninja -C build/vec $T/game/boot/cd_stream_port.c.o $T/platform/pe_plat2_cdx_etc.c.o $T/platform/pe_plat_irq_callback.c.o $T/platform/pe_libsnd.c.o $T/platform/pe_stream.c.o $T/platform/pe_save.c.o
P=pc_port/tools
run python3 $P/pe_cdx_vectors.py $OUT/vec/c42.txt cdgen42 -n 200
run python3 $P/pe_cdx_irq_vectors.py $OUT/vec/c43_libetc.txt -fam libetc -n 100
run python3 $P/pe_cdx_irq_vectors.py $OUT/vec/c43_vsync.txt -fam vsync -n 100
run python3 $P/pe_cdx_irq_vectors.py $OUT/vec/c43_irq.txt -fam irq -n 100
run python3 $P/pe_cdx_vectors.py $OUT/vec/c44.txt cdgen44 -n 200
run python3 $P/pe_cdx_vectors.py $OUT/vec/c45.txt cdgen45 -n 150
run python3 $P/pe_cdx_vectors.py $OUT/vec/c46.txt cdgen46 -n 200
run python3 $P/pe_cdx_vectors.py $OUT/vec/c46e.txt cdgen46 -n 200 -edge
sha256sum $OUT/vec/*.txt | tee -a "$LOG"
git show "$BASE":pc_port/tools/pe_cdx_common.h > "$OUT/old/pe_cdx_common.h"
for f in 42 43 44 45 46; do cp $P/pe_cdx_cdgen${f}_check.c "$OUT/old/"; done
cp $P/pe_cdx_cdgen45_stub_icall.c "$OUT/old/" 2>/dev/null || true
b() { local src=$1 tag=$2 bin=$3 line; line=$(grep "^ \* BUILD$tag:" $P/$src | sed "s/^ \* BUILD$tag: //"); line=${line//build\/vec\//$OBJROOT/}; 
  bash -c "$line" >> "$LOG" 2>&1 || echo "NEW BUILD FAIL $bin" | tee -a "$LOG"
  local oldline=${line//pc_port\/tools\/$src/$OUT/old/$src}; oldline=${oldline// -o $bin/ -o $OUT/old/$bin}
  bash -c "$oldline" >> "$LOG" 2>&1 || echo "OLD BUILD FAIL $bin" | tee -a "$LOG"; }
b pe_cdx_cdgen42_check.c "" check42
b pe_cdx_cdgen43_check.c "-LIBETC" check43a
b pe_cdx_cdgen43_check.c "-VSYNC" check43c
b pe_cdx_cdgen43_check.c "-IRQ" check43b
b pe_cdx_cdgen44_check.c "" check44
b pe_cdx_cdgen45_check.c "" check45
b pe_cdx_cdgen46_check.c "" check46
cmpone() { local bin=$1 vec=$2 extra=${3:-}; for m in "" noop flip; do
  ./$bin $vec $extra $m > $OUT/new.out 2>/dev/null; rn=$?; $OUT/old/$bin $vec $extra $m > $OUT/old.out 2>/dev/null; ro=$?
  if cmp -s $OUT/new.out $OUT/old.out && [ $rn = $ro ]; then echo "SAME $bin ${m:-native}: $(head -1 $OUT/new.out) (exit $rn)" | tee -a "$LOG"; else echo "DIFF $bin ${m:-native}" | tee -a "$LOG"; diff $OUT/old.out $OUT/new.out | head -5 | tee -a "$LOG"; fi; done; }
cmpone check42 $OUT/vec/c42.txt
cmpone check43a $OUT/vec/c43_libetc.txt build/disc1.candidate.exe
cmpone check43c $OUT/vec/c43_vsync.txt build/disc1.candidate.exe
cmpone check43b $OUT/vec/c43_irq.txt build/disc1.candidate.exe
cmpone check44 $OUT/vec/c44.txt
cmpone check45 $OUT/vec/c45.txt
cmpone check46 $OUT/vec/c46.txt
cmpone check46 $OUT/vec/c46e.txt
rm -f check42 check43a check43b check43c check44 check45 check46
