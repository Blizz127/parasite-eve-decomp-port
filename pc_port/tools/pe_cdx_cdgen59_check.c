/* Checker for the cdgen59 set (pe_cdx_cdgen52_61_vectors.py --set cdgen59): libcd initialize callbacks 0x8007F994().
 * Scaffold pe_cdx_common_canary.h (canary seeding, store-footprint category, controls noop / flip / retflip / storeflip / poke0 / same).  One provider object per
 * build, linked as a black box (pc_port/platform/pe_plat_cd_initialize_callbacks.c of the int3 tree, read-only).  Seams (from `nm -u` of the object; stand-ins here): pe_plat_cdxstream_initialize_device = 0x8007BBFC, pe_plat_cd_init_vol = 0x8007BAC0, pe_plat_cdxstream_initialize_state = 0x8007FA2C, pe_plat_cd_set_guarded_state(v) = 0x800812F4 (arity 1), pe_plat_cdxetc_vsync_callbacks(slot, handler) = 0x80073D58 (arity 2); PE_Callback_Bind is host-only (counted); pe_plat_cdxstream_status_pump is referenced by address only.
 * Build line (executed by pe_cdx_cdgen52_61_run.sh):
 * BUILD-initialize_callbacks: cc -std=gnu11 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen59_check.c $O/pe_plat_cd_initialize_callbacks.o -o $O/c59_initialize_callbacks
 * Run: $O/c59_initialize_callbacks <set>.txt [noop|flip|retflip|storeflip|poke0|same]   (CDX_CATEGORIES=1, CDX_LINES=1, CDX_DEBUG=1, CDX_TRACE=1) */
#include "pe_cdx_common_canary.h"

int PE_Port_ShouldStop(void) { return 0; }
void PE_Port_RequestStop(int r) { (void)r; bad_call = 1; trap_name = "PE_Port_RequestStop"; }
IGNORE(PE_Callback_Bind)
void pe_plat_cdxstream_status_pump(void) { bad_call = 1; trap_name = "pe_plat_cdxstream_status_pump"; }
CALLEE(pe_plat_cdxstream_initialize_device, 0x8007BBFCu, 0)
CALLEE(pe_plat_cd_init_vol, 0x8007BAC0u, 0)
CALLEE(pe_plat_cdxstream_initialize_state, 0x8007FA2Cu, 0)
CALLEE(pe_plat_cd_set_guarded_state, 0x800812F4u, 1)
CALLEE(pe_plat_cdxetc_vsync_callbacks, 0x80073D58u, 2)
int pe_plat_cd_initialize_callbacks(void);
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    (void)a; (void)ret;
    if (entry != 0x8007F994u) return -1;
    if (mutant == 1) { *ret = 0; return 1; }
    *ret = (uint32_t)pe_plat_cd_initialize_callbacks(); return 1;
}
