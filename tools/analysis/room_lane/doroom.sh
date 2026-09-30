#!/bin/bash
# doroom.sh <room> [src targets...]: config + all-asm split, port masked twins, link-check, carve, preflight, gate.
set -uo pipefail
cd "$(dirname "$0")/../../.."; export TMPDIR=${TMPDIR:-$PWD/build/tmp}

export LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu
r=$1; shift; t=room_$r; O=${ROOM_LANE_WORK:-build/room_lane}; L=$O/logs; mkdir -p $L $O/drafts
if [ ! -f configs/USA/overlays/$t.yaml ]; then
  info=$(python3 tools/extract/peimg.py room $r | tail -1)
  slot=$(echo "$info" | sed -n 's/.* slot \([0-9]*\) .*/\1/p')
  secs=$(echo "$info" | sed -n 's/.*PE.IMG sectors \[\(0x[0-9A-F]*\),\(0x[0-9A-F]*\)).*/\1 \2/p')
  size=$(echo "$info" | sed -n 's/.* size \(0x[0-9A-F]*\) .*/\1/p')
  sha=$(echo "$info" | sed -n 's/.* sha1 \([0-9a-f]*\)$/\1/p')
  win=$(awk -v r=$r '$1==r{print toupper($2), toupper($3)}' $O/groups.txt | sed 's/0X/0x/g')
  python3 tools/analysis/room_lane/newroom.py $r $slot $secs $size $sha $win || exit 1
fi
split() { for i in 1 2 3; do scripts/split_overlay.sh $t > $L/split_$r.log 2>&1 && break; sleep 15; done; }
split
for i in 1 2 3; do fw=$(python3 tools/analysis/room_lane/fixwindow.py $t); echo "fixwindow: $fw"; case "$fw" in *'->'*) split;; *) break;; esac; done
python3 tools/analysis/room_lane/port.py $r "$@" > $L/port_$r.txt 2>&1
ok=""; nbad=0
while read f off sz src envs; do [ "${f:0:5}" = func_ ] || continue
  [ "${envs:--}" = - ] && envs=""
  res=$(env $envs python3 tools/analysis/era_link_check.py --target $t src/overlays/$t/$f.c 0x${f#func_} $sz -O2 -G0 | tail -1)
  if [ "$res" = LINK_EXACT ]; then ok="$ok $f"; else echo "NOTEXACT $f $res"; mv src/overlays/$t/$f.c $O/drafts/${t}_$f.c; nbad=$((nbad+1)); fi
done < $L/port_$r.txt
[ -n "$ok" ] && python3 tools/analysis/room_lane/carve.py $t $ok > /dev/null || { echo "carve failed or nothing"; }
for i in 1 2 3; do git add -N src/overlays/$t/*.c 2>/dev/null && break; sleep 3; done
python3 tools/analysis/room_lane/fixwindow.py $t >/dev/null
python3 tools/build/disc1_preflight.py --deep --target $t 2>&1 | tail -1
for i in 1 2 3; do scripts/exact_rebuild_overlay.sh $t > $L/gate_$r.log 2>&1; g=$(grep EXACT_REBUILD_GATE $L/gate_$r.log); case "$g" in *=PASS*) break;; esac; grep -q 'does not ignore' $L/gate_$r.log || break; sleep 15; done
echo "$g"
echo "ported=$(echo $ok | wc -w) notexact=$nbad skips=$(grep -c SKIP $L/port_$r.txt)"
