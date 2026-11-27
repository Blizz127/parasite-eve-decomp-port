#!/usr/bin/env bash
# Build the cdgen47 checkers (the `BUILD-<name>:` lines of pe_cdx_cdgen47_check.c, executed verbatim) and run native / noop / flip /
# retflip / storeflip on the matching vector set, plus the per-class tables and the fresh-process-per-case run for the stream handler.
# Run from the repo root of a worktree with build/vec configured (see the REPORT).  CDX_VECTORS = dir holding cdgen47/*.txt (retail-derived, uncommitted).
set -uo pipefail
# HISTORICAL: builds and checks the former providers platform/pe_plat_cd_*_host.c (ack, poll, cmd, sector, dma are removed in sw-cdhost-author-20261127).
# For the clean-room replacements use pc_port/tools/pe_cdx_cdhost_check_run.sh and pe_cdx_cdhost_replay_sets.sh.
V=${CDX_VECTORS:?set CDX_VECTORS to the directory that holds cdgen47/}/cdgen47
LOG=${CDX_LOG:-$V/run47.log}
mkdir -p build/chk
: > "$LOG"
run() { echo "\$ $*" | tee -a "$LOG"; "$@" >> "$LOG" 2>&1; local rc=$?; echo "  exit=$rc" | tee -a "$LOG"; return $rc; }
T=CMakeFiles/pe_field_runtime.dir/platform
run ninja -C build/vec $T/pe_plat_cd_ack_host.c.o $T/pe_plat_cd_status_poll_host.c.o $T/pe_plat_cd_command_issue_host.c.o \
    $T/pe_plat_cd_sector_dma_host.c.o $T/pe_plat_cd_stream_handler_host.c.o $T/pe_plat_cd_dma_issue_host.c.o
sha256sum build/vec/$T/pe_plat_cd_{ack,status_poll,command_issue,sector_dma,stream_handler,dma_issue}_host.c.o | tee -a "$LOG"
SRC=pc_port/tools/pe_cdx_cdgen47_check.c
names=$(grep -o '^ \* BUILD-[a-z0-9-]*:' $SRC | sed 's/^ \* BUILD-//; s/:$//')
for n in $names; do
  cmd=$(grep "^ \* BUILD-$n:" $SRC | sed "s/^ \* BUILD-$n: //")
  echo "\$ $cmd" | tee -a "$LOG"; bash -c "$cmd" >> "$LOG" 2>&1; echo "  exit=$?" | tee -a "$LOG"
done
for n in $names; do
  base=${n%%-*}
  case $n in *silent*) f=$V/${base}_silent.txt;; *) f=$V/$base.txt;; esac
  for m in "" noop flip retflip storeflip; do
    echo "\$ build/chk/check47_$n $f $m" | tee -a "$LOG"
    CDX_CATEGORIES=1 CDX_LINES=$([ -z "$m" ] && echo 1 || true) build/chk/check47_$n $f $m > "$LOG.out" 2> "$LOG.err"; rc=$?
    cat "$LOG.out" | tee -a "$LOG"; echo "  exit=$rc" | tee -a "$LOG"
    if [ -z "$m" ]; then grep '^FAILLINE' "$LOG.err" > "$V/$n.faillines" || true; python3 pc_port/tools/pe_cdx_cdgen47_classes.py $f "$V/$n.faillines" | tee -a "$LOG"; fi
  done
done
echo "== stream handler, one fresh process per case + forward/reversed shared order (variant stream-scratch-mdec)" | tee -a "$LOG"
python3 pc_port/tools/pe_cdx_cdgen47_percase.py build/chk/check47_stream-scratch-mdec $V/stream.txt | tee -a "$LOG"
echo "== stream handler, strict build, same" | tee -a "$LOG"
python3 pc_port/tools/pe_cdx_cdgen47_percase.py build/chk/check47_stream $V/stream.txt | tee -a "$LOG"
( cd "$V" && sha256sum ack.txt poll.txt cmd.txt sector.txt stream.txt dma.txt ack_silent.txt poll_silent.txt cmd_silent.txt dma_silent.txt *.classes ) | tee "$V/VECTORS_SHA256SUMS" | tee -a "$LOG"
