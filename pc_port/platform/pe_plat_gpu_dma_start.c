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
 * the port returns the same value.
 *
 * Host hooks (not in the vectors; pe_plat/gpu_host.h): a list-node preflight before
 * the first register write and, after the CHCR write arms the channel in the host
 * GPU model, the list worker that runs it to completion node by node. */
#include "pe_guest_ram.h"
#include "pe_plat/gpu_dma_start.h"
#include "pe_plat/gpu_regs.h"
#include "pe_plat/gpu_host.h"

int pe_plat_gpu_dma_start(uint32_t packet, uint32_t auxiliary)
{
    (void)auxiliary;
    /* Host: the first list node must be whole GP0 packets, else a named cut before any register write. */
    if (!pe_plat_gpu_host_dma_preflight(packet)) return 0;
    uint32_t gpu = pe_gpu_reg_word(0x80095854u, PE_GPU_REG_GPU1, 4);
    if (!pe_gpu_reg_ok(gpu, 4)) return pe_gpu_reg_refuse("gpu_dma_start", "func_80076B98", gpu);
    PE_StoreU32(gpu, 0x04000002u);
    uint32_t madr = pe_gpu_reg_word(0x80095858u, PE_GPU_REG_MADR2, 4);
    if (!pe_gpu_reg_ok(madr, 4)) return pe_gpu_reg_refuse("gpu_dma_start", "func_80076B98", madr);
    PE_StoreU32(madr, packet);
    uint32_t bcr = pe_gpu_reg_word(0x8009585Cu, PE_GPU_REG_BCR2, 4);
    if (!pe_gpu_reg_ok(bcr, 4)) return pe_gpu_reg_refuse("gpu_dma_start", "func_80076B98", bcr);
    PE_StoreU32(bcr, 0u);
    uint32_t chcr = pe_gpu_reg_word(0x80095860u, PE_GPU_REG_CHCR2, 4);
    if (!pe_gpu_reg_ok(chcr, 4)) return pe_gpu_reg_refuse("gpu_dma_start", "func_80076B98", chcr);
    PE_StoreU32(chcr, 0x01000401u);
    /* Host: the DMA2 channel is now armed in the GPU model; run its list worker (see gpu_host.h). */
    pe_plat_gpu_host_dma_run();
    return (int)chcr;
}
