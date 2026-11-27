/* State60 vector checker, family spu (batch state60a): SPU register setters / heap / DMA-callback leaves (pe_plat_state.c).
 * Build (run from the repository root; provider object is linked as a black box):
 *   cc -std=c99 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -O1 -fno-stack-protector -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap -Ipc_port/tools pc_port/tools/pe_state60_spu_check.c build/state60/prov/pe_plat_state.c.o -Wl,--unresolved-symbols=ignore-all -o build/state60/check_spu
 * Run:  ./build/state60/check_spu <vector files of state60a>
 * Provider object under test: build/state60/prov/pe_plat_state.c.o (copied from the int3 build tree, hash in REPORT.md).
 * Replay rules are in pe_state60_check_core.h; vectors come from pc_port/tools/pe_state60_vectors.py state60a. */
#define _POSIX_C_SOURCE 200809L
#include "pe_state60_check_core.h"
#include "pe_plat/state.h"
#include "pe_plat/spu_dma_hw.h"

/* Host / callee recorders required by pe_plat_state.c.o.  Callee targets follow the include/pe_plat headers
 * (#define maps: func_80072714 -> pe_plat_bios_enter_critical, the BIOS close-event/disable-event bindings -> pe_plat_bios_close_event/_disable_event, ...). */
int pe_plat_bios_enter_critical(void) { return (int)state60_note_call(0x80072714u, 0, 0, 0, 0); }
void pe_plat_bios_exit_critical(void) { state60_note_call(0x80072724u, 0, 0, 0, 0); }
int pe_plat_bios_close_event(int h) { return (int)state60_note_call(0x800857E4u, (uint32_t)h, 0, 0, 0); }
int pe_plat_bios_disable_event(int h) { return (int)state60_note_call(0x800857F4u, (uint32_t)h, 0, 0, 0); }
uint32_t pe_plat_dma_callback(uint32_t ch, uint32_t h) { return state60_note_call(0x80073CF4u, ch, h, 0, 0); }
uint32_t pe_plat_interrupt_callback(uint32_t s, uint32_t h) { return state60_note_call(0x80073CC4u, s, h, 0, 0); }
int32_t PE_Printf_Guest(pe_addr_t fmt, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6, uint32_t a7)
{ (void)a4; (void)a5; (void)a6; (void)a7; return (int32_t)state60_note_call(0x80071A74u, fmt, a1, a2, a3); }
static unsigned family_arity(uint32_t fn)
{
    switch (fn) {
    case 0x80072714u: case 0x80072724u: return 0;
    case 0x800857E4u: case 0x800857F4u: return 1;
    case 0x80073CF4u: case 0x80073CC4u: return 2;
    case 0x80071A74u: return 1;      /* format pointer only: the printf varargs a1..a3 are not all consumed */
    }
    return 0;
}
/* Host services the provider reaches that the original never called: any use is reported as an
 * unexpected host call (the case then fails, with the reason under STATE60_VERBOSE=1). */
void PE_Decomp_GuestTrapDivZero(const char *fn) { (void)fn; host_calls_unexpected++; }
void PE_Decomp_NoteStopDetail(const char *t) { (void)t; host_calls_unexpected++; }
void PE_Port_RequestStop(int reason) { (void)reason; host_calls_unexpected++; }
int PE_Port_ShouldStop(void) { return 0; }
bool PE_RangeIsRam(pe_addr_t a, size_t n) { return (a & 0x1FFFFFFFu) < MEM && (a & 0x1FFFFFFFu) + n <= MEM; }
pe_addr_t PE_RamCanonical(pe_addr_t a, size_t n) { (void)n; return 0x80000000u | (a & (MEM - 1)); }
int PE_SpuDma_Begin(pe_addr_t a, uint32_t b, uint32_t c, pe_addr_t d) { (void)a; (void)b; (void)c; (void)d; host_calls_unexpected++; return 0; }
int PE_SpuDma_BeginRead(pe_addr_t a, uint32_t b, uint32_t c, pe_addr_t d) { (void)a; (void)b; (void)c; (void)d; host_calls_unexpected++; return 0; }
void PE_SpuDma_GetState(PeSpuDmaState *out) { host_calls_unexpected++; memset(out, 0, sizeof *out); }
uint32_t PE_Spu_TransferCursor(void) { host_calls_unexpected++; return 0; }
uint16_t PE_SpuRegister_LoadU16(uint32_t off)
{ uint32_t v; if (mmio_read(0x1F801C00u + off, 2, &v)) return (uint16_t)v; return 0; }
void __stack_chk_fail(void) { abort(); }
static int family_call(uint32_t entry, const uint32_t *a, uint32_t *ret, int *has_ret, unsigned *rw)
{
    *rw = 4; *has_ret = 0;
    switch (entry) {
    case 0x8007DCACu: pe_plat_spu_wait_cycles(); return 1;
    case 0x8007D1D4u: *ret = (uint32_t)pe_plat_spu_initialize((int)a[0]); *has_ret = 1; return 1;
    case 0x8007D454u: pe_plat_spu_write_io(a[0], a[1]); return 1;
    case 0x80085984u: pe_plat_spu_shutdown(); return 1;
    case 0x80085F74u: pe_plat_spu_set_common_attributes(a[0]); return 1;
    case 0x800862F4u: pe_plat_spu_voice_volume((int)a[0], (uint16_t)a[1], (uint16_t)a[2], (int16_t)a[3], (int16_t)a[4]); return 1;
    case 0x80085C44u: *ret = (uint32_t)pe_plat_spu_set_control_bit((int)a[0]); *has_ret = 1; return 1;
    case 0x80085A64u: *ret = (uint32_t)pe_plat_spu_set_reverb_enabled((int)a[0]); *has_ret = 1; return 1;
    case 0x80085EB4u: *ret = pe_plat_spu_set_transfer_address(a[0]); *has_ret = 1; return 1;
    case 0x80085B34u: *ret = (uint32_t)pe_plat_spu_heap_contains_raw(a[0]); *has_ret = 1; return 1;
    case 0x80085BB4u: *ret = (uint32_t)pe_plat_spu_heap_contains(a[0]); *has_ret = 1; return 1;
    case 0x80085A04u: *ret = (uint32_t)pe_plat_spu_configure_heap((int)a[0], a[1]); *has_ret = 1; return 1;
    case 0x80089EB8u: *ret = (uint32_t)pe_plat_spu_set_control_level((int)a[0]); *has_ret = 1; return 1;
    case 0x8007DD14u: pe_plat_spu_register_dma_callback(a[0]); return 1;
    case 0x80089F28u: pe_plat_spu_set_pair((uint16_t)a[0], (uint16_t)a[1]); return 1;
    case 0x80085D84u: *ret = (uint32_t)pe_plat_spu_set_irq_callback((int)a[0]); *has_ret = 1; return 1;
    case 0x80085DC4u: pe_plat_spu_register_irq((int)a[0]); return 1;
    case 0x80085F14u: *ret = (uint32_t)pe_plat_spu_set_transfer_mode((int)a[0]); *has_ret = 1; return 1;
    case 0x80089F08u: pe_plat_spu_copy_voice_field(a[0], a[1]); return 1;
    case 0x8007DC04u: pe_plat_spu_set_dma_control_priority((int)a[0]); return 1;
    case 0x8007DAE0u: pe_plat_spu_store_value((int)a[0], a[1], (int)a[2]); return 1;
    case 0x8007DB24u: *ret = (uint32_t)pe_plat_spu_set_address_units((int)a[0], a[1]); *has_ret = 1; return 1;
    case 0x8007DC84u: pe_plat_spu_dma_priority_enabled(); return 1;
    case 0x8007DBC8u: *ret = (uint32_t)pe_plat_spu_address_value(a[0], (int)a[1]); *has_ret = 1; return 1;
    case 0x8007DC5Cu: pe_plat_spu_dma_priority(); return 1;
    case 0x80085F44u: pe_plat_spu_set_callback((int)a[0]); return 1;
    }
    return -1;
}
