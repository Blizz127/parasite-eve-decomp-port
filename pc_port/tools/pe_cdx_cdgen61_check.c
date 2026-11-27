/* Checker for the cdgen61 set (pe_cdx_cdgen52_61_vectors.py --set cdgen61): libcd data-ready callback 0x8007C214().
 * Scaffold pe_cdx_common_canary.h (canary seeding, store-footprint category, controls noop / flip / retflip / storeflip / poke0 / same).  One provider object per
 * build, linked as a black box (pc_port/platform/pe_plat_cd_data_ready.c of the int3 tree, read-only).  Seams (from `nm -u` of the object; stand-ins here): PE_GuestHook_Call(slot, caller, fn, arity, a0..a3, &ret) = the retail callee `fn` with `arity` arguments (result 0, ran); the chain word 0x800B0CC8 is seeded 0 or a stopped sentinel (arity 3 recorded).
 * Build line (executed by pe_cdx_cdgen52_61_run.sh):
 * BUILD-data_ready: cc -std=gnu11 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen61_check.c $O/pe_plat_cd_data_ready.o -o $O/c61_data_ready
 * BUILD-data_ready_mapped: cc -std=gnu11 -Wall -Wextra -Werror -DC61_ARITY3=1 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen61_check.c $O/pe_plat_cd_data_ready.o -o $O/c61_data_ready_mapped
 * Run: $O/c61_data_ready <set>.txt [noop|flip|retflip|storeflip|poke0|same]   (CDX_CATEGORIES=1, CDX_LINES=1, CDX_DEBUG=1, CDX_TRACE=1) */
#include "pe_cdx_common_canary.h"

int PE_Port_ShouldStop(void) { return 0; }
void PE_Port_RequestStop(int r) { (void)r; bad_call = 1; trap_name = "PE_Port_RequestStop"; }
#include "pe_guestcode.h"
int PE_GuestHook_Call(const char *slot, const char *caller, pe_addr_t fn, unsigned arity, uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3, int *ret)
{
    (void)slot; (void)caller;
#if C61_ARITY3
    if (arity > 3u) arity = 3u;    /* mapped: the original sets three argument registers; the fourth is a leftover, not compared */
#endif
    (void)cdx_callee(fn, arity, (uint32_t)a0, (uint32_t)a1, (uint32_t)a2, (uint32_t)a3, 0); if (ret) *ret = 0; return 1;
}
void pe_plat_cd_data_ready_callback(void);
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    (void)a; (void)ret;
    if (entry != 0x8007C214u) return -1;
    if (mutant == 1) { return 0; }
    pe_plat_cd_data_ready_callback(); return 0;
}
