/* DrawSync (guest 0x80074DC0): debug trace, then the GPU jump table's wait
 * slot; host model in platform/pe_plat_gpu_drawsync.c. */
#ifndef PE_PLAT_GPU_DRAWSYNC_H
#define PE_PLAT_GPU_DRAWSYNC_H
#include <stdint.h>
int pe_plat_gpu_drawsync(int32_t mode);
#define func_80074DC0 pe_plat_gpu_drawsync
/* Canonical guest prototype (expands to the pe_plat declaration above). */
int func_80074DC0(int32_t mode);
#endif
