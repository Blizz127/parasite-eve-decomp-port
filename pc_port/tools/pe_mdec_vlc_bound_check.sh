#!/usr/bin/env bash
# Builds and runs the standalone bound/resume VLC checker (the compile line is the one in
# pe_mdec_vlc_bound_check.c's header), then proves it can fail: the decoder's compile-time mutants
# 13 (bound checkpoint after the block's EOB instead of after its DC word), 14 (off by one: >= instead of >),
# 15 (DC predictors lost at a bounded exit), 16 (input cursor at a bounded exit without the 17-bit window)
# must each be rejected.   Needs PE_DISC1_BIN.   Usage: pe_mdec_vlc_bound_check.sh
set -euo pipefail
root="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$root"
work="$(mktemp -d "${TMPDIR:-/var/tmp}/mdecvlcb.XXXXXX")"
trap 'rm -rf "$work"' EXIT
cc -std=c99 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/tests pc_port/tools/pe_mdec_vlc_bound_check.c pc_port/platform/pe_vlc_decode.c -o "$work/check"
"$work/check"
status=0
for n in 13 14 15 16; do
  cc -std=c99 -Wall -Wextra -Werror -DPE_VLC_MUTANT=$n -Ipc_port/include -Ipc_port/tests pc_port/tools/pe_mdec_vlc_bound_check.c pc_port/platform/pe_vlc_decode.c -o "$work/mut$n"
  if "$work/mut$n" > "$work/mut.out" 2>&1; then echo "MUTANT $n SURVIVED"; status=1
  else echo "mutant $n rejected: $(head -1 "$work/mut.out" | head -c 160)"; fi
done
[ "$status" = 0 ] && echo "PASS: bound checker explains every step, all bound mutants rejected"
exit "$status"
