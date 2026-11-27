#!/usr/bin/env bash
# Regenerate the cdgen47 vector sets.  usage: pe_cdx_cdgen47_gen.sh OUTDIR CAPTURE.cap [CAPTURE.cap ..]   (repo root of a worktree set up as in the REPORT)
# One file per provider (ack poll cmd sector stream dma) plus `<name>_silent.txt` (printf / puts silent stops) for the four entries that call them.
# Vectors are retail-derived: OUTDIR is outside the repo.  JOBS (default 4) processes in parallel.
set -uo pipefail
OUT=$1; shift
mkdir -p "$OUT"
P=pc_port/tools/pe_cdx_cdhost_vectors.py
jobs=()
for pair in ack:8007AAB4 poll:8007B010 cmd:8007B558 sector:8007BF44 stream:8007C564 dma:8007CEAC; do
  n=${pair%%:*}; e=${pair##*:}
  jobs+=("python3 $P $OUT/$n.txt -n 200 --capture $* --only $e > $OUT/$n.gen.log 2>&1")
  case $n in ack|poll|cmd|dma) jobs+=("python3 $P $OUT/${n}_silent.txt -n 200 --capture $* --only $e --silent > $OUT/${n}_silent.gen.log 2>&1");; esac
done
printf '%s\n' "${jobs[@]}" | xargs -P "${JOBS:-4}" -I{} bash -c '{}'
for f in "$OUT"/*.gen.log; do cat "$f"; done
