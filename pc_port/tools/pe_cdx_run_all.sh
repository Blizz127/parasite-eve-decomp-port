#!/usr/bin/env bash
# Regenerate every cdx vector set (cdgen42, cdgen42b, cdgen43..cdgen46), build the checkers and run native / noop / flip.
# Run from the repo root (a worktree with pc_port/game/decomp{,_ovl} synced and build/extracted +
# build/disc1.candidate.exe in place; build/disc1.candidate.exe must be the retail EXE, sha1 452fb033...).
# Vectors are retail-derived: written outside the repo (default $CDX_OUT), only their sha256 is committed.
set -uo pipefail
OUT=${CDX_VECTORS:?set CDX_VECTORS to the local (uncommitted, retail-derived) vector directory}
LOG=${CDX_LOG:-$OUT/run_all.log}
mkdir -p "$OUT/cdgen42" "$OUT/cdgen42b" "$OUT/cdgen43" "$OUT/cdgen44" "$OUT/cdgen45" "$OUT/cdgen46"
: > "$LOG"
run() { echo "\$ $*" | tee -a "$LOG"; "$@" >> "$LOG" 2>&1; local rc=$?; echo "  exit=$rc" | tee -a "$LOG"; return $rc; }
sha1sum build/disc1.candidate.exe | tee -a "$LOG"
T=CMakeFiles/pe_field_runtime.dir
run ninja -C build/vec $T/game/boot/cd_stream_port.c.o $T/platform/pe_plat2_cdx_etc.c.o $T/platform/pe_plat_irq_callback.c.o \
    $T/platform/pe_libsnd.c.o $T/platform/pe_stream.c.o $T/platform/pe_save.c.o
P=pc_port/tools
run python3 $P/pe_cdx_vectors.py $OUT/cdgen42/vectors.txt cdgen42 -n 200
# cdgen42b: runtime-observed tuples come from the scratch CDXT trace logs (CDX42B_TRACE=a.err,b.err; unset = product + random only)
run env CDX42B_TRACE="${CDX42B_TRACE:-}" python3 $P/pe_cdx_vectors.py $OUT/cdgen42b/vectors.txt cdgen42b -n 1
run python3 $P/pe_cdx_irq_vectors.py $OUT/cdgen43/libetc.txt -fam libetc -n 100
run python3 $P/pe_cdx_irq_vectors.py $OUT/cdgen43/vsync.txt -fam vsync -n 100
run python3 $P/pe_cdx_irq_vectors.py $OUT/cdgen43/irq.txt -fam irq -n 100
run python3 $P/pe_cdx_vectors.py $OUT/cdgen44/vectors.txt cdgen44 -n 200
run python3 $P/pe_cdx_vectors.py $OUT/cdgen45/vectors.txt cdgen45 -n 150
run python3 $P/pe_cdx_vectors.py $OUT/cdgen46/vectors.txt cdgen46 -n 200
run python3 $P/pe_cdx_vectors.py $OUT/cdgen46/edge.txt cdgen46 -n 200 -edge
run python3 $P/pe_cdx_vectors.py $OUT/cdgen44/regonly.txt cdgen44 -regonly -n 100   # register-level only, ~12 min
build() { local src=$1 tag=$2 out; out=$(grep "^ \* BUILD$3:" $src | sed "s/^ \* BUILD$3: //"); echo "\$ $out" | tee -a "$LOG"; $out >> "$LOG" 2>&1; echo "  exit=$?" | tee -a "$LOG"; }
build $P/pe_cdx_cdgen42_check.c 42 ""
build $P/pe_cdx_cdgen42b_check.c 42b ""
build $P/pe_cdx_cdgen43_check.c 43a "-ETC-LIBETC"
build $P/pe_cdx_cdgen43_check.c 43b "-IRQ"
build $P/pe_cdx_cdgen43_check.c 43c "-ETC-VSYNC"
build $P/pe_cdx_cdgen44_check.c 44 ""
build $P/pe_cdx_cdgen45_check.c 45 ""
build $P/pe_cdx_cdgen46_check.c 46 ""
chk() { for m in "" noop flip; do echo "\$ $* $m" | tee -a "$LOG"; "$@" $m 2>>"$LOG" | tee -a "$LOG"; echo "  exit=${PIPESTATUS[0]}" | tee -a "$LOG"; done; }
chk ./check42 $OUT/cdgen42/vectors.txt
chk ./check42b $OUT/cdgen42b/vectors.txt
chk ./check43a $OUT/cdgen43/libetc.txt build/disc1.candidate.exe
chk ./check43c $OUT/cdgen43/vsync.txt build/disc1.candidate.exe
chk ./check43b $OUT/cdgen43/irq.txt build/disc1.candidate.exe
chk ./check44 $OUT/cdgen44/vectors.txt
chk ./check45 $OUT/cdgen45/vectors.txt
chk ./check46 $OUT/cdgen46/vectors.txt
chk ./check46 $OUT/cdgen46/edge.txt
sha256sum $OUT/cdgen4[2-6]/*.txt $OUT/cdgen42b/*.txt | tee -a "$LOG" | tee $OUT/SHA256SUMS
