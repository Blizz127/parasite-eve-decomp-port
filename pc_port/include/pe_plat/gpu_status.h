/* GPU status getters (psx-spx GPUSTAT); host model in platform/pe_plat_gpu_status.c. */
#ifndef PE_PLAT_GPU_STATUS_H
#define PE_PLAT_GPU_STATUS_H
unsigned int pe_plat_gpu_get_ode(void);
#define func_80075B1C pe_plat_gpu_get_ode
/* Canonical guest prototypes (read by the port generators; they expand to
 * the pe_plat declarations above). */
unsigned int func_80075B1C(void);
#endif
