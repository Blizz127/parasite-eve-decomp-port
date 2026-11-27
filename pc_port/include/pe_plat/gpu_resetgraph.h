/* ResetGraph (guest 0x80074A44): reset modes (low three bits 0, 3, 5) in
 * platform/pe_plat_gpu_resetgraph.c, every other mode through the light path
 * (platform/pe_plat2_resetgraph_light.c). */
#ifndef PE_PLAT_GPU_RESETGRAPH_H
#define PE_PLAT_GPU_RESETGRAPH_H
#include <stdint.h>
int pe_plat_gpu_resetgraph(int mode);
/* GPU command-word writer (guest 0x80077A54, BIOS A(49h) GPU_cw): sends one word to GP0.  Host side in
 * platform/pe_plat_gpu_host.c; the vector checker stubs it as a named callee. */
void pe_plat_gpu_cw(uint32_t word);
#define func_80074A44 pe_plat_gpu_resetgraph
/* Canonical guest prototype (expands to the pe_plat declaration above). */
int func_80074A44(int mode);
#endif
