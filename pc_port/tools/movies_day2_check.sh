#!/usr/bin/env bash
# Movies-on route into Day 2 (r4 gate). Usage: movies_day2_check.sh <bin> <outdir>
set -euo pipefail
BIN=$1; OUT=$2; mkdir -p "$OUT"
export TMPDIR=$PWD/build/tmp
PE_ROUTE_PLAY_MOVIES=1 PE_FMV_LOG=1 "$BIN" --headless --route-pad --max-frames 90000 \
  --disc-image "rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin" \
  --screenshot "$OUT/end.ppm" > "$OUT/run.log" 2>&1 || true
L=$OUT/run.log; ok=1
n=$(grep -c 'stream close.*dropped=0' "$L" || true); echo "fmv_closed_dropped0=$n"; [ "$n" -ge 5 ] || ok=0
for pat in 'token=A8000048 story=00000080' 'token=A80650C8' 'token=A8004148 story=00000088' 'story=00000090'; do
  if grep -q "change.*$pat" "$L"; then echo "reached: $pat"; else echo "MISSING: $pat"; ok=0; fi
done
if grep -qE 'STUB:BOOTSTRAP_RET\] func_80077404|stop_reason=unresolved' "$L"; then echo "BOUNDARY STOP found"; ok=0; fi
grep -a 'unresolved' "$L" | grep -v 'PE_M0023I_Flash' | head -3 || true
grep -a 'stop_reason' "$L" | tail -1 || true
[ $ok = 1 ] && echo MOVIES_DAY2=PASS || { echo MOVIES_DAY2=FAIL; exit 1; }
