#!/usr/bin/env bash
# Regenerate every state60 vector file (4 batches, state60a in 4 shards) into OUTDIR and compare the sha256
# of each file (vector files and stats.json) with docs/evidence/native-state60-20261126/SHA256SUMS.txt.
# Run from the repository root.  The vector files are retail-derived and are NOT committed: keep them under
# <local vector dir>/state60-vectors/<batch>/ (pass that directory as OUTDIR).
# usage: pc_port/tools/pe_state60_regen.sh OUTDIR    (needs build/disc1.candidate.exe = the retail SLUS_006.62)
set -uo pipefail
OUT=${1:?outdir}
mkdir -p "$OUT"
T=pc_port/tools/pe_state60_vectors.py
declare -A SH=( [0]="8007D1D4 8007DCAC 80085F74 80085D84 80089EB8 8007DD14 80085DC4"
                [1]="8007D454 80085984 80085C44 80085A04 80089F28 80085F14 8007DAE0 8007DC5C"
                [2]="800862F4 80085A64 80085EB4 80089F08 8007DB24 80085F44 8007DC84"
                [3]="80085B34 80085BB4 8007DBC8 8007DC04" )
pids=(); names=()
for b in state60b state60c state60d; do python3 $T "$OUT/$b" $b > "$OUT/$b.log" 2>&1 & pids+=($!); names+=($b); done
for i in 0 1 2 3; do python3 $T "$OUT/state60a_$i" state60a ${SH[$i]} > "$OUT/a$i.log" 2>&1 & pids+=($!); names+=(state60a_$i); done
rc=0
for k in "${!pids[@]}"; do wait "${pids[$k]}"; e=$?; echo "generate ${names[$k]} exit $e"; [ $e -ne 0 ] && rc=1; done
mkdir -p "$OUT/state60a"
for i in 0 1 2 3; do mv "$OUT"/state60a_$i/func_*.txt "$OUT/state60a/"; done
python3 - "$OUT" <<'PY'
import glob, json, sys
out = sys.argv[1]
st = {}
for f in sorted(glob.glob(out + '/state60a_*/stats.json')):
    st.update(json.load(open(f)))
json.dump(st, open(out + '/state60a/stats.json', 'w'), indent=1, sort_keys=True)
PY
bad=0
while read -r h f; do
  g=$(sha256sum "$OUT/$f" | cut -d' ' -f1)
  [ "$g" = "$h" ] || { echo "DIFF $f"; bad=$((bad+1)); }
done < docs/evidence/native-state60-20261126/SHA256SUMS.txt
echo "compared $(wc -l < docs/evidence/native-state60-20261126/SHA256SUMS.txt) files, $bad differ"
[ $bad -eq 0 ] && [ $rc -eq 0 ]
