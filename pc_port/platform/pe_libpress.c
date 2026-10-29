/*
 * Host replacements for three Psy-Q libpress entries in the MDEC module
 * overlay ovl_039F (PE.IMG [0x39F,0x3C5) -> 0x8010BCF8).  psyq_port_guard
 * forbids defining func_<psyq address>, so lowered overlay code calls these
 * PE_ functions by name (pe_port_compat.h `host-replacement:` markers).  Their
 * only caller is the credits overlay ovl_0457 (func_8012403C, func_80125094),
 * which decodes its background pictures with DecDCTvlc / DecDCTin / DecDCTout.
 *
 * Each body follows the retail bytes:
 *   func_8010BF94  lhu v0,0(a0)                      (src/overlays/ovl_039F/func_8010BF94.c)
 *   func_8010C86C  t0 = &D_8011EB8C; v0 = *t0;
 *                  addi at,a0,-1; blez at -> *t0 = 0xFFFFFF, else *t0 = a0 << 1
 *   func_8010C078  a0 != 0: (func_8010C430() >> 24) & 1, func_8010C430 reading
 *                  *D_8010DB50 = 0x1F801824 (MDEC1 status); a0 == 0: func_8010C39C,
 *                  0x100000 polls of *D_8010DB30 = 0x1F801098 (DMA1 CHCR) bit 24,
 *                  then func_8010C448 and -1
 *                  (src/overlays/ovl_039F/func_8010C078.c, func_8010C430.c,
 *                  func_8010C39C.c, func_8010C448.c).
 */
#include "pe_port_compat.h"
#include "pe_guest_ram.h"
#include "pe_mdec.h"

#include <stdio.h>

#define GA_VLC_BOUND 0x8011EB8Cu    /* D_8011EB8C: func_8010C89C's output bound */

uint32_t PE_DecDCTBufSize_Guest(pe_addr_t bs)
{
    return PE_LoadU16(bs);
}

/* `addi at,a0,-1` traps on signed overflow for a0 == 0x80000000; the only
 * callers pass a halfword size shifted right, so that input never occurs. */
int32_t PE_DecDCTvlcSize_Guest(uint32_t size)
{
    uint32_t previous = PE_LoadU32(GA_VLC_BOUND);
    PE_StoreU32(GA_VLC_BOUND, (int32_t)(size - 1u) > 0 ? size << 1 : 0x00FFFFFFu);
    return (int32_t)previous;
}

int32_t PE_DecDCToutSync_Guest(int32_t mode)
{
    PeMdecState st;
    if (mode) {
        /* MDEC1 status bit 24 is "data output signed", the current command's
         * bit 26 (CMD.26); the status bits 26-23 hold CMD.28-25 until the
         * next command or a reset (which reads back 0x80040000).  Retail
         * returns that bit, not a busy flag, and so does this. */
        PE_MDEC_GetState(&st);
        return (int32_t)((st.command_last_write >> 26) & 1u);
    }
    /* func_8010C39C polls DMA1 CHCR bit 24 while the hardware runs.  The
     * host model completes DMA1 inside PE_MDEC_Service when it can; when it
     * cannot, nothing else advances it during the poll, which is retail's
     * timeout. */
    PE_MDEC_GetState(&st);
    if (st.dma1_chcr & 0x01000000u) {
        (void)PE_MDEC_Service();
        PE_MDEC_GetState(&st);
    }
    if (!(st.dma1_chcr & 0x01000000u))
        return 0;
    /* func_8010C448: print the timeout, MDEC1 = 0x80000000 (reset), DMA0 and
     * DMA1 CHCR = 0, MDEC1 = 0x60000000 (enable data in/out requests). */
    fprintf(stderr, "[MDEC] DecDCToutSync timeout\n");
    PE_MDEC_BeginReset();
    PE_MDEC_ClearDmaChannels();
    PE_MDEC_WriteControl(0x60000000u);
    return -1;
}
