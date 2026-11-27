#ifndef PE_PLAT_MDEC_CALLBACK_H
#define PE_PLAT_MDEC_CALLBACK_H
#include <stdint.h>
/* libpress MDEC reset wrapper 0x8010BE3C (pe_plat_mdec_reset.c, independent
 * re-implementation from black-box vectors: pc_port/tools/pe_mdec_reset_vectors.py). */
void pe_plat_mdec_reset(int mode);
#define func_8010BE3C pe_plat_mdec_reset
/* libpress MDEC reset core 0x8010C0FC and table-upload start 0x8010C1EC
 * (pe_plat_mdec_core.c, independent re-implementation from ordered black-box
 * traces: pc_port/tools/pe_mdec_core_vectors.py, batch cdgen40). */
void pe_plat_mdec_core(int mode);
void pe_plat_mdec_upload_start(uint32_t block, uint32_t words);
#define func_8010C0FC pe_plat_mdec_core
#define func_8010C1EC pe_plat_mdec_upload_start
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
void func_8010BE3C(int mode);
void func_8010C0FC(int mode);
void func_8010C1EC(uint32_t block, uint32_t words);
#endif
