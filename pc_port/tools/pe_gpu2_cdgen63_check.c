/* Checker for the cdgen63 set (pe_gpu2_vectors.py cdgen63): the three libpress entries hosted in pc_port/platform/pe_plat2_press_dct.c -
 * DecDCTBufSize 0x8010BF94 (PE_DecDCTBufSize_Guest), DecDCTvlcSize 0x8010C86C (PE_DecDCTvlcSize_Guest), DecDCToutSync 0x8010C078
 * (PE_DecDCToutSync_Guest).  Overlay ovl_039F code (PE.IMG [0x39F,0x3C5) -> 0x8010BCF8) is what the oracle ran; the DecDCToutSync callees
 * func_8010C39C / func_8010C430 / func_8010C448 ran INLINE in the oracle, so the register accesses they make are in the event stream.
 * Contract: pe_gpu2_check_core.h (hardware window loads / stores form one ordered stream, RAM loads only of bytes the original read,
 * the written-byte set and values equal, return value equal).  Controls: GPU2_MUTANT=1 no-op entry, =2 return bit flip, =3 store bit flip
 * (the cdx noop / retflip / storeflip controls); the run script adds the reversed-order run and the missing-provider link check.
 * Seams of the provider (nm -u): PE_MDEC_GetState, PE_MDEC_Service, PE_MDEC_BeginReset, PE_MDEC_ClearDmaChannels, PE_MDEC_WriteControl - host
 * MDEC model entries with NO retail counterpart: the original reads / writes the MDEC and DMA registers.  Two builds:
 *   strict (default): the stand-ins are host-only (counted, no register access): the provider's register-level behaviour is not modelled;
 *   mapped (-DC63_MAP=1): PE_MDEC_GetState performs ONE register read per call - the MDEC1 status word (0x1F801824) for mode != 0, the DMA1
 *       CHCR (0x1F801098) for mode == 0 - and fills command_last_write (status bit 24 moved to bit 26) / dma1_chcr from it; Service answers 0;
 *       BeginReset / ClearDmaChannels / WriteControl perform the register writes the original's timeout path makes.  The most generous
 *       mapping: only the provider's own logic (how many reads, which bit) is measured.
 * Build lines (executed verbatim by pe_gpu2_cdgen63_64_run.sh; $O = output dir with pe_plat2_press_dct.o):
 * BUILD-strict: cc -std=gnu11 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_cdgen63_check.c $O/pe_plat2_press_dct.o -o $O/c63_strict
 * BUILD-mapped: cc -std=gnu11 -Wall -Wextra -Werror -DC63_MAP=1 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_cdgen63_check.c $O/pe_plat2_press_dct.o -o $O/c63_mapped
 * Run: $O/c63_strict <vectors.txt>   (G2_VERBOSE=1 prints every mismatch)  */
#include "pe_gpu2_check_core.h"
#include "pe_guest_ram.h"
#include "pe_gpu2_host_stubs.h"
#include "pe_plat/mdec_hw.h"

uint32_t PE_DecDCTBufSize_Guest(pe_addr_t bs);
int32_t PE_DecDCTvlcSize_Guest(uint32_t size);
int32_t PE_DecDCToutSync_Guest(int32_t mode);
static uint32_t c63_mode __attribute__((unused));
#if C63_MAP
void PE_MDEC_GetState(PeMdecState *o)
{
    memset(o, 0, sizeof *o);
    if (c63_mode) o->command_last_write = ((g2_ld(0x1F801824u, 4) >> 24) & 1u) << 26;
    else o->dma1_chcr = g2_ld(0x1F801098u, 4);
}
int PE_MDEC_Service(void) { return 0; }
void PE_MDEC_BeginReset(void) { g2_st(0x1F801824u, 4, 0x80000000u); }
void PE_MDEC_ClearDmaChannels(void) { g2_st(0x1F801098u, 4, 0); }   /* only reached by the timeout path, which the vectors do not sample */
void PE_MDEC_WriteControl(uint32_t v) { g2_st(0x1F801824u, 4, v); }
#else
static unsigned c63_host_only;
void PE_MDEC_GetState(PeMdecState *o) { memset(o, 0, sizeof *o); c63_host_only++; }
int PE_MDEC_Service(void) { c63_host_only++; return 0; }
void PE_MDEC_BeginReset(void) { c63_host_only++; }
void PE_MDEC_ClearDmaChannels(void) { c63_host_only++; }
void PE_MDEC_WriteControl(uint32_t v) { (void)v; c63_host_only++; }
#endif
static unsigned g2_selected_list(uint32_t *out)
{
    out[0] = 0x8010BF94u; out[1] = 0x8010C86Cu; out[2] = 0x8010C078u;
    return 3;
}
static int g2_entry(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
#if defined(GPU2_MUTANT) && GPU2_MUTANT == 1
    { uint32_t sel[8]; unsigned i, n = g2_selected_list(sel); (void)a; (void)ret; for (i = 0; i < n; i++) if (sel[i] == entry) return 1; return -2; }
#else
    switch (entry) {
    case 0x8010BF94u: *ret = PE_DecDCTBufSize_Guest(a[0]); return 1;
    case 0x8010C86Cu: *ret = (uint32_t)PE_DecDCTvlcSize_Guest(a[0]); return 1;
    case 0x8010C078u: c63_mode = a[0]; *ret = (uint32_t)PE_DecDCToutSync_Guest((int32_t)a[0]); return 1;
    }
    return -2;
#endif
}
int main(int c, char **v) { return g2_main(c, v); }
