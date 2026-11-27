#!/usr/bin/env bash
# cdgen63 / cdgen64: build the provider objects out of tree from the repo (black boxes) and execute the `BUILD-<name>:` lines of the set's checker
# header verbatim; run native, the three gpu2 mutants (1 no-op entry = cdx noop, 2 return flip = retflip, 3 store flip = storeflip), the vector file
# REVERSED (state carried between cases would change the count) and a link of the checker WITHOUT the provider object (must fail).  Usage (repo root):
#   G6_SET=cdgen63|cdgen64 G6_OUT=<dir> G6_VEC=<dir with the set's vector files> pe_gpu2_cdgen63_64_run.sh
#   cdgen63: vector file $G6_VEC/cdgen63.txt, providers: pe_plat2_press_dct
#   cdgen64: BUILD-stopped on $G6_VEC/cdgen64s.txt, BUILD-inline on $G6_VEC/cdgen64i.txt, providers: pe_plat_resetgraph_light_host pe_plat_gpu_gen9
set -uo pipefail
S=${G6_SET:?}; O=${G6_OUT:?}; V=${G6_VEC:?}
mkdir -p "$O"
case $S in
  cdgen63) OBJS="pe_plat2_press_dct"; SRC=pc_port/tools/pe_gpu2_cdgen63_check.c; declare -A VF=([strict]=cdgen63.txt [mapped]=cdgen63.txt) ;;
  cdgen64) OBJS="pe_plat_resetgraph_light_host pe_plat_gpu_gen9"; SRC=pc_port/tools/pe_gpu2_cdgen64_check.c; declare -A VF=([stopped]=cdgen64s.txt [inline]=cdgen64i.txt) ;;
  *) echo "unknown set"; exit 2 ;;
esac
# provider source per object name (the object NAME is what the checker BUILD lines expect, whatever file defines it): the ResetGraph light provider is
# pe_plat2_resetgraph_light.c since the clean-room rewrite (the symbol pe_plat_resetgraph_light_host is unchanged); the previous file name is the fallback;
# G6_RGLIGHT_SRC=<path> overrides it (e.g. a rewrite not yet merged into this tree; G6_EXTRA_INC=<dir> adds an include directory for its new headers)
for f in $OBJS; do
  src=pc_port/platform/$f.c
  if [ "$f" = pe_plat_resetgraph_light_host ]; then
    if [ -n "${G6_RGLIGHT_SRC:-}" ]; then src=$G6_RGLIGHT_SRC
    elif [ -f pc_port/platform/pe_plat2_resetgraph_light.c ]; then src=pc_port/platform/pe_plat2_resetgraph_light.c; fi
  fi
  cc -c -std=gnu11 -O2 ${G6_EXTRA_INC:+-I"$G6_EXTRA_INC"} -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap "$src" -o "$O/$f.o" || exit 1
done
for n in $(grep -o '^ \* BUILD-[A-Za-z0-9_-]*:' $SRC | sed 's/^ \* BUILD-//; s/:$//'); do
  vf="$V/${VF[$n]}"
  tac "$vf" > "$O/${n}_reversed.txt"
  for m in 0 1 2 3; do
    exe="$O/c_${S}_${n}_m$m"; flags=""; [ $m != 0 ] && flags="-DGPU2_MUTANT=$m"
    cmd=$(grep "^ \* BUILD-$n:" $SRC | sed "s/^ \* BUILD-$n: //; s#\$O#$O#g; s# -o [^ ]*\$# $flags -o $exe#")
    echo "BUILD $n mutant=$m: $cmd"
    bash -c "$cmd" || { echo "  build FAILED"; continue; }
    G2_VERBOSE=1 timeout 1500 "$exe" "$vf" > "$O/run_${n}_m$m.out" 2> "$O/run_${n}_m$m.err"; echo "$n/mutant=$m: $(tail -1 "$O/run_${n}_m$m.out")"
  done
  echo "$n/reversed: $(G2_VERBOSE=1 timeout 1500 "$O/c_${S}_${n}_m0" "$O/${n}_reversed.txt" 2>/dev/null | tail -1)"
  nolink=$(grep "^ \* BUILD-$n:" $SRC | sed "s/^ \* BUILD-$n: //; s#\$O#$O#g; s# -o [^ ]*\$# -o $O/nolink#; s# [^ ]*pe_plat2_press_dct\.o##; s# [^ ]*pe_plat_resetgraph_light_host\.o##; s# [^ ]*pe_plat_gpu_gen9\.o##")
  if bash -c "$nolink" >/dev/null 2>&1; then echo "$n/no-provider link: UNEXPECTEDLY LINKED"; else echo "$n/no-provider link: fails as required"; fi
done
