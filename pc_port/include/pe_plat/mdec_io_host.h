#ifndef PE_PLAT_MDEC_IO_HOST_H
#define PE_PLAT_MDEC_IO_HOST_H
#include <stdint.h>
/* Guest rows 0x8010BFA0 (DecDCTin wrapper) and 0x8010C01C (DecDCTout wrapper): thin adapters that apply the ordered
 * register writes of the clean-room pe_plat/mdec_io.h (fresh author, psx-spx + black-box vectors) through the port's
 * host MDEC/DMA model (platform/pe_plat_mdec_io_host.c). */
void pe_plat_mdec_input_host(uint32_t command_block, uint32_t mode);
void pe_plat_mdec_output_host(uint32_t destination, uint32_t words);
#define func_8010BFA0 pe_plat_mdec_input_host
#define func_8010C01C pe_plat_mdec_output_host
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
void func_8010BFA0(uint32_t command_block, uint32_t mode);
void func_8010C01C(uint32_t destination, uint32_t words);
#endif
