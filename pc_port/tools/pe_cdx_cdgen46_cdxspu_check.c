/* Vector check for batch cdgen46 (pe_cdx_vectors.py cdgen46): the stream (SPU) bring-up func_80085644 of
 * platform/pe_stream.c.  Black-box: the provider is linked as a built object (never read).  The retail callees that
 * the current object reaches through a bound host symbol (names from `nm -u` + the public pe_plat headers) are
 * recorded stand-ins; the retail libspu helpers with no host symbol ran inline in the vectors (their SPU register
 * accesses and RAM writes are part of the entry).  Scaffold: pe_cdx_common.h.
 * Argument widths: the public prototypes of pe_plat_ctl_counter_setup (unsigned short, short, int) and
 * pe_plat_spu_voice_volume (int, unsigned short, unsigned short, short, short) take 16-bit values where retail
 * passes full registers; want_norm compares them at the prototype's width (the retail 0xF2000002 counter id is
 * compared as 0x0002).
 * Variant of pe_cdx_cdgen46_check.c for the clean-room provider platform/pe_plat2_cdx_spu.c (family cdx-spu), which also
 * calls the clean-room heap configuration leaf 80085A04 of platform/pe_plat2_spu_a.c as real code (the vectors run it
 * inline): the case loop, the stand-in semantics and the entry are those of the original checker; changes: the 7D15C
 * stand-in is pe_plat_cdxspu_install_dma_event (the provider's own symbol, weakened in the build line so that the stand-in
 * replaces it; the provider marks it noinline), pe_plat_spua_set_reverb_enabled is weakened in the spu_a copy for the same
 * reason, the entry is pe_plat_cdxspu_stream_bringup, and the stand-ins the two objects leave undefined are added
 * (PE_SpuDma_InstallIrq = host-only, returns 1; pe_plat_sio_poll_tick = no-op; the rest TRAP).
 * cdx-spu2: the bring-up now calls the clean-room transfer-address leaf func_80085EB4 (pe_plat_spub_set_transfer_address of
 * platform/pe_plat2_spu_b.c) as real code, like 80085A04 (the vectors run both inline), so the spu_b object is linked as well with
 * its voice-volume and common-attributes leaves weakened (the stand-ins above replace them).
 * Build (objects: ninja -C build/dev CMakeFiles/pe_field_runtime.dir/platform/pe_plat2_cdx_spu.c.o CMakeFiles/pe_field_runtime.dir/platform/pe_plat2_spu_a.c.o):
 * BUILD: objcopy --weaken-symbol=pe_plat_cdxspu_install_dma_event build/dev/CMakeFiles/pe_field_runtime.dir/platform/pe_plat2_cdx_spu.c.o build/chk46_cdxspu.o && objcopy --weaken-symbol=pe_plat_spua_set_reverb_enabled build/dev/CMakeFiles/pe_field_runtime.dir/platform/pe_plat2_spu_a.c.o build/chk46_spu_a.o && objcopy --weaken-symbol=pe_plat_spub_voice_volume --weaken-symbol=pe_plat_spub_set_common_attributes build/dev/CMakeFiles/pe_field_runtime.dir/platform/pe_plat2_spu_b.c.o build/chk46_spu_b.o && cc -std=gnu11 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen46_cdxspu_check.c build/chk46_cdxspu.o build/chk46_spu_a.o build/chk46_spu_b.o -o check46cdxspu
 * Run: ./check46 vectors.txt [noop|flip]
 */
#include "pe_cdx_common.h"
CALLEE(PE_Event_Open, 0x800726E4u, 4)
CALLEE(PE_Event_Enable, 0x80072704u, 1)
CALLEE(pe_plat_cdxspu_install_dma_event, 0x8007D15Cu, 0)
CALLEE(func_800850F4, 0x800850F4u, 2)
CALLEE(pe_plat_ctl_counter_start, 0x800858E8u, 1)
CALLEE(pe_plat_spua_set_reverb_enabled, 0x80085A64u, 1)
CALLEE(pe_plat_spu_set_reverb_mode_type, 0x8008CF70u, 1)
CALLEE(pe_plat_spu_heap_contains, 0x80085BB4u, 1)
CALLEE(pe_plat_spu_set_common_attributes, 0x80085F74u, 1)
CALLEE(func_80085084, 0x80085084u, 1)
int pe_plat_ctl_counter_setup(unsigned short counter, short target, int flags)
{ return (int)cdx_callee(0x80085814u, 3, counter, (uint16_t)target, (uint32_t)flags, 0, 0); }
void pe_plat_spu_voice_volume(int voice, uint16_t left, uint16_t right, int16_t left_mode, int16_t right_mode)
{ cdx_callee(0x800862F4u, 5, (uint32_t)voice, left, right, (uint16_t)left_mode, (uint16_t)right_mode); }
static uint32_t norm46(uint32_t fn, unsigned idx, uint32_t v)
{
    if (fn == 0x80085814u && idx < 2) return v & 0xFFFFu;
    if (fn == 0x800862F4u && idx >= 1) return v & 0xFFFFu;
    return v;
}
static uint32_t (*const want_norm_init)(uint32_t, unsigned, uint32_t) = norm46;
/* host seams the retail entry has no 1:1 counterpart for: a call fails the case (reported by name) */
TRAP(PE_Event_ConsumeSpuDma)
TRAP(PE_SpuDma_Begin) TRAP(PE_SpuDma_DeliverPending) TRAP(PE_SpuDma_Service)
TRAP(PE_SpuRegister_LoadU16) TRAP(PE_SpuRegister_StoreU16)
/* symbols the provider object and the spu_a object leave undefined and the entry does not reach */
TRAP(pe_plat_bios_enter_critical) TRAP(pe_plat_bios_exit_critical) TRAP(pe_plat_reset_callback)
TRAP(pe_plat_interrupt_callback) TRAP(pe_plat_bios_close_event) TRAP(pe_plat_bios_disable_event) TRAP(pe_plat_dma_callback) TRAP(PE_Printf_Guest)
void PE_Decomp_GuestTrapDivZero(const char *fn) { (void)fn; bad_call = 1; trap_name = "PE_Decomp_GuestTrapDivZero"; }
void pe_plat_sio_poll_tick(void) {}
/* host-only seam of the installer (not reached: the installer is a stand-in here) */
uint32_t PE_SpuDma_InstallIrq(uint32_t h) { (void)h; host_only_calls++; return 1; }
/* Host-only seams the current native reaches and the retail has no 1:1 counterpart for (counted, not compared):
 * PE_SpuDma_WaitForCompletion = the host adaptation of the retail wait (85174, which polls guest RAM flags);
 * PE_Event_SpuDmaEnabled: undocumented gate (pe_sdk.h declares it without a comment); the stand-in returns 1
 * (the SPU DMA event was just registered and enabled) unless CDX46_SPUDMA_ENABLED=0 is set (the native then skips
 * the reverb calls: reported in the REPORT). */
uint32_t PE_Event_SpuDmaEnabled(void)
{
    const char *e = getenv("CDX46_SPUDMA_ENABLED");
    host_only_calls++;
    return e && e[0] == '0' ? 0u : 1u;
}
void PE_SpuDma_WaitForCompletion(void) { host_only_calls++; }
int PE_Port_ShouldStop(void) { return 0; }
void PE_Port_RequestStop(int r) { (void)r; bad_call = 1; trap_name = "PE_Port_RequestStop"; }
void pe_plat_cdxspu_stream_bringup(void);
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    (void)a; (void)ret;
    want_norm = want_norm_init;
    if (mutant == 1) return entry == 0x80085644u ? 0 : -1;
    if (entry != 0x80085644u) return -1;
    pe_plat_cdxspu_stream_bringup();
    return 0;
}
