/* ResetGraph light path (guest 0x80074A44, every mode whose low three bits are
 * not 0, 3 or 5): optional debug trace, then the GPU function table's slot 13.
 * Clean-room provider in platform/pe_plat2_resetgraph_light.c; the reset modes
 * are in platform/pe_plat_gpu_resetgraph.c (gpu_resetgraph.h), which calls this. */
#ifndef PE_PLAT_GPU_RESETGRAPH_LIGHT_H
#define PE_PLAT_GPU_RESETGRAPH_LIGHT_H
int pe_plat_resetgraph_light_host(int mode);
#endif
