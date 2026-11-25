#ifndef PE_PLAT_MDEC_EXPAND_H
#define PE_PLAT_MDEC_EXPAND_H
#include <stdint.h>
uint32_t pe_plat_mdec_expand_table_host(uint32_t dst, uint32_t count);
#define func_8010BD4C pe_plat_mdec_expand_table_host
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
uint32_t func_8010BD4C(uint32_t dst, uint32_t count);
#endif
