/* Bind the BIOS rand/srand trampoline addresses to the host model in
 * platform/pe_bios_rand.c (psx-spx BIOS A(2Fh)/A(30h)). */
#ifndef PE_PLAT_BIOS_RAND_H
#define PE_PLAT_BIOS_RAND_H
#include <stdint.h>
unsigned int pe_plat_bios_rand(void);
void pe_plat_bios_srand(uint32_t seed);
#define func_80071A54 pe_plat_bios_rand
#define func_80071A64 pe_plat_bios_srand
/* Canonical guest prototypes (read by the port generators; they expand to
 * the pe_plat declarations above). */
unsigned int func_80071A54(void);
void func_80071A64(uint32_t seed);
#endif
