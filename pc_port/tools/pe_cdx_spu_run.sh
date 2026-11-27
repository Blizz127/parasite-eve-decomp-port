#!/usr/bin/env bash
# Family cdx-spu checkers (clean-room provider platform/pe_plat2_cdx_spu.c): builds the three checker variants against
# the built provider objects and runs native / noop / flip on the lane's vector sets and the black-box sweeps.
# Run from the repository root after: ninja -C build/dev CMakeFiles/pe_field_runtime.dir/platform/pe_plat2_cdx_spu.c.o \
#   CMakeFiles/pe_field_runtime.dir/platform/pe_plat2_spu_a.c.o CMakeFiles/pe_field_runtime.dir/platform/pe_plat2_spu_b.c.o
# V = vector root (default the lane scratch), X = extra sweeps (V/cdgen44-cdxspu).
set -uo pipefail
V=${V:?set V to the directory holding the cdgen44/46 vector sets}
X=${X:-$V/cdgen44-cdxspu}
O=build/dev/CMakeFiles/pe_field_runtime.dir/platform
INC="-Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap"
CF="-std=gnu11 -Wall -Wextra -Werror"
# CDX_DEFS: identifier renames for a tree whose bindings moved on (applied to the checker sources only), e.g. the int3 tip after
# the cdx-etc / state60 spu_b merges:
#   CDX_DEFS="-Dpe_plat_bios_enter_critical=pe_plat_cdxetc_enter_critical -Dpe_plat_bios_exit_critical=pe_plat_cdxetc_exit_critical -Dpe_plat_spu_voice_volume=pe_plat_spub_voice_volume -Dpe_plat_spu_set_common_attributes=pe_plat_spub_set_common_attributes"
CD=${CDX_DEFS:-}
run() { echo "\$ $*"; "$@"; echo "  exit=$?"; }
run cc $CF $CD $INC pc_port/tools/pe_cdx_cdgen44_cdxspu_check.c $O/pe_plat2_cdx_spu.c.o -o build/check44cdxspu
run cc $CF $CD $INC pc_port/tools/pe_cdx_cdgen44_regonly_check.c $O/pe_plat2_cdx_spu.c.o $O/pe_plat2_spu_a.c.o -o build/check44reg
run objcopy --weaken-symbol=pe_plat_cdxspu_install_dma_event $O/pe_plat2_cdx_spu.c.o build/chk46_cdxspu.o
run objcopy --weaken-symbol=pe_plat_spua_set_reverb_enabled $O/pe_plat2_spu_a.c.o build/chk46_spu_a.o
run objcopy --weaken-symbol=pe_plat_spub_voice_volume --weaken-symbol=pe_plat_spub_set_common_attributes $O/pe_plat2_spu_b.c.o build/chk46_spu_b.o
run cc $CF $CD $INC pc_port/tools/pe_cdx_cdgen46_cdxspu_check.c build/chk46_cdxspu.o build/chk46_spu_a.o build/chk46_spu_b.o -o build/check46cdxspu
chk() { local bin=$1 f=$2 m; for m in "" noop flip; do echo "\$ $bin $f $m"; ./$bin $f $m 2>&1 | grep -E "^mode|^  [0-9A-F]{8} cases"; echo "  exit=${PIPESTATUS[0]}"; done; }
chk build/check44cdxspu $V/cdgen44/vectors.txt
chk build/check44reg $V/cdgen44/regonly.txt
chk build/check46cdxspu $V/cdgen46/vectors.txt
chk build/check46cdxspu $V/cdgen46/edge.txt
# cdx-spu2: sets regenerated with the own-written-load fix (the old rows seed 0x8009B414 from 80085EB4's own readback)
chk build/check46cdxspu $V/cdgen46/vectors_s2.txt
chk build/check46cdxspu $V/cdgen46/edge_s2.txt
chk build/check44reg $V/cdgen44/regonly_s2.txt
for f in cba8.txt cba8_sweep.txt imm.txt imm2.txt imm3.txt other_sweep.txt; do echo "\$ build/check44cdxspu $X/$f"; ./build/check44cdxspu $X/$f 2>&1 | grep -E "^mode"; done
echo "\$ build/check46cdxspu $X/sweep46.txt"; ./build/check46cdxspu $X/sweep46.txt 2>&1 | grep -E "^mode"
echo "\$ build/check44reg $X/regonly400.txt"; ./build/check44reg $X/regonly400.txt 2>&1 | grep -E "^mode|^  [0-9A-F]{8} cases"
# long: cba8_30k.txt cba8_sweep4k.txt (about 2 minutes each)
