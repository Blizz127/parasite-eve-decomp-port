/* Start a GPU linked-list DMA (guest 0x80076B98), written clean-room from
 * black-box vectors of the original code (pc_port/tools/pe_gpu_rows_vectors.py
 * `dmastart`, checked by pe_gpu_rows_check.c) plus the hardware documentation
 * (psx-spx GPU: GP1(04h) DMA direction; DMA: MADR / BCR / CHCR, channel 2,
 * "SyncMode 2 = linked list", start bit 24).  No SDK source, matched C or
 * disassembly was read; see docs/evidence/native-gpu-rows-20261125/.
 *
 * Contract (from vectors): through the register pointer words
 * 0x80095854 (GPU), 0x80095858 (DMA2 MADR), 0x8009585C (BCR) and 0x80095860
 * (CHCR), in this order: GP1(04h) = 2 (DMA direction CPU->GPU), MADR = the
 * packet address (all 32 bits), BCR = 0, CHCR = 0x01000401 (start, linked
 * list, from RAM).  The second argument is not used.  The original leaves the
 * CHCR register address in the result register; callers do not use it, and
 * the port returns the same value. */
#include "pe_guest_ram.h"
#include "pe_plat/gpu_dma_start.h"
#include "pe_plat/gpu_regs.h"

int pe_plat_gpu_dma_start(uint32_t packet, uint32_t auxiliary)
{
    (void)auxiliary;
    uint32_t gpu = PE_LoadU32(0x80095854u);
    if (!pe_gpu_reg_ok(gpu, 4)) return pe_gpu_reg_refuse("gpu_dma_start", "func_80076B98", gpu);
    PE_StoreU32(gpu, 0x04000002u);
    uint32_t madr = PE_LoadU32(0x80095858u);
    if (!pe_gpu_reg_ok(madr, 4)) return pe_gpu_reg_refuse("gpu_dma_start", "func_80076B98", madr);
    PE_StoreU32(madr, packet);
    uint32_t bcr = PE_LoadU32(0x8009585Cu);
    if (!pe_gpu_reg_ok(bcr, 4)) return pe_gpu_reg_refuse("gpu_dma_start", "func_80076B98", bcr);
    PE_StoreU32(bcr, 0u);
    uint32_t chcr = PE_LoadU32(0x80095860u);
    if (!pe_gpu_reg_ok(chcr, 4)) return pe_gpu_reg_refuse("gpu_dma_start", "func_80076B98", chcr);
    PE_StoreU32(chcr, 0x01000401u);
    return (int)chcr;
}
