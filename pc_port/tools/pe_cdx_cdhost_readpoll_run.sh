#!/usr/bin/env bash
# cdgen50 against the clean-room read poll platform/pe_plat2_cdhost_readpoll.c (symbol pe_plat_cdhost_readpoll): builds the vector lane's checker pe_cdx_cdgen50_check.c in its
# retail-core configuration (-DC50_CORE=1: VSync 0x80073A44, ready entry 0x8007F608 and abort entry 0x80081268 are scripted callees) with the entry renamed to the new symbol, and
# runs native + the six controls on both sets.  poll_stop.txt (abort entry stopped) is the contract for a provider that calls the abort entry; poll.txt is the inlined-variant set
# (the oracle expands the abort body), run with -DC50_EXPAND_ABORT=1 so the abort-entry stand-in performs the inlined body, as the vector lane specified; without that define
# it fails on the time-out cases by design (reported as "poll/native_noexpand").
# usage (repo root): CDX50_VEC=<dir holding poll.txt poll_stop.txt> [CDX50_OUT=build/chk50] pe_cdx_cdhost_readpoll_run.sh
set -uo pipefail
V=${CDX50_VEC:?}; O=${CDX50_OUT:-build/chk50}; mkdir -p "$O"
cmd="cc -std=gnu11 -Wall -Wextra -Werror -DC50_CORE=1 -Dpe_plat_cd_read_poll=pe_plat_cdhost_readpoll -Ipc_port/tools -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen50_check.c pc_port/tools/pe_cdx_cdgen50_stop.c pc_port/platform/pe_plat2_cdhost_readpoll.c -o $O/c50_cdhost"
echo "BUILD: $cmd"; bash -c "$cmd" || exit 1
cmdx="${cmd/-DC50_CORE=1/-DC50_CORE=1 -DC50_EXPAND_ABORT=1} "; cmdx="${cmdx% }"; cmdx="${cmdx/-o $O\/c50_cdhost/-o $O/c50_cdhost_expand}"
echo "BUILD: $cmdx"; bash -c "$cmdx" || exit 1
for m in "" noop flip retflip storeflip poke0 same; do
  echo "poll_stop/${m:-native}: $("$O/c50_cdhost" "$V/poll_stop.txt" $m 2>/dev/null | head -1)"
done
for m in "" noop flip retflip storeflip poke0 same; do
  echo "poll(expand)/${m:-native}: $("$O/c50_cdhost_expand" "$V/poll.txt" $m 2>/dev/null | head -1)"
done
echo "poll/native_noexpand: $("$O/c50_cdhost" "$V/poll.txt" 2>/dev/null | head -1)"
