/* GPU status getters written from the hardware documentation, not from SDK
 * source.  psx-spx "GPU Status Register" (GP1 read, 1F801814h):
 *   "31 Drawing even/odd lines in interlace mode (0=Even or Vblank, 1=Odd)".
 * The guest address is bound in pe_plat/gpu_status.h. */
#include "pe_plat/gpu_status.h"
#include "pe_gpu.h"

unsigned int pe_plat_gpu_get_ode(void)
{
    return PE_GPU_ReadStatus() >> 31;
}
