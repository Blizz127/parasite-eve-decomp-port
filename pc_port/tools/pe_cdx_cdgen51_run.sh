#!/usr/bin/env bash
# cdgen51: build the provider object (read-only source from the int3 tree) and the checker variants (the `BUILD-<name>:` lines of pe_cdx_cdgen51_check.c, executed
# verbatim), run native + the six controls.  usage (repo root of the worktree): C51_OUT=<scratch dir> C51_VEC=<dir with clear.txt> C51_INT3=<int3 tree> pe_cdx_cdgen51_run.sh
set -uo pipefail
O=${C51_OUT:?}; V=${C51_VEC:?}; I=${C51_INT3:?}
mkdir -p "$O"
( cd "$I" && cc -c -std=gnu11 -O2 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/platform/pe_stream.c -o "$O/pe_stream.o" ) || exit 1
SRC=pc_port/tools/pe_cdx_cdgen51_check.c
for n in ${C51_NAMES:-strict mapped}; do
  cmd=$(grep "^ \* BUILD-$n:" $SRC | sed "s/^ \* BUILD-$n: //" | sed "s#\$O#$O#g")
  echo "BUILD $n: $cmd"; bash -c "$cmd" || { echo "  build FAILED"; continue; }
  for m in "" noop flip retflip storeflip poke0 same; do
    out=$(CDX_CATEGORIES=1 CDX_LINES=$([ -z "$m" ] && echo 1 || true) "$O/c51_$n" "$V/clear.txt" $m 2> "$O/c51_$n.${m:-native}.err"); r=$?
    echo "$n/${m:-native}: $(echo "$out" | head -1) exit=$r"
  done
done
