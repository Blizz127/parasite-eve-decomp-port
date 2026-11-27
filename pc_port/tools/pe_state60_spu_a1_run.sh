#!/bin/bash
# Run the state60a checker (pe_state60_spu_check.c, unmodified) against the clean-room
# provider pe_plat2_spu_a.c on rows 1-13 of state60a, plus the mutant controls.
# usage (repo root): pc_port/tools/pe_state60_spu_a1_run.sh VECTOR_DIR
# The build line is taken verbatim from the checker header; the only changes are: the
# provider object is replaced by the clean-room object, plus `-include <map header>` and
# the stub object (pe_plat_sio_poll_tick) on the link line.  m3 = no-op object generated
# from pc_port/include/pe_plat/spu_a.h.
set -uo pipefail
VEC=${1:?vector dir}
FUNCS="8007D1D4 8007D454 8007DAE0 8007DB24 8007DBC8 8007DC04 8007DC5C 8007DC84 8007DCAC 8007DD14 80085984 80085A04 80085A64"
SRC=pc_port/tools/pe_state60_spu_check.c
OUT=build/state60a1; mkdir -p $OUT
LINE=$(grep -E '^ \*   cc ' $SRC | head -1 | sed 's/^ \*   //')
CF="-std=c99 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -O1 -fno-stack-protector -Ipc_port -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap"
cc $CF -c pc_port/platform/pe_plat2_spu_a.c -o $OUT/prov.o || exit 2
cc -c -w pc_port/tools/pe_state60_spu_a1_stubs.c -o $OUT/stubs.o || exit 2
python3 - <<'P' > $OUT/noop.c
import re
t=open('pc_port/include/pe_plat/spu_a.h').read()
t=re.sub(r'/\*.*?\*/','',t,flags=re.S)
print('#include <stdint.h>')
for m in re.finditer(r'^(int|void) (pe_plat_spua_\w+)\(([^)]*)\);',t,flags=re.M):
    print('%s %s(%s) { %s}'%(m.group(1),m.group(2),m.group(3),'return 0; ' if m.group(1)=='int' else ''))
P
cc -c -w $OUT/noop.c -o $OUT/noop.o || exit 2
build() { # name extra-defs object
  local l=$LINE
  l=${l/build\/state60\/prov\/pe_plat_state.c.o/$3 $OUT/stubs.o}
  l=${l/-Wall/$2 -include pc_port/tools/pe_state60_spu_a1_map.h -Wall}
  l=$(echo "$l" | sed "s# -o build/state60/check_spu# -o $OUT/check_$1#")
  echo "$l" > $OUT/buildline_$1.txt
  eval "$l" || exit 3
}
build base "" $OUT/prov.o
build m1 "-DSTATE60_MUTANT=1" $OUT/prov.o
build m2 "-DSTATE60_MUTANT=2" $OUT/prov.o
build m3 "" $OUT/noop.o
printf '%-14s %-18s %-12s %-12s %-12s\n' func base m1-fail m2-fail m3-fail
tp=0; tf=0
for f in $FUNCS; do
  row="func_$f"; 
  for k in base m1 m2 m3; do
    res=$($OUT/check_$k $VEC/func_$f.txt 2>/dev/null | grep -v '^total' | head -1)
    rc=$?
    n=$(echo "$res" | sed -n 's/.*checked \([0-9]*\) cases, PASS \([0-9]*\), FAIL \([0-9]*\).*/\1 \2 \3/p')
    [ -z "$n" ] && n="CRASH"
    set -- $n
    if [ "$k" = base ]; then row="$row $2/$1"; tp=$((tp+${2:-0})); tf=$((tf+${3:-0})); else row="$row ${3:-CRASH}"; fi
  done
  printf '%s\n' "$row"
done
echo "base total PASS=$tp FAIL=$tf"
