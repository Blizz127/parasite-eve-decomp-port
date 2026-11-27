/* Checker for the cdgen54 set (pe_cdx_cdgen52_61_vectors.py --set cdgen54): libcd response query 0x8007F418(sequence, response).
 * Scaffold pe_cdx_common_canary.h (canary seeding, store-footprint category, controls noop / flip / retflip / storeflip / poke0 / same).  One provider object per
 * build, linked as a black box (pc_port/platform/pe_plat_cd_response_query_host.c of the int3 tree, read-only).  Seams (from `nm -u` of the object; stand-ins here): pe_plat_cd_poll_sync(result) = 0x8007FC64 (arity 1), pe_plat_cd_copy8(dst, src) = 0x80080998 (arity 2); HostFB_DevicePoll and PE_Port_EscapePoint are host-only (counted).
 * Build line (executed by pe_cdx_cdgen52_61_run.sh):
 * BUILD-response_query: cc -std=gnu11 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen54_check.c $O/pe_plat_cd_response_query_host.o -o $O/c54_response_query
 * BUILD-response_query_effect: cc -std=gnu11 -Wall -Wextra -Werror -DC54_COPY_EFFECT=1 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen54_check.c $O/pe_plat_cd_response_query_host.o -o $O/c54_response_query_effect
 * (effect build, for cdgen54c: the copy helper 0x80080998 runs INLINE in the vectors, so the stand-in performs its data effect through PE_LoadU8 / PE_StoreU8: both pointers nonzero: eight bytes copied forward byte by byte; source null: a zero byte stored at the destination; destination null: nothing; the call is not recorded)
 * Run: $O/c54_response_query <set>.txt [noop|flip|retflip|storeflip|poke0|same]   (CDX_CATEGORIES=1, CDX_LINES=1, CDX_DEBUG=1, CDX_TRACE=1) */
#include "pe_cdx_common_canary.h"

int PE_Port_ShouldStop(void) { return 0; }
void PE_Port_RequestStop(int r) { (void)r; bad_call = 1; trap_name = "PE_Port_RequestStop"; }
IGNORE(HostFB_DevicePoll) IGNORE(PE_Port_EscapePoint)
CALLEE(pe_plat_cd_poll_sync, 0x8007FC64u, 1)
#if C54_COPY_EFFECT
void pe_plat_cd_copy8(uint32_t dst, uint32_t src)
{
    if (!dst) return;
    if (!src) { PE_StoreU8(dst, 0); return; }
    for (uint32_t i = 0; i < 8; i++) { uint8_t b = PE_LoadU8(src + i); PE_StoreU8(dst + i, b); }
}
#else
CALLEE(pe_plat_cd_copy8, 0x80080998u, 2)
#endif
int pe_plat_cd_response_query_host(uint32_t sequence, uint32_t response);
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    (void)a; (void)ret;
    if (entry != 0x8007F418u) return -1;
    if (mutant == 1) { *ret = 0; return 1; }
    *ret = (uint32_t)pe_plat_cd_response_query_host(a[0], a[1]); return 1;
}
