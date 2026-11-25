#ifndef PE_PLAT_MDEC_CALLBACK_H
#define PE_PLAT_MDEC_CALLBACK_H
#include <stdint.h>
uint32_t pe_plat_mdec_out_callback_host(uint32_t callback);
#define func_8010C0D8 pe_plat_mdec_out_callback_host
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
uint32_t func_8010C0D8(uint32_t callback);
#endif
