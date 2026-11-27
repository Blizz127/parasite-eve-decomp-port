/* Vector check for batch cdgen46 (pe_cdx_vectors.py cdgen46): the stream (SPU) bring-up func_80085644 of
 * platform/pe_stream.c.  Black-box: the provider is linked as a built object (never read).  The retail callees that
 * the current object reaches through a bound host symbol (names from `nm -u` + the public pe_plat headers) are
 * recorded stand-ins; the retail libspu helpers with no host symbol ran inline in the vectors (their SPU register
 * accesses and RAM writes are part of the entry).  Scaffold: pe_cdx_common.h.
 * Argument widths: the public prototypes of pe_plat_ctl_counter_setup (unsigned short, short, int) and
 * pe_plat_spu_voice_volume (int, unsigned short, unsigned short, short, short) take 16-bit values where retail
 * passes full registers; want_norm compares them at the prototype's width (the retail 0xF2000002 counter id is
 * compared as 0x0002).
 * Build (objects: ninja -C build/vec CMakeFiles/pe_field_runtime.dir/platform/pe_stream.c.o):
 * BUILD: cc -std=gnu11 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen46_check.c build/vec/CMakeFiles/pe_field_runtime.dir/platform/pe_stream.c.o -o check46
 * Run: ./check46 vectors.txt [noop|flip]
 */
#include "pe_cdx_common.h"
CALLEE(PE_Event_Open, 0x800726E4u, 4)
CALLEE(PE_Event_Enable, 0x80072704u, 1)
CALLEE(pe_plat_spu_install_dma_event, 0x8007D15Cu, 0)
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
void func_80085644(void);
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    (void)a; (void)ret;
    want_norm = want_norm_init;
    if (mutant == 1) return entry == 0x80085644u ? 0 : -1;
    if (entry != 0x80085644u) return -1;
    func_80085644();
    return 0;
}
