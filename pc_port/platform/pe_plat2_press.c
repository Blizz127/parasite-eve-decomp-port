/* libpress host bindings (guest 0x8010C0D8), written clean-room from the
 * black-box vector sets gpu2d (docs/evidence/native-gpu2-20261126), the port's
 * host GPU/DMA model API and psx-spx; no SDK source, decompiled source, tables
 * or disassembly were read.
 *
 * DecDCToutCallback (guest 0x8010C0D8): the vectors (300 cases) show exactly one
 * observable action and no guest RAM access: a call of the libetc DMA
 * callback registration (guest 0x80073CF4, pe_plat_dma_callback) with channel
 * 1 (the MDEC-out DMA channel, psx-spx "DMA Channels") and the caller's
 * handler word, whose result is returned unchanged.  The other argument
 * registers do not matter.  A zero handler is passed through like any other
 * word (vector cases 202211333 and 202211334 call with 0 and 0x80000000). */
#include "pe_plat/press.h"
#include "pe_plat/irq_callback.h"

#define PRESS_DMA_CHANNEL_MDEC_OUT 1u

uint32_t pe_plat_press_out_callback(uint32_t callback)
{
    return pe_plat_dma_callback(PRESS_DMA_CHANNEL_MDEC_OUT, callback);
}
