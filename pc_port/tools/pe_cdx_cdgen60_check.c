/* Checker for the cdgen60 set (pe_cdx_cdgen52_61_vectors.py --set cdgen60): libcd unexpected-status handler 0x8008068C(status, response).
 * Scaffold pe_cdx_common_canary.h (canary seeding, store-footprint category, controls noop / flip / retflip / storeflip / poke0 / same).  One provider object per
 * build, linked as a black box (pc_port/platform/pe_plat_cd_unexpected_status.c of the int3 tree, read-only).  Seams (from `nm -u` of the object; stand-ins here): PE_Cd_DispatchDataCallback(target, status, response) = the retail callee `target` (arity 2), result 1 (handled); the callback words are seeded with the stopped sentinels 0x80131140 / 0x80131180 / 0x801311C0 and the runtime targets 0x80080778 / 0x80080164.
 * Build line (executed by pe_cdx_cdgen52_61_run.sh):
 * BUILD-unexpected_status: cc -std=gnu11 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen60_check.c $O/pe_plat_cd_unexpected_status.o -o $O/c60_unexpected_status
 * BUILD-unexpected_status_mapped: cc -std=gnu11 -Wall -Wextra -Werror -DC60_ARITY1=1 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen60_check.c $O/pe_plat_cd_unexpected_status.o -o $O/c60_unexpected_status_mapped
 * Run: $O/c60_unexpected_status <set>.txt [noop|flip|retflip|storeflip|poke0|same]   (CDX_CATEGORIES=1, CDX_LINES=1, CDX_DEBUG=1, CDX_TRACE=1) */
#include "pe_cdx_common_canary.h"

int PE_Port_ShouldStop(void) { return 0; }
void PE_Port_RequestStop(int r) { (void)r; bad_call = 1; trap_name = "PE_Port_RequestStop"; }
int PE_Cd_DispatchDataCallback(pe_addr_t target, uint32_t status, pe_addr_t response)
{
#if C60_ARITY1
    (void)response;            /* mapped: the original calls the callback through its word with ONE argument register set (the status, 5); the second is a leftover */
    (void)cdx_callee(target, 1, status, 0, 0, 0, 0);
#else
    (void)cdx_callee(target, 2, status, response, 0, 0, 0);
#endif
    return 1;
}
void pe_plat_cd_unexpected_status(uint32_t status, uint32_t response);
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    (void)a; (void)ret;
    if (entry != 0x8008068Cu) return -1;
    if (mutant == 1) { return 0; }
    pe_plat_cd_unexpected_status(a[0], a[1]); return 0;
}
