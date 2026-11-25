/* PutDispEnv (guest 0x800755F0): GP1 display commands from a DISPENV; host
 * model in platform/pe_plat_gpu_putdispenv.c. */
#ifndef PE_PLAT_GPU_PUTDISPENV_H
#define PE_PLAT_GPU_PUTDISPENV_H
#include <stdint.h>
uint32_t pe_plat_gpu_put_disp_env(uint32_t env);
#define func_800755F0 pe_plat_gpu_put_disp_env
/* Canonical guest prototype (expands to the pe_plat declaration above). */
uint32_t func_800755F0(uint32_t env);
#endif
