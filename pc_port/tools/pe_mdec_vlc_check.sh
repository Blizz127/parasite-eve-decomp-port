#!/usr/bin/env bash
# Builds and runs the standalone VLC vector checker (the compile line is the one in
# pe_mdec_vlc_vectors_check.c's header), then proves it can fail: the decoder's own
# compile-time mutants (-DPE_VLC_MUTANT=n: table entries, DC offset sign/magnitude, DC
# class tables, per-component DC accumulator, DC quant packing) must each be rejected.
# Needs PE_DISC1_BIN.   Usage: pe_mdec_vlc_check.sh
set -euo pipefail
root="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$root"
work="$(mktemp -d "${TMPDIR:-/var/tmp}/mdecvlc.XXXXXX")"
trap 'rm -rf "$work"' EXIT
cc -std=c99 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/tests pc_port/tools/pe_mdec_vlc_vectors_check.c pc_port/platform/pe_vlc_decode.c -o "$work/check"
"$work/check"
status=0
for n in 1 2 3 4 5 6 7 8 10; do
  cc -std=c99 -Wall -Wextra -Werror -DPE_VLC_MUTANT=$n -Ipc_port/include -Ipc_port/tests pc_port/tools/pe_mdec_vlc_vectors_check.c pc_port/platform/pe_vlc_decode.c -o "$work/mut$n"
  if "$work/mut$n" > "$work/mut.out" 2>&1; then echo "MUTANT $n SURVIVED"; status=1
  else echo "mutant $n rejected: $(head -1 "$work/mut.out" | head -c 120)"; fi
done
[ "$status" = 0 ] && echo "PASS: checker 0 mismatches, all mutants rejected"
exit "$status"
