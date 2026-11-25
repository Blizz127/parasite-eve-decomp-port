#!/usr/bin/env bash
# Builds and runs the standalone C89C adapter replay checker (compile line verbatim from pe_c89c_replay_check.c), then
# proves it can fail: each mutant of the adapter must be rejected.  Needs PE_DISC1_BIN.
set -euo pipefail
root="$(cd "$(dirname "$0")/../.." && pwd)"; cd "$root"
work="$(mktemp -d "${TMPDIR:-/var/tmp}/c89cck.XXXXXX")"; trap 'rm -rf "$work"' EXIT
flags=(-std=gnu99 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap -Ipc_port/tests)
build() { cc "${flags[@]}" pc_port/tools/pe_c89c_replay_check.c "$1" pc_port/platform/pe_vlc_decode.c pc_port/platform/pe_guest_ram.c -Wl,--unresolved-symbols=ignore-all -o "$2"; }
build pc_port/platform/pe_plat_vlc_frame.c "$work/check"
"$work/check"
mutants=(
  's/end_bits + 7u + 15u/end_bits + 8u + 15u/~~input cursor read-ahead 7 -> 8 bits'
  's/a1 + 4u + 4u \* (uint32_t)PE_LoadU16(g_frame_ptr)/a1 + 4u * (uint32_t)PE_LoadU16(g_frame_ptr)/~~end word without the +4 header'
  's/bound = PE_LoadU32(VLC_BOUND_WORD);/bound = 0xFFFFFFu;/~~bound word ignored'
  's/out_ptr = g_out_ptr;/out_ptr = a1;/~~resume writes at its a1 instead of the saved output pointer'
  's/g_active = (rc == 1);/g_active = 0;/~~no resume state kept'
  's/n_bytes = (uint32_t)(hw \* 2u) + (first ? 4u : 0u);/n_bytes = (uint32_t)(hw * 2u);/~~header halfwords not accounted in the output pointer'
)
status=0
for m in "${mutants[@]}"; do
  expr="${m%%~~*}"; desc="${m#*~~}"
  sed -e "$expr" pc_port/platform/pe_plat_vlc_frame.c > "$work/mut.c"
  if cmp -s "$work/mut.c" pc_port/platform/pe_plat_vlc_frame.c; then echo "MUTANT NOT APPLIED: $desc"; status=1; continue; fi
  cp pc_port/platform/pe_plat_vlc_frame.c "$work/orig.c"
  build "$work/mut.c" "$work/mutcheck" 2>/dev/null || { echo "mutant does not build: $desc"; status=1; continue; }
  if "$work/mutcheck" > "$work/mut.out" 2>&1; then echo "MUTANT SURVIVED: $desc"; status=1
  else echo "mutant rejected ($(head -1 "$work/mut.out" | head -c 100)): $desc"; fi
done
[ "$status" = 0 ] && echo "PASS: replay checker clean, all mutants rejected"
exit "$status"
