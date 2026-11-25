/* GPU command/DMA queue pump and wait (guest 0x80077294, "text_2850" /
 * "dma_queue" in the classification); host model in
 * platform/pe_plat_gpu_dma_queue.c. */
#ifndef PE_PLAT_GPU_DMA_QUEUE_H
#define PE_PLAT_GPU_DMA_QUEUE_H
#include <stdint.h>
int32_t pe_plat_gpu_dma_queue_run(int32_t mode);
#define func_80077294 pe_plat_gpu_dma_queue_run
/* Canonical guest prototype (expands to the pe_plat declaration above). */
int32_t func_80077294(int32_t mode);
#endif
