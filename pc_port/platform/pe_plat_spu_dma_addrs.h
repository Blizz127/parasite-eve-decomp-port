/* Guest code addresses the host SPU-DMA model recognises; internal, not part of
 * the public API.  Included only by host/test sources that need to name them. */
#ifndef PE_PLAT_SPU_DMA_ADDRS_H
#define PE_PLAT_SPU_DMA_ADDRS_H

#define PE_SPU_DMA_IRQ_HANDLER 0x8007D614u
#define PE_SPU_DMA_CALLBACK    0x80085098u

#endif /* PE_PLAT_SPU_DMA_ADDRS_H */
