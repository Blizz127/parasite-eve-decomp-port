#!/usr/bin/env bash
# usage: pe_cdharness_relink.sh  (run from the repo root after `ninja -C build/cap parasite-eve-port`)
# Relinks the SAME objects into build/cap/pe-cdh with the cdharness --wrap logger (pe_cdharness_capture.c).  No provider / CMake edit.
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd); cd "$ROOT"
cc -std=gnu11 -O1 -Wall -Wextra -Ipc_port/platform -Ipc_port/include -Ipc_port/bootstrap -c pc_port/tools/pe_cdharness_capture.c -o build/cdh_wrap.o
W=pe_plat_cd_acknowledge_host,pe_plat_cd_status_poll_host,pe_plat_cd_command_issue_host,pe_plat_cd_sector_dma_host,pe_plat_cd_stream_handler_host,pe_plat_cd_dma_issue_host,PE_CdLoadU8,PE_CdLoadU32,PE_GPU_ReadDPCR,PE_GPU_ReadDICR,PE_GuestCall,PE_Cd_DispatchDataCallback,PE_Port_RequestStop,pe_plat_cd_stream_stop,PE_Printf_Guest,func_801214D4,func_80191DC8,ovl_03C5__func_801214D4,ovl_03D2__func_80191DC8,pe_plat_ds_813e8
cd build/cap
cc -O3 -DNDEBUG -Wl,$(echo $W | sed 's/\([^,]*\)/--wrap=\1/g') "$ROOT/build/cdh_wrap.o" \
  CMakeFiles/parasite-eve-port.dir/src/port_main.c.o CMakeFiles/parasite-eve-port.dir/platform/host_window.c.o CMakeFiles/parasite-eve-port.dir/platform/host_pad.c.o \
  CMakeFiles/parasite-eve-port.dir/platform/host_pad_sdl.c.o CMakeFiles/parasite-eve-port.dir/platform/host_dev_menu.c.o CMakeFiles/parasite-eve-port.dir/platform/host_pad_remap.c.o \
  CMakeFiles/parasite-eve-port.dir/platform/host_key_merge.c.o CMakeFiles/parasite-eve-port.dir/platform/host_audio.c.o CMakeFiles/parasite-eve-port.dir/game/decomp_ovl/_dispatch/zz_dispatch.c.o \
  -o pe-cdh libpe_field_runtime.a -lm -ldl
echo relinked build/cap/pe-cdh; objdump -d pe-cdh | grep -c "__wrap_"
