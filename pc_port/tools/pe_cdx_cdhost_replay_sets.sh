#!/usr/bin/env bash
# Replay every vector family against the clean-room CD host replacements (strict CDX47_CDHOST builds):
#   own/cdgen47 (boot sets, + controls)  own/cdgen47_s2  own3/cdgen47 (dma regenerated under the drop-the-case rule)
#   own/cdgen47_credits (ack/poll/cmd/sector, poll also with -DCDX47_EXPAND_80080778=1, see the checker)  own/cdgen47_credits_wait (sector).
# Usage (repo root): CDX_ROOT=<dir holding own/ own3/> pe_cdx_cdhost_replay_sets.sh
# Needs build/chk/c47h_<fn>_{plain,silent} from pe_cdx_cdhost_check_run.sh.  Prints one line per (family, function, build).
set -uo pipefail
R=${CDX_ROOT:?set CDX_ROOT to the cdx-vectors directory}
CHK=pc_port/tools/pe_cdx_cdgen47_check.c
INC="-Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap"
# CDX_CONTROLS=1 also runs the six controls (noop flip retflip storeflip poke0 same) and prints their mismatch counts after the native line.
run() { # label exe set
  [ -f "$3" ] || { echo "$1: (no set $3)"; return; }
  local line ctl=""
  line=$("$2" "$3" 2>/dev/null | head -1)
  if [ -n "${CDX_CONTROLS:-}" ]; then
    for m in noop flip retflip storeflip poke0 same; do ctl="$ctl $m=$("$2" "$3" $m 2>/dev/null | head -1 | sed 's/.*checked [0-9]* cases, \([0-9]*\) mismatches.*/\1/')"; done
  fi
  echo "$1: $line [set sha256 $(sha256sum "$3" | cut -c1-12)]${ctl:+ controls:$ctl}"
}
c=$R/own/cdgen47_credits; w=$R/own/cdgen47_credits_wait; s2=$R/own/cdgen47_s2; o3=$R/own3/cdgen47
for f in ack poll cmd sector; do run "credits $f/plain" build/chk/c47h_${f}_plain $c/$f.txt; done
for f in poll; do run "credits $f/silent" build/chk/c47h_${f}_silent $c/${f}_silent.txt; done
cc -std=gnu11 -Wall -Wextra -Werror -DCDX47_CDHOST=1 -DCDX47_POLL=1 -DCDX47_EXPAND_80080778=1 $INC $CHK pc_port/platform/pe_plat2_cdhost_poll.c -o build/chk/c47h_poll_expand_plain || exit 1
cc -std=gnu11 -Wall -Wextra -Werror -DCDX47_CDHOST=1 -DCDX47_POLL=1 -DCDX47_EXPAND_80080778=1 -DCDX47_DIAG_SILENT=1 $INC $CHK pc_port/platform/pe_plat2_cdhost_poll.c -o build/chk/c47h_poll_expand_silent || exit 1
run "credits poll+expand80080778/plain" build/chk/c47h_poll_expand_plain $c/poll.txt
run "credits poll+expand80080778/silent" build/chk/c47h_poll_expand_silent $c/poll_silent.txt
run "credits_wait sector/plain" build/chk/c47h_sector_plain $w/sector_wait.txt
for f in ack poll cmd sector stream dma; do
  run "s2 $f/plain" build/chk/c47h_${f}_plain $s2/${f}_s2.txt
  [ -x build/chk/c47h_${f}_silent ] && run "s2 $f/silent" build/chk/c47h_${f}_silent $s2/${f}_silent_s2.txt
done
run "own3 dma/plain" build/chk/c47h_dma_plain $o3/dma_s2.txt
run "own3 dma/silent" build/chk/c47h_dma_silent $o3/dma_silent.txt
# Deep stream set (cdgen47_stream_deep): CDX_DEEP_CHECKER = pe_cdx_streamdeep_check.c, CDX_DEEP_SET = stream_deep.txt (both from the vector lane); built against this tree's pe_cdx_common.h.
if [ -n "${CDX_DEEP_CHECKER:-}" ] && [ -n "${CDX_DEEP_SET:-}" ]; then
  cc -std=gnu11 -Wall -Wextra -Werror -DSD_AUTHOR=1 -Ipc_port/tools -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap "$CDX_DEEP_CHECKER" pc_port/platform/pe_plat2_cdhost_stream.c -o build/chk/sd_author_mapped || exit 1
  run "deep stream (window mapped)" build/chk/sd_author_mapped "$CDX_DEEP_SET"
  cc -std=gnu11 -Wall -Wextra -Werror -DSD_AUTHOR=1 -DCDX_STRICT_FRAME_PTR -Ipc_port/tools -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap "$CDX_DEEP_CHECKER" pc_port/platform/pe_plat2_cdhost_stream.c -o build/chk/sd_author_strict || exit 1
  run "deep stream (strict frame pointers)" build/chk/sd_author_strict "$CDX_DEEP_SET"
fi
