/* Smoke-only layer (-DG2_SMOKE): the PE_GPU_* host-model calls the CURRENT providers make instead of touching the GPU
 * registers.  A clean-room provider addresses the hardware through PE_LoadU32/PE_StoreU32 and does not need this
 * file.  Prototypes are the ones declared in pc_port/platform/pe_gpu.h (prototype lines only; no provider source is
 * read).  Semantics are inferred from the names:
 *   WriteGP0/WriteGP1 = store to 0x1F801810/14, ReadStatus = load 0x1F801814, ReadGP0 = load 0x1F801810,
 *   ReadDMA2CHCR = load 0x1F8010A8, DMA2Issue = stores to MADR/BCR/CHCR, VSyncQuery = the guest VSync(-1) call. */
#ifndef PE_GPU2_SMOKE_HOST_H
#define PE_GPU2_SMOKE_HOST_H
uint32_t PE_GPU_ReadStatus(void) { return g2_ld(0x1F801814u, 4); }
int PE_GPU_WriteGP0(uint32_t v) { g2_st(0x1F801810u, 4, v); return 1; }
int PE_GPU_WriteGP1(uint32_t v) { g2_st(0x1F801814u, 4, v); return 1; }
int PE_GPU_ReadGP0(uint32_t *v) { *v = g2_ld(0x1F801810u, 4); return 1; }
uint32_t PE_GPU_ReadDMA2CHCR(void) { return g2_ld(0x1F8010A8u, 4); }
int PE_GPU_DMA2Issue(pe_addr_t madr, uint32_t bcr, uint32_t chcr)
{ g2_st(0x1F8010A0u, 4, madr); g2_st(0x1F8010A4u, 4, bcr); g2_st(0x1F8010A8u, 4, chcr); return 1; }
int PE_GPU_CanBeginImageLoad(int needs_dma) { (void)needs_dma; return 1; }
int PE_GPU_DMA2Pending(void) { return 0; }
uint32_t PE_GPU_ReadDPCR(void) { return g2_ld(0x1F8010F0u, 4); }
void PE_GPU_WriteDPCR(uint32_t v) { g2_st(0x1F8010F0u, 4, v); }
uint32_t PE_GPU_ReadStoredDICR(void) { return 0; }
uint32_t PE_GPU_VSyncQuery(void) { return g2_call(0x80073A44u, 0xFFFFFFFFu, 0, 0, 0); }
void PE_GPU_GetState(void *out) { (void)out; }
uint16_t PE_IRQ_GetMask(void) { return 0; }
#endif
