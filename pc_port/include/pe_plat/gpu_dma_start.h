/* Start a GPU linked-list DMA (guest 0x80076B98, "text_2154" in the
 * classification); host model in platform/pe_plat_gpu_dma_start.c. */
#ifndef PE_PLAT_GPU_DMA_START_H
#define PE_PLAT_GPU_DMA_START_H
#include <stdint.h>
int pe_plat_gpu_dma_start(uint32_t packet, uint32_t auxiliary);
#define func_80076B98 pe_plat_gpu_dma_start
/* Canonical guest prototype (expands to the pe_plat declaration above). */
int func_80076B98(uint32_t packet, uint32_t auxiliary);
#endif
