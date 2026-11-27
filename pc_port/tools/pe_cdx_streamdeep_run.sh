#!/usr/bin/env bash
# cdgen47_stream_deep: build the provider objects + checker variants (the `BUILD-<name>:` lines of pe_cdx_streamdeep_check.c, executed verbatim)
# and run native + the four controls on one vector set.  Usage (repo root of the worktree): SD_OUT=<scratch dir> SD_VEC=<vectors.txt> SD_AUTHOR_WT=<author worktree> pe_cdx_streamdeep_run.sh
# Runs under nice 19 / ionice idle by the caller.  Output: one summary line per (build, mode), failing line numbers in $SD_OUT/<name>.<mode>.err
set -uo pipefail
O=${SD_OUT:?}; V=${SD_VEC:?}; A=${SD_AUTHOR_WT:?}
mkdir -p "$O"
printf 'void pe_plat_cdhost_stream(void) {}\n' > "$O/stub_stream.c"
cc -c -std=gnu11 -O2 "$O/stub_stream.c" -o "$O/stub_stream.o"
cc -c -std=gnu11 -O2 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/platform/pe_plat_cd_stream_handler_host.c -o "$O/public_stream.o"
cc -c -std=gnu11 -O2 -I"$A/pc_port/include" -I"$A/pc_port/platform" -I"$A/pc_port/bootstrap" "$A/pc_port/platform/pe_plat2_cdhost_stream.c" -o "$O/author_stream.o"
SRC=pc_port/tools/pe_cdx_streamdeep_check.c
for n in ${SD_NAMES:-public public-mapped author stub}; do
  cmd=$(grep "^ \* BUILD-$n:" $SRC | sed "s/^ \* BUILD-$n: //" | sed "s#\$O#$O#g")
  echo "BUILD $n: $cmd"; bash -c "$cmd" || { echo "  build FAILED"; continue; }
  for m in "" noop flip retflip storeflip; do
    out=$(CDX_CATEGORIES=1 CDX_LINES=$([ -z "$m" ] && echo 1 || true) "$O/sd_$n" "$V" $m 2> "$O/sd_$n.${m:-native}.err"); r=$?
    echo "$n/${m:-native}: $(echo "$out" | head -1) exit=$r"
  done
done
