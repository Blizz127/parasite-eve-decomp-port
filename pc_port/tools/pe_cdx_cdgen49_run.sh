#!/usr/bin/env bash
# cdgen49 driver: build objects + checkers, regenerate the vector sets from the runtime traces, run native / noop / flip /
# retflip / storeflip for every (build, set).  Run from the repo root of a worktree with build/vec configured.
#   CDX_VECTORS = dir that holds cdgen49/ (retail-derived vectors, never committed); PE_PYLIB = dir holding pycparser/.
set -uo pipefail
V=${CDX_VECTORS:?set CDX_VECTORS}/cdgen49
LOG=${CDX_LOG:-$V/run49.log}
mkdir -p build/chk "$V/gen"
: > "$LOG"
run() { echo "\$ $*" | tee -a "$LOG"; "$@" >> "$LOG" 2>&1; local rc=$?; echo "  exit=$rc" | tee -a "$LOG"; return $rc; }
T=CMakeFiles/pe_field_runtime.dir/platform
run ninja -C build/vec $T/pe_libcd.c.o
sha256sum build/vec/$T/pe_libcd.c.o | tee -a "$LOG"
# the generator's own translation of 0x8006E6D4 (scratch, outside the tree)
rm -rf "$V/gen/out"; mkdir -p "$V/gen/out"
echo "\$ PYTHONPATH=\$PE_PYLIB python3 pc_port/tools/pe_cdx_cdgen49_gen6e6d4.py $V/gen/out" | tee -a "$LOG"
PYTHONPATH=${PE_PYLIB:-} python3 pc_port/tools/pe_cdx_cdgen49_gen6e6d4.py "$V/gen/out" >> "$LOG" 2>&1; echo "  exit=$?" | tee -a "$LOG"
sha256sum "$V/gen/out/func_8006E6D4_port.c" | tee -a "$LOG"
run cc -DPE_PORT_HEADLESS=1 -Ipc_port -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap -Ipc_port/src -O1 -std=gnu11 -funsigned-char -fno-strict-aliasing -fwrapv -c "$V/gen/out/func_8006E6D4_port.c" -o build/chk/gen6e6d4.o
nm -u build/chk/gen6e6d4.o | tee -a "$LOG"
SRC=pc_port/tools/pe_cdx_cdgen49_check.c
for n in hand gen; do
  cmd=$(grep "^ \* BUILD-$n:" $SRC | sed "s/^ \* BUILD-$n: *//; s#\$CDX49_GENOBJ#build/chk/gen6e6d4.o#")
  echo "\$ $cmd" | tee -a "$LOG"; bash -c "$cmd" >> "$LOG" 2>&1; echo "  exit=$?" | tee -a "$LOG"
done
TR=""; for f in "$V"/runtime/cold600.cap "$V"/runtime/cold9000.cap "$V"/runtime/route-route_fs.cap "$V"/runtime/route-route_ev.cap; do [ -f "$f" ] && TR="$TR --trace $f"; done
run python3 pc_port/tools/pe_cdx_cdgen49_vectors.py "$V/gen" $TR -n 480
for set in readissue timeout_poll timeout_issue; do
  for b in hand gen; do
    [ "$b" = gen ] && [ "$set" != readissue ] && continue
    for m in "" noop flip retflip storeflip poke0 same; do
      echo "\$ build/chk/check49_$b $V/gen/$set.txt $m" | tee -a "$LOG"
      CDX_CATEGORIES=1 CDX_LINES=$([ -z "$m" ] && echo 1 || true) build/chk/check49_$b "$V/gen/$set.txt" $m > "$LOG.out" 2> "$LOG.err"; rc=$?
      cat "$LOG.out" | tee -a "$LOG"; echo "  exit=$rc" | tee -a "$LOG"
      if [ -z "$m" ]; then grep '^FAILLINE' "$LOG.err" > "$V/gen/${set}_$b.faillines" || true; fi
    done
  done
done
(cd "$V/gen" && sha256sum *.txt) | tee -a "$LOG"
