/* Checker for the cdgen57 set (pe_cdx_cdgen52_61_vectors.py --set cdgen57): libds read stream open 0x80081314(location, mode).
 * Scaffold pe_cdx_common_canary.h (canary seeding, store-footprint category, controls noop / flip / retflip / storeflip / poke0 / same).  One provider object per
 * build, linked as a black box (pc_port/platform/pe_plat_cd_read_stream_host.c of the int3 tree, read-only).  Seams (from `nm -u` of the object; stand-ins here): pe_plat_cd_command_sequence_host = 0x8007F0C8 (arity 5), pe_plat_cd_ds_set_ready_callback(cb) = 0x800824C8, pe_plat_ds_data_callback(cb) = 0x800824F0; PE_CdReg_DeviceEnabled() = 1; PE_StrFeed_Open / PE_StrFeed_Probe are host-only (counted, 0).
 * Build line (executed by pe_cdx_cdgen52_61_run.sh):
 * BUILD-read_stream: cc -std=gnu11 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen57_check.c $O/pe_plat_cd_read_stream_host.o -o $O/c57_read_stream
 * BUILD-read_stream_devoff_probe0: cc -std=gnu11 -Wall -Wextra -Werror -DC57_DEV_ENABLED=0 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen57_check.c $O/pe_plat_cd_read_stream_host.o -o $O/c57_devoff_probe0
 * BUILD-read_stream_devoff_probe1: cc -std=gnu11 -Wall -Wextra -Werror -DC57_DEV_ENABLED=0 -DC57_PROBE=1 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen57_check.c $O/pe_plat_cd_read_stream_host.o -o $O/c57_devoff_probe1
 * BUILD-read_stream_mix: cc -std=gnu11 -Wall -Wextra -Werror -DC57_MIX=1 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen57_check.c $O/pe_plat_cd_read_stream_host.o -o $O/c57_mix
 * (the three extra builds only change the host-model answers of PE_CdReg_DeviceEnabled / PE_StrFeed_Probe: the original has NO counterpart of either - the retail entry always issues the command sequence - so the vectors expect the retail flow whatever they answer; PE_StrFeed_Open stays a counted host-only call.  MIX: device enabled from mode bit 12, probe from location bit 2.)
 * Run: $O/c57_read_stream <set>.txt [noop|flip|retflip|storeflip|poke0|same]   (CDX_CATEGORIES=1, CDX_LINES=1, CDX_DEBUG=1, CDX_TRACE=1) */
#include "pe_cdx_common_canary.h"

int PE_Port_ShouldStop(void) { return 0; }
void PE_Port_RequestStop(int r) { (void)r; bad_call = 1; trap_name = "PE_Port_RequestStop"; }
#ifndef C57_DEV_ENABLED
#define C57_DEV_ENABLED 1
#endif
#ifndef C57_PROBE
#define C57_PROBE 0
#endif
static int dev_en = C57_DEV_ENABLED, probe_v = C57_PROBE;
uint32_t PE_CdReg_DeviceEnabled(void) { return (uint32_t)dev_en; }
int PE_StrFeed_Probe(uint32_t location) { (void)location; host_only_calls++; return probe_v; }
IGNORE(PE_StrFeed_Open)
CALLEE(pe_plat_cd_command_sequence_host, 0x8007F0C8u, 5)
CALLEE(pe_plat_cd_ds_set_ready_callback, 0x800824C8u, 1)
CALLEE(pe_plat_ds_data_callback, 0x800824F0u, 1)
int pe_plat_cd_read_stream_host(uint32_t location, uint32_t mode);
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    (void)a; (void)ret;
    if (entry != 0x80081314u) return -1;
#if C57_MIX
    dev_en = (a[1] >> 12) & 1; probe_v = (a[0] >> 2) & 1;
#endif
    if (mutant == 1) { *ret = 0; return 1; }
    *ret = (uint32_t)pe_plat_cd_read_stream_host(a[0], a[1]); return 1;
}
