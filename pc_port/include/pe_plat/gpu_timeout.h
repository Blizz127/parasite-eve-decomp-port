/* GPU wait timeout check (guest 0x80077404, "gpu3" in the classification);
 * host model in platform/pe_plat_gpu_timeout.c. */
#ifndef PE_PLAT_GPU_TIMEOUT_H
#define PE_PLAT_GPU_TIMEOUT_H
#include <stdint.h>
int32_t pe_plat_gpu_timeout(void);
#define func_80077404 pe_plat_gpu_timeout
/* Canonical guest prototype (expands to the pe_plat declaration above). */
int32_t func_80077404(void);
#endif
