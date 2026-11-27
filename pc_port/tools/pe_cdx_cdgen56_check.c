/* Checker for the cdgen56 set (pe_cdx_cdgen52_61_vectors.py --set cdgen56): libds queue command 0x8007EE84(command, param, extra, callback).
 * Scaffold pe_cdx_common_canary.h (canary seeding, store-footprint category, controls noop / flip / retflip / storeflip / poke0 / same).  One provider object per
 * build, linked as a black box (pc_port/platform/pe_plat_cd_queue_command_host.c of the int3 tree, read-only).  Seams (from `nm -u` of the object; stand-ins here): pe_plat_ds_cq_next_slot() = 0x8007E6B0, pe_plat_cd_copy4(dst, src) = 0x80080950 (arity 2), pe_plat_ds_status_lane(idx) = 0x8007FBF0 (arity 1), pe_plat_ds_text_360() = 0x8007E8F4.
 * Build line (executed by pe_cdx_cdgen52_61_run.sh):
 * BUILD-queue_command: cc -std=gnu11 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen56_check.c $O/pe_plat_cd_queue_command_host.o -o $O/c56_queue_command
 * Run: $O/c56_queue_command <set>.txt [noop|flip|retflip|storeflip|poke0|same]   (CDX_CATEGORIES=1, CDX_LINES=1, CDX_DEBUG=1, CDX_TRACE=1) */
#include "pe_cdx_common_canary.h"

int PE_Port_ShouldStop(void) { return 0; }
void PE_Port_RequestStop(int r) { (void)r; bad_call = 1; trap_name = "PE_Port_RequestStop"; }
CALLEE(pe_plat_ds_cq_next_slot, 0x8007E6B0u, 0)
CALLEE(pe_plat_cd_copy4, 0x80080950u, 2)
CALLEE(pe_plat_ds_status_lane, 0x8007FBF0u, 1)
CALLEE(pe_plat_ds_text_360, 0x8007E8F4u, 0)
uint32_t pe_plat_cd_queue_command_host(uint32_t command, uint32_t param, uint32_t extra, uint32_t callback);
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    (void)a; (void)ret;
    if (entry != 0x8007EE84u) return -1;
    if (mutant == 1) { *ret = 0; return 1; }
    *ret = pe_plat_cd_queue_command_host(a[0], a[1], a[2], a[3]); return 1;
}
