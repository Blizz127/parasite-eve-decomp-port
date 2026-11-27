#!/usr/bin/env bash
# Run the own-written-read audit (pe_vector_selfread_audit.py) over every black-box vector generator in pc_port/tools.
# Test tooling only; vectors and JSON stay under $OUT (scratch, retail-derived: never commit them).
# Run from the repo root (a worktree with build/disc1.candidate.exe = the retail EXE (sha1 452fb033...), build/extracted and
# PE_DISC1_BIN set for the two mdec generators).  Usage: OUT=/scratch/dir JOBS=4 pc_port/tools/pe_vector_selfread_run_all.sh
# CDX42B_TRACE (comma-separated trace logs, exported) reproduces the shipped cdgen42b set byte for byte; without it only the
# product + random cases are produced.  gte_trig runs one process per entry (the all-entry run took > 25 minutes, per entry ~2 minutes).
# Two environment limits, both recorded in the report:
#   * pe_state60_vectors.py asserts on an interpreter without DIVU, i.e. it only imports against the pe_battle_hud_oracle.py of
#     the commit before b4ad5761d; STATE60_TREE must be a tree (pc_port/tools copy, with that older oracle) it can import in.
#   * cdgen47 (pe_cdx_cdhost_vectors.py + its pe_cdx_specs.py batch) was uncommitted in the sw-vec-cdhost worktree when this
#     audit ran; CDHOST_TREE must be a tree holding those files (same layout, with build/ and configs symlinks).
set -uo pipefail
OUT=${OUT:?set OUT}
JOBS=${JOBS:-4}
T=pc_port/tools/pe_vector_selfread_audit.py
mkdir -p "$OUT"
cat > "$OUT/jobs.txt" <<'EOF'
cdgen42|dyn|pe_cdx_vectors|cdgen42 -n 200
cdgen44|dyn|pe_cdx_vectors|cdgen44 -n 200
cdgen45|dyn|pe_cdx_vectors|cdgen45 -n 150
cdgen46|dyn|pe_cdx_vectors|cdgen46 -n 200
cdgen46edge|dyn|pe_cdx_vectors|cdgen46 -n 200 -edge
cdgen44_regonly|dyn|pe_cdx_vectors|cdgen44 -regonly -n 100
cdgen42b|dyn|pe_cdx_vectors|cdgen42b -n 1
cdgen9|dyn|pe_cd_leaf_vectors|cdgen9
cdgen10|dyn|pe_cd_leaf_vectors|cdgen10
cdgen11|dyn|pe_cd_leaf_vectors|cdgen11
cdgen12|dyn|pe_cd_leaf_vectors|cdgen12
cdgen13|dyn|pe_cd_leaf_vectors|cdgen13
cdgen14|dyn|pe_cd_leaf_vectors|cdgen14
cdgen15|dyn|pe_cd_leaf_vectors|cdgen15
cdgen16|dyn|pe_cd_leaf_vectors|cdgen16
cdgen17|dyn|pe_cd_leaf_vectors|cdgen17
cdgen18|dyn|pe_cd_leaf_vectors|cdgen18
cdgen19|dyn|pe_cd_leaf_vectors|cdgen19
cdgen20|dyn|pe_cd_leaf_vectors|cdgen20
cdgen21|dyn|pe_cd_leaf_vectors|cdgen21
cdgen22|dyn|pe_cd_leaf_vectors|cdgen22
cdgen23|dyn|pe_cd_leaf_vectors|cdgen23
cdgen24|dyn|pe_cd_leaf_vectors|cdgen24
cdgen25|dyn|pe_cd_leaf_vectors|cdgen25
cdgen26|dyn|pe_cd_leaf_vectors|cdgen26
cdgen28|dyn|pe_cd_leaf_vectors|cdgen28
cdgen29|dyn|pe_cd_leaf_vectors|cdgen29
cdgen30|dyn|pe_cd_leaf_vectors|cdgen30
cdgen31|dyn|pe_cd_leaf_vectors|cdgen31
cdgen32|dyn|pe_cd_leaf_vectors|cdgen32
cdgen33|dyn|pe_cd_leaf_vectors|cdgen33
cdgen34|dyn|pe_cd_leaf_vectors|cdgen34
cdgen35|dyn|pe_cd_leaf_vectors|cdgen35
cdgen36a|dyn|pe_cd_leaf_vectors|cdgen36 6000 8007B290
cdgen36b|dyn|pe_cd_leaf_vectors|cdgen36 6000 80081110
cdgen38|dyn|pe_cd_leaf_vectors|cdgen38
cdgen39|dyn|pe_cd_leaf_vectors|cdgen39
cdgen7|dyn|pe_cd_leaf_vectors|cdgen7
cdgen8|dyn|pe_cd_leaf_vectors|cdgen8
cdgen|dyn|pe_cd_leaf_vectors|cdgen
cdgen2|dyn|pe_cd_leaf_vectors|cdgen2
cdgen3|dyn|pe_cd_leaf_vectors|cdgen3
cdgen5|dyn|pe_cd_leaf_vectors|cdgen5
cdgen6|dyn|pe_cd_leaf_vectors|cdgen6
flush|dyn|pe_cd_leaf_vectors|flush
callback|dyn|pe_cd_leaf_vectors|callback
modetype|dyn|pe_cd_leaf_vectors|modetype
reverb|dyn|pe_cd_leaf_vectors|reverb
errmath|dyn|pe_cd_leaf_vectors|errmath
spucallees|dyn|pe_cd_leaf_vectors|spucallees
spureverb|dyn|pe_cd_leaf_vectors|spureverb
fidma|dyn|pe_cd_leaf_vectors|fidma
cardclear|dyn|pe_cd_leaf_vectors|cardclear
qsort|dyn|pe_cd_leaf_vectors|qsort
cdleaf_default|dyn|pe_cd_leaf_vectors|
cdleaf_libcard|dyn|pe_cd_leaf_vectors|80082ADC 80082CDC 80084FC4 80084FE4 80083578
padleaf_default|dyn|pe_pad_leaf_vectors|
padleaf_b2|dyn|pe_pad_leaf_vectors|batch2
padleaf_b3|dyn|pe_pad_leaf_vectors|batch3
gpuleaf_default|dyn|pe_gpu_leaf_vectors|
gpuleaf_drawenv|dyn|pe_gpu_leaf_vectors|drawenv
gteleaf_default|dyn|pe_gte_leaf_vectors|
gteleaf_stack|dyn|pe_gte_leaf_vectors|stack
gtetrig_80077CF4|dyn|pe_gte_trig_vectors|80077CF4
gtetrig_80077D30|dyn|pe_gte_trig_vectors|80077D30
gtetrig_80077DC4|dyn|pe_gte_trig_vectors|80077DC4
gtetrig_800794C4|dyn|pe_gte_trig_vectors|800794C4
gtetrig_80079754|dyn|pe_gte_trig_vectors|80079754
gtetrig_800799E4|dyn|pe_gte_trig_vectors|800799E4
gtetrig_80079E14|dyn|pe_gte_trig_vectors|80079E14
dsleaf|dyn|pe_ds_leaf_vectors|
tim|dyn|pe_gpu_tim_vectors|
mdeccore|dyn|pe_mdec_core_vectors|
mdecreset|dyn|pe_mdec_reset_vectors|
gpu2a|dynR|pe_gpu2_vectors|gpu2a
gpu2b|dynR|pe_gpu2_vectors|gpu2b
gpu2c|dynR|pe_gpu2_vectors|gpu2c
gpu2d|dynR|pe_gpu2_vectors|gpu2d
gpu2e|dynR|pe_gpu2_vectors|gpu2e
gpurows|dynR|pe_gpu_rows_vectors|resetgraph dispenv2 timeout dmastart dmaq drawsync dispenv
gpuswap|dynR|pe_gpu_swap_vectors|
EOF
cat > "$OUT/runjob.sh" <<'EOF'
#!/bin/bash
IFS='|' read -r name kind mod args <<< "$1"
SELFREAD_OUT=$OUT/$name.json timeout 3000 python3 pc_port/tools/pe_vector_selfread_audit.py $kind $mod $OUT/$name.txt $args > $OUT/$name.log 2>&1
echo "exit=$? $(date -u +%FT%TZ)" >> $OUT/$name.log
EOF
chmod +x "$OUT/runjob.sh"
export OUT
date -u +%FT%TZ > "$OUT/start-utc.txt"
xargs -P "$JOBS" -d '\n' -n1 "$OUT/runjob.sh" < "$OUT/jobs.txt"
# the four state60 batches (older interpreter tree)
if [ -n "${STATE60_TREE:-}" ]; then
  for b in a b c d; do
    ( cd "$STATE60_TREE" && SELFREAD_OUT=$OUT/state60$b.json python3 pc_port/tools/pe_vector_selfread_audit.py dyn pe_state60_vectors $OUT/state60$b.dir state60$b > $OUT/state60$b.log 2>&1; echo "exit=$?" >> $OUT/state60$b.log )
  done
fi
# cdgen47 (uncommitted generator tree); captures: cdgen47-work/{b600,route-fs2}.cap
if [ -n "${CDHOST_TREE:-}" ] && [ -n "${CDX47_CAPS:-}" ]; then
  for spec in "ack 8007AAB4" "ack_silent 8007AAB4 --silent" "cmd 8007B558" "cmd_silent 8007B558 --silent" "sector 8007BF44" \
              "stream 8007C564" "dma 8007CEAC" "dma_silent 8007CEAC --silent"; do
    set -- $spec; n=$1; e=$2; shift 2
    ( cd "$CDHOST_TREE" && SELFREAD_OUT=$OUT/c47_$n.json python3 pc_port/tools/pe_vector_selfread_audit.py dyn pe_cdx_cdhost_vectors \
        $OUT/c47_$n.txt -n 400 --capture $CDX47_CAPS --only $e "$@" > $OUT/c47_$n.log 2>&1; echo "exit=$?" >> $OUT/c47_$n.log )
  done
fi
date -u +%FT%TZ > "$OUT/end-utc.txt"
python3 $T summ "$OUT"/*.json > "$OUT/summary.txt"
