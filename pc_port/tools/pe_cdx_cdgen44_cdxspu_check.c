/* Vector check for batch cdgen44 (pe_cdx_vectors.py cdgen44): the SPU / streaming family:
 * libsnd (retail 0x8007D15C SPU DMA event installer, 0x8007D054 default init, 0x8007D074 init wrapper;
 * platform/pe_libsnd.c) and the stream engine pieces of platform/pe_stream.c (0x8008CB54 reverb mode
 * transition, 0x8008CBA8 streaming command dispatcher).  Providers are linked as built objects, never read.
 * Retail callees that the provider reaches through a host symbol are recorded stand-ins (arguments as the
 * retail callee takes them); retail callees with no host symbol run inline in the vectors (their register /
 * RAM effects are part of the entry).  BIOS OpenEvent / EnableEvent are the veneers 0x800726E4 / 0x80072704
 * (the provider calls them through PE_Event_Open / PE_Event_Enable).  Host-only seams (PE_SpuDma_InstallIrq)
 * are ignored.  Scaffold: pe_cdx_common.h.
 * Variant of pe_cdx_cdgen44_check.c for the clean-room provider platform/pe_plat2_cdx_spu.c (family cdx-spu):
 * identical case loop, entry dispatcher and stand-in semantics; only the two remaining stand-ins that the provider
 * reaches through the pe_plat/spu_a.h bindings (func_8007D1D4 / 8007DAE0 -> pe_plat_spua_initialize / _store_value)
 * carry those names instead of the old pe_plat_spu_* ones, and the entries under test are the pe_plat_cdxspu_* symbols.
 * Build (object: ninja -C build/dev CMakeFiles/pe_field_runtime.dir/platform/pe_plat2_cdx_spu.c.o):
 * BUILD: cc -std=gnu11 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen44_cdxspu_check.c build/dev/CMakeFiles/pe_field_runtime.dir/platform/pe_plat2_cdx_spu.c.o -o check44cdxspu
 * Run: ./check44 vectors.txt [noop|flip]
 */
#include "pe_cdx_common.h"
CALLEE(PE_Event_Open, 0x800726E4u, 4)
CALLEE(PE_Event_Enable, 0x80072704u, 1)
CALLEE(pe_plat_bios_enter_critical, 0x80072714u, 0)
CALLEE(pe_plat_bios_exit_critical, 0x80072724u, 0)
CALLEE(pe_plat_reset_callback, 0x80073C94u, 0)
CALLEE(pe_plat_spua_initialize, 0x8007D1D4u, 1)
CALLEE(pe_plat_spua_register_dma_callback, 0x8007DD14u, 1)
CALLEE(pe_plat_spua_store_value, 0x8007DAE0u, 3)
CALLEE(pe_plat_spua_set_reverb_enabled, 0x80085A64u, 1)
CALLEE(pe_plat_spu_set_reverb_mode_type, 0x8008CF70u, 1)
CALLEE(func_80085084, 0x80085084u, 1)
/* Host IRQ hook for the SPU DMA completion: no retail counterpart, effect not compared.  Its result is part of
 * the host seam contract: the native branches on it (0 = install failed -> it skips the DMA callback / BIOS event
 * registration the retail code performs; with 1 = installed the retail sequence is reproduced).  Checked both ways. */
uint32_t PE_SpuDma_InstallIrq(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, uintptr_t e)
{ (void)a; (void)b; (void)c; (void)d; (void)e; host_only_calls++; return 1; }
/* host seams not reached by these entries: a call fails the case */
TRAP(func_800850F4) TRAP(PE_Event_ConsumeSpuDma) TRAP(PE_Event_SpuDmaEnabled) TRAP(pe_plat_ctl_counter_setup)
TRAP(pe_plat_ctl_counter_start) TRAP(pe_plat_spu_heap_contains) TRAP(pe_plat_spu_set_common_attributes)
TRAP(pe_plat_spu_voice_volume) TRAP(PE_SpuDma_Begin) TRAP(PE_SpuDma_DeliverPending) TRAP(PE_SpuDma_Service)
TRAP(PE_SpuDma_WaitForCompletion) TRAP(PE_SpuRegister_LoadU16) TRAP(PE_SpuRegister_StoreU16)
/* symbols of the provider object that these entries do not reach (the DMA adapters, the stream bring-up): a call fails the case */
TRAP(pe_plat_spua_dma_priority) TRAP(pe_plat_spua_dma_priority_enabled) TRAP(pe_plat_spua_configure_heap)
/* cdx-spu2: only the stream bring-up (not an entry of this batch) calls the transfer-address leaf of platform/pe_plat2_spu_b.c */
TRAP(pe_plat_spub_set_transfer_address)
/* cdx-spu2: only the reverb work-area clear (not an entry of this batch) reaches these */
TRAP(pe_plat_spub_heap_contains)

void pe_plat_sio_poll_tick(void) { host_only_calls++; }
int PE_Port_ShouldStop(void) { return 0; }
void PE_Port_RequestStop(int r) { (void)r; bad_call = 1; trap_name = "PE_Port_RequestStop"; }
void pe_plat_cdxspu_install_dma_event(void); void pe_plat_cdxspu_init_default(void); void pe_plat_cdxspu_init_wrapper(int);
void pe_plat_cdxspu_reverb_transition(uint32_t); int pe_plat_cdxspu_stream_dispatch(void);
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    if (mutant == 1) return entry == 0x8008CBA8u ? (*ret = 0, 1) : 0;
    switch (entry) {
    case 0x8007D15Cu: pe_plat_cdxspu_install_dma_event(); return 0;
    case 0x8007D054u: pe_plat_cdxspu_init_default(); return 0;
    case 0x8007D074u: pe_plat_cdxspu_init_wrapper((int)a[0]); return 0;
    case 0x8008CB54u: pe_plat_cdxspu_reverb_transition(a[0]); return 0;
    case 0x8008CBA8u: *ret = (uint32_t)pe_plat_cdxspu_stream_dispatch(); return 1;
    }
    return -1;
}
