#!/usr/bin/env bash
# cdgen50: build the provider object (read-only source from the int3 tree) and the checker variants (the `BUILD-<name>:` lines of pe_cdx_cdgen50_check.c, executed verbatim),
# run native + the six controls on both sets.  usage (repo root of the worktree): C50_OUT=<scratch dir> C50_VEC=<dir with poll.txt poll_stop.txt> C50_INT3=<int3 tree> pe_cdx_cdgen50_run.sh
set -uo pipefail
O=${C50_OUT:?}; V=${C50_VEC:?}; I=${C50_INT3:?}
mkdir -p "$O"
( cd "$I" && cc -c -std=gnu11 -O2 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/platform/pe_plat_cd_poll.c -o "$O/legacy_poll.o" ) || exit 1
SRC=pc_port/tools/pe_cdx_cdgen50_check.c
for n in ${C50_NAMES:-host host-mapped core}; do
  cmd=$(grep "^ \* BUILD-$n:" $SRC | sed "s/^ \* BUILD-$n: //" | sed "s#\$O#$O#g")
  echo "BUILD $n: $cmd"; bash -c "$cmd" || { echo "  build FAILED"; continue; }
  for set in poll poll_stop; do
    for m in "" noop flip retflip storeflip poke0 same; do
      out=$(CDX_CATEGORIES=1 CDX_LINES=$([ -z "$m" ] && echo 1 || true) "$O/c50_${n//-/_}" "$V/$set.txt" $m 2> "$O/c50_${n//-/_}.$set.${m:-native}.err"); r=$?
      echo "$n/$set/${m:-native}: $(echo "$out" | head -1) exit=$r"
    done
  done
done
