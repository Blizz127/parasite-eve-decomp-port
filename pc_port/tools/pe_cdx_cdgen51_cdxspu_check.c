/* Vector check for batch cdgen51 (pe_cdx_cdgen51_vectors.py): libspu SpuClearReverbWorkArea, retail 0x8008D610.  Variant of
 * pe_cdx_cdgen51_check.c (the vector lane's checker: same scaffold pe_cdx_common_canary.h, same case loop, stand-ins and controls) for the
 * clean-room provider platform/pe_plat2_cdx_spu.c (family cdx-spu), entry pe_plat_cdxspu_clear_work_area.  The provider writes the transfer
 * footprint itself through PE_Load/PE_Store (SPU registers, SPU_DELAY, DMA4), so the host seams of the lane's translated object
 * (PE_SpuRegister_*, PE_SpuDma_Begin) are not used; the stand-ins here: pe_plat_spub_heap_contains = 0x80085BB4 (scripted result), PE_Event_ConsumeSpuDma(handle)
 * = the event wait 0x8008D7B0 (scripted result, ignored by the original), PE_SpuDma_DeliverPending / PE_SpuDma_Service host-only (counted), the SPU_DELAY
 * leaves 0x8007DC84 / 0x8007DC5C and the port clock tick are the real clean-room leaves (pe_plat2_spu_a.o) / a no-op; every other symbol the objects leave
 * undefined is a trap (a call fails the case).
 * Build (provider objects from the cmake tree, `CDX_DEFS`-style renames for the int3 tip are not needed here):
 * BUILD-cdxspu: cc -std=gnu11 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen51_cdxspu_check.c $O/pe_plat2_cdx_spu.c.o $O/pe_plat2_spu_a.c.o -o $O/c51_cdxspu
 * Run: $O/c51_cdxspu clear.txt [noop|flip|retflip|storeflip|poke0|same]   (CDX_CATEGORIES=1, CDX_LINES=1, CDX_DEBUG=1, CDX_TRACE=1) */
#include "pe_cdx_common_canary.h"

int PE_Port_ShouldStop(void) { return 0; }
void PE_Port_RequestStop(int r) { (void)r; bad_call = 1; trap_name = "PE_Port_RequestStop"; }
CALLEE(pe_plat_spub_heap_contains, 0x80085BB4u, 1)
int PE_Event_ConsumeSpuDma(int handle) { return (int)cdx_callee(0x8008D7B0u, 1, (uint32_t)handle, 0, 0, 0, 0); }
int PE_SpuDma_DeliverPending(void) { host_only_calls++; return 1; }
int PE_SpuDma_Service(void) { host_only_calls++; return 1; }
void pe_plat_sio_poll_tick(void) {}
void PE_Decomp_GuestTrapDivZero(const char *fn) { (void)fn; bad_call = 1; trap_name = "PE_Decomp_GuestTrapDivZero"; }
/* referenced by the other functions of the same object: a call fails the case */
/* referenced by the other functions of the provider objects: a call fails the case */
TRAP(func_80085084) TRAP(PE_Event_Enable) TRAP(PE_Event_Open) TRAP(PE_Event_SpuDmaEnabled)
TRAP(pe_plat_ctl_counter_setup) TRAP(pe_plat_ctl_counter_start) TRAP(pe_plat_spub_set_common_attributes)
TRAP(pe_plat_spub_voice_volume) TRAP(pe_plat_spu_set_reverb_mode_type) TRAP(pe_plat_spuupload_upload) TRAP(PE_SpuDma_WaitForCompletion)
TRAP(pe_plat_cdxetc_enter_critical) TRAP(pe_plat_cdxetc_exit_critical) TRAP(pe_plat_reset_callback) TRAP(PE_SpuDma_InstallIrq) TRAP(pe_plat_spub_set_transfer_address)
TRAP(pe_plat_bios_close_event) TRAP(pe_plat_bios_disable_event) TRAP(pe_plat_dma_callback) TRAP(PE_Printf_Guest)
int pe_plat_cdxspu_clear_work_area(int mode);
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    if (entry != 0x8008D610u) return -1;
    if (mutant == 1) { *ret = 0; return 1; }
    *ret = (uint32_t)pe_plat_cdxspu_clear_work_area((int)a[0]);
    return 1;
}
