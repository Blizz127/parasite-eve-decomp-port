#!/usr/bin/env bash
# cdgen52..61: build the provider objects (read-only sources from the int3 tree) and the checker variants (the `BUILD-<name>:` lines of pe_cdx_cdgen<NN>_check.c, executed
# verbatim), run native + the six controls on every vector file of the set.  usage (repo root of the worktree):
#   C5_SET=cdgen52 C5_OUT=<scratch dir> C5_VEC=<dir with the set's .txt files> C5_INT3=<int3 tree> C5_OBJS="pe_plat_cd_sync" pe_cdx_cdgen52_61_run.sh
# C5_OBJS: space-separated provider source basenames (platform/<name>.c, or <dir>:<name> for pc_port/<dir>/<name>.c, e.g. game/boot:func_80071A84_port) compiled into $O/<name>.o; C5_FILES (default: every *.txt in C5_VEC) the vector files.
set -uo pipefail
S=${C5_SET:?}; O=${C5_OUT:?}; V=${C5_VEC:?}; I=${C5_INT3:?}; OBJS=${C5_OBJS:?}
mkdir -p "$O"
for f in $OBJS; do d=platform; case $f in *:*) d=${f%%:*}; f=${f##*:} ;; esac; ( cd "$I" && cc -c -std=gnu11 -O2 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/$d/$f.c -o "$O/$f.o" ) || exit 1; done   # C5_OBJS entries are platform/<name>.c or <dir>:<name> (pc_port/<dir>/<name>.c)
SRC=pc_port/tools/pe_cdx_${S}_check.c
FILES=${C5_FILES:-$(cd "$V" && ls *.txt)}
for n in $(grep -o '^ \* BUILD-[A-Za-z0-9_-]*:' $SRC | sed 's/^ \* BUILD-//; s/:$//'); do
  cmd=$(grep "^ \* BUILD-$n:" $SRC | sed "s/^ \* BUILD-$n: //" | sed "s#\$O#$O#g")
  echo "BUILD $n: $cmd"; bash -c "$cmd" || { echo "  build FAILED"; continue; }
  for f in $FILES; do
    for m in "" noop flip retflip storeflip poke0 same; do
      out=$(CDX_CATEGORIES=1 CDX_LINES=$([ -z "$m" ] && echo 1 || true) "$O/c${S#cdgen}_${n//-/_}" "$V/$f" $m 2> "$O/c${S#cdgen}_${n//-/_}.${f%.txt}.${m:-native}.err"); r=$?
      echo "$n/${f%.txt}/${m:-native}: $(echo "$out" | head -1) exit=$r"
    done
  done
done
