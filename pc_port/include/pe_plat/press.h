/* libpress (DecDCT) host bindings: clean-room provider
 * pc_port/platform/pe_plat2_press.c, written from the black-box vector sets
 * gpu2d (docs/evidence/native-gpu2-20261126) and the port's host DMA model API.
 * The binding rows for the DecDCTout completion callback registration and the
 * VLC table expander moved here from mdec_callback.h / mdec_expand.h. */
#ifndef PE_PLAT_PRESS_H
#define PE_PLAT_PRESS_H
#include <stdint.h>

uint32_t pe_plat_press_out_callback(uint32_t callback);
uint32_t pe_plat_press_expand_table(uint32_t dst, uint32_t count);

#define func_8010C0D8 pe_plat_press_out_callback
#define func_8010BD4C pe_plat_press_expand_table
/* Canonical guest-name prototype for the binding above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
uint32_t func_8010C0D8(uint32_t callback);
uint32_t func_8010BD4C(uint32_t dst, uint32_t count);
#endif
