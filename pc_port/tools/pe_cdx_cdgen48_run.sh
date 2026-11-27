#!/usr/bin/env bash
# cdgen48 driver (SPU data upload 800850F4, wait 80085174, callback 80085098, prepare 800850C0).  Run from a worktree root
# set up as in docs/evidence/native-cdx-20261126/REPORT.md section 2 (build/extracted, build/disc1.candidate.exe, build/vec configured).
# Env: OUT (vector dir, default the cdx-vectors/cdgen48 scratch dir), TRACE (comma list of CDXT logs from pe_cdx_trace_wrap48.c; optional).
# Usage: pe_cdx_cdgen48_run.sh gen | check
set -uo pipefail
OUT=${OUT:?set OUT to the directory holding the cdgen48 vector sets}
P=pc_port/tools; O=build/vec/CMakeFiles/pe_field_runtime.dir
G=$P/pe_cdx_vectors.py
[ -n "${TRACE:-}" ] && export CDX48_TRACE=$TRACE
case "${1:-}" in
gen)
  mkdir -p "$OUT"
  python3 $G $OUT/upload.txt cdgen48 800850F4 > $OUT/gen_upload.log &
  python3 $G $OUT/upload_bound.txt cdgen48 800850F4 -variant bound > $OUT/gen_upload_bound.log &
  python3 $G $OUT/upload_fault.txt cdgen48 800850F4 -variant fault -n 200 > $OUT/gen_upload_fault.log &
  python3 $G $OUT/wait.txt cdgen48 80085174 > $OUT/gen_wait.log &
  python3 $G $OUT/callback.txt cdgen48 80085098 > $OUT/gen_callback.log &
  python3 $G $OUT/callback_bound.txt cdgen48 80085098 -variant bound > $OUT/gen_callback_bound.log &
  python3 $G $OUT/prepare.txt cdgen48 800850C0 > $OUT/gen_prepare.log &
  python3 $G $OUT/prepare_bound.txt cdgen48 800850C0 -variant bound > $OUT/gen_prepare_bound.log &
  wait
  (cd $OUT && sha256sum upload.txt upload_bound.txt upload_fault.txt wait.txt callback.txt callback_bound.txt prepare.txt prepare_bound.txt > VECTORS_SHA256SUMS)
  cat $OUT/gen_*.log | grep cases= ;;
check)
  # Objects (clean-room provider, cmake tree build/dev): ninja -C build/dev CMakeFiles/pe_field_runtime.dir/platform/pe_plat2_spu_upload.c.o
  # plus, for the register-level sets, pe_plat2_cdx_spu.c.o / pe_plat2_spu_a.c.o / pe_plat2_spu_b.c.o (the clean-room 80085E54, the SPU_DELAY
  # leaves and 80085F44).  Bound sets link the provider ALONE (the weak stand-ins record the stopped callees); register-level sets add the
  # three objects and -DCDX48_REGSTUBS (trap stand-ins for symbols they leave undefined).  Set O to another object directory if needed.
  OBJ=${OBJ:-build/dev/CMakeFiles/pe_field_runtime.dir/platform}
  CF="-std=gnu11 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap"
  cc $CF -Dfunc_80085F44=cdx48_stub_80085F44 -Dfunc_80085E54=cdx48_stub_80085E54 $P/pe_cdx_cdgen48_check.c $OBJ/pe_plat2_spu_upload.c.o -o build/check48 || exit 1
  cc $CF -DCDX48_REGSTUBS -Dfunc_80085F44=cdx48_stub_80085F44 -Dfunc_80085E54=cdx48_stub_80085E54 $P/pe_cdx_cdgen48_check.c $OBJ/pe_plat2_spu_upload.c.o $OBJ/pe_plat2_cdx_spu.c.o $OBJ/pe_plat2_spu_a.c.o $OBJ/pe_plat2_spu_b.c.o -o build/check48reg || exit 1
  for f in upload upload_bound upload_fault wait callback callback_bound prepare prepare_bound; do
    case $f in *_bound|wait) B=build/check48 ;; *) B=build/check48reg ;; esac
    for m in "" noop flip; do
      echo "\$ CDX_CATEGORIES=1 $B $f.txt $m"
      CDX_CATEGORIES=1 CDX_MAXPRINT=0 timeout 900 $B $OUT/$f.txt $m 2>/dev/null | grep -E "^mode"; echo "  exit=${PIPESTATUS[0]}"
    done
  done ;;
*) echo "usage: $0 gen|check" >&2; exit 2 ;;
esac
