/* Checker for the cdgen55 set (pe_cdx_cdgen52_61_vectors.py --set cdgen55): libds command sequence 0x8007F0C8(mode, location, command, extra, callback).
 * Scaffold pe_cdx_common_canary.h (canary seeding, store-footprint category, controls noop / flip / retflip / storeflip / poke0 / same).  One provider object per
 * build, linked as a black box (pc_port/platform/pe_plat_cd_command_sequence_host.c of the int3 tree, read-only).  Seams (from `nm -u` of the object; stand-ins here): pe_plat_cd_pos_to_int(loc) = 0x80080C48 (canonical 0x8007AA34; arity 1), pe_plat_ds_cq_next_slot() = 0x8007E6B0, pe_plat_ds_status_lane(idx) = 0x8007FBF0 (arity 1), pe_plat_ds_text_360() = 0x8007E8F4; the copy helper 0x80080950 has no bound symbol in this provider (runs inline in the vectors).
 * Build line (executed by pe_cdx_cdgen52_61_run.sh):
 * BUILD-command_sequence: cc -std=gnu11 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen55_check.c $O/pe_plat_cd_command_sequence_host.o -o $O/c55_command_sequence
 * Run: $O/c55_command_sequence <set>.txt [noop|flip|retflip|storeflip|poke0|same]   (CDX_CATEGORIES=1, CDX_LINES=1, CDX_DEBUG=1, CDX_TRACE=1) */
#include "pe_cdx_common_canary.h"

int PE_Port_ShouldStop(void) { return 0; }
void PE_Port_RequestStop(int r) { (void)r; bad_call = 1; trap_name = "PE_Port_RequestStop"; }
CALLEE(pe_plat_cd_pos_to_int, 0x8007AA34u, 1)
CALLEE(pe_plat_ds_cq_next_slot, 0x8007E6B0u, 0)
CALLEE(pe_plat_ds_status_lane, 0x8007FBF0u, 1)
CALLEE(pe_plat_ds_text_360, 0x8007E8F4u, 0)
int pe_plat_cd_command_sequence_host(uint32_t mode, uint32_t location, int command, uint32_t extra, uint32_t callback);
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    (void)a; (void)ret;
    if (entry != 0x8007F0C8u) return -1;
    if (mutant == 1) { *ret = 0; return 1; }
    *ret = (uint32_t)pe_plat_cd_command_sequence_host(a[0], a[1], (int)a[2], a[3], a[4]); return 1;
}
