#!/usr/bin/env bash
# cdgen49b driver (DsRead 0x80080E34): regenerate dsread.txt from the cdgen49 runtime traces and run the checker on a provider object.
#   CDX_VECTORS = dir holding cdgen49/ and cdgen49b/ (retail-derived vectors, never committed)
#   CDX49B_OBJ  = provider object to check, CDX49B_FN = its symbol (default func_80080E34).  With CDX49B_OBJ unset only the vectors are generated.
set -uo pipefail
V=${CDX_VECTORS:?set CDX_VECTORS}/cdgen49b
R=${CDX_VECTORS}/cdgen49/runtime
LOG=${CDX_LOG:-$V/run49b.log}
mkdir -p build/chk "$V"
: > "$LOG"
run() { echo "\$ $*" | tee -a "$LOG"; "$@" >> "$LOG" 2>&1; local rc=$?; echo "  exit=$rc" | tee -a "$LOG"; return $rc; }
TR=""; for f in "$R"/cold600.cap "$R"/cold9000.cap "$R"/route-route_fs.cap "$R"/route-route_ev.cap; do [ -f "$f" ] && TR="$TR --trace $f"; done
run python3 pc_port/tools/pe_cdx_cdgen49b_vectors.py "$V" $TR -n 480
[ -n "${CDX49B_OBJ:-}" ] || { (cd "$V" && sha256sum dsread.txt) | tee -a "$LOG"; exit 0; }
FN=${CDX49B_FN:-func_80080E34}
SRC=pc_port/tools/pe_cdx_cdgen49b_check.c
cmd=$(grep "^ \* BUILD:" $SRC | sed 's/^ \* BUILD: *//; s#\$CDX49B_FN#'"$FN"'#; s#\$CDX49B_OBJ#'"$CDX49B_OBJ"'#')
echo "\$ $cmd" | tee -a "$LOG"; bash -c "$cmd" >> "$LOG" 2>&1; echo "  exit=$?" | tee -a "$LOG"
for m in "" noop flip retflip storeflip poke0 same; do
  echo "\$ build/chk/check49b $V/dsread.txt $m" | tee -a "$LOG"
  CDX_CATEGORIES=1 build/chk/check49b "$V/dsread.txt" $m > "$LOG.out" 2> "$LOG.err"; rc=$?
  cat "$LOG.out" | tee -a "$LOG"; echo "  exit=$rc" | tee -a "$LOG"
done
(cd "$V" && sha256sum dsread.txt) | tee -a "$LOG"
