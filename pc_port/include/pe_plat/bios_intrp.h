/* Bind the two kernel interrupt-chain service trampolines (kernel C0 vector)
 * to the host chain model in platform/pe_plat_bios_intrp.c:
 *   C(02h) SysEnqIntRP(priority, struc)   guest 0x8007E1E4
 *   C(03h) SysDeqIntRP(priority, struc)   guest 0x8007E1F4
 * (the trampolines load the C0 table address and the function number 2 / 3;
 * psx-spx kernelbios, "Priority Chains", "BIOS Control Blocks").
 *
 * PROVENANCE: written from the psx-spx text and the port's own BIOS host
 * model only.  No SDK source, decompiled source, tables or disassembly were
 * read (the project's matching-decomp notes that auto-load were not used).
 *
 * Naming: the symbols use the prefix pe_plat_bios_ because
 * gen_decomp_ports.py and pe_plat_canonical_protos.py only accept
 * `#define func_X pe_plat_<word>` bindings. */
#ifndef PE_PLAT_BIOS_INTRP_H
#define PE_PLAT_BIOS_INTRP_H
#include <stdint.h>

#define PE_BIOS_INTRP_PRIORITIES 4u   /* chains 0..3 */

/* C(02h): insert the 4-word record `record` (guest address) at the begin of
 * chain `priority`.  Returns 0 (psx-spx gives no return value). */
int pe_plat_bios_sys_enq_int_rp(uint32_t priority, uint32_t record);
/* C(03h): remove `record` from chain `priority`.  Returns 0. */
int pe_plat_bios_sys_deq_int_rp(uint32_t priority, uint32_t record);

/* Host model inspection (tests, debugging): first element of a chain, 0 when
 * empty or `priority` is out of range; and a reset to four empty chains. */
uint32_t pe_plat_bios_intrp_head(uint32_t priority);
void pe_plat_bios_intrp_reset(void);

#define func_8007E1E4 pe_plat_bios_sys_enq_int_rp
#define func_8007E1F4 pe_plat_bios_sys_deq_int_rp
/* Canonical guest prototypes (read by the port generators; they expand to
 * the pe_plat declarations above). */
int func_8007E1E4(uint32_t priority, uint32_t record);
int func_8007E1F4(uint32_t priority, uint32_t record);
#endif
