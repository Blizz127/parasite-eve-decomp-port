#!/usr/bin/env bash
# Build and run the cdgen47 checker against the clean-room replacements platform/pe_plat2_cdhost_<fn>.c (-DCDX47_CDHOST=1).
# Usage (repo root of the worktree): CDX_VECTORS=<dir holding cdgen47/*.txt> pe_cdx_cdhost_check_run.sh [fn ...]   (fn = ack poll cmd sector stream dma; default all)
# Controls (pe_cdx_common.h numbering after the canary union): noop, flip, retflip, storeflip, poke0 (a store the original never made: fails with the canary seeding), same.
# Each function: build with the diagnostics recorded (and, for ack/poll/cmd/dma, the --silent vector set with -DCDX47_DIAG_SILENT=1), then run native and the
# four controls (noop flip retflip storeflip).  Output: one summary line per (build, mode).  Strict: no seam mapping variants exist for these builds.
set -uo pipefail
V=${CDX_VECTORS:?set CDX_VECTORS to the directory holding cdgen47/*.txt}
[ -d "$V/cdgen47" ] && V=$V/cdgen47
OUT=${CDX_OUT:-build/chk}
mkdir -p "$OUT"
CHK=pc_port/tools/pe_cdx_cdgen47_check.c
fns=${*:-ack poll cmd sector stream dma}
rc=0
for fn in $fns; do
  case $fn in ack) D=CDX47_ACK; var="";; poll) D=CDX47_POLL; var="";; cmd) D=CDX47_CMD; var="";; sector) D=CDX47_SECTOR; var="";; stream) D=CDX47_STREAM; var="";; dma) D=CDX47_DMA; var="";; *) echo "unknown $fn"; exit 2;; esac
  case $fn in ack|poll|cmd|dma) variants="plain silent";; *) variants="plain";; esac
  for v in $variants; do
    exe=$OUT/c47h_${fn}_$v; flags="-DCDX47_CDHOST=1 -D$D=1"; set_f=$V/$fn.txt
    if [ $v = silent ]; then flags="$flags -DCDX47_DIAG_SILENT=1"; set_f=$V/${fn}_silent.txt; fi
    build="cc -std=gnu11 -Wall -Wextra -Werror $flags -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap $CHK pc_port/platform/pe_plat2_cdhost_$fn.c -o $exe"
    echo "BUILD $fn/$v: $build"
    bash -c "$build" || { echo "  build FAILED"; rc=1; continue; }
    for m in "" noop flip retflip storeflip poke0 same; do
      out=$(CDX_CATEGORIES=1 CDX_LINES=$([ -z "$m" ] && echo 1 || true) "$exe" "$set_f" $m 2> "$OUT/c47h_${fn}_$v.${m:-native}.err"); r=$?
      echo "$fn/$v/${m:-native}: $(echo "$out" | head -1) exit=$r"
      [ -z "$m" ] && [ $r -ne 0 ] && rc=1
    done
  done
done
exit $rc
