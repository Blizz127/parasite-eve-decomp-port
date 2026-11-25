/* Bind BIOS string/memory entry addresses to the existing host models.
 * Contracts and provenance remain in platform/pe_bios_string.h. */
#ifndef PE_PLAT_BIOS_STRING_H
#define PE_PLAT_BIOS_STRING_H
#include <stdint.h>
#ifndef PE_PLAT_GUEST_ADDR_DEFINED
#define PE_PLAT_GUEST_ADDR_DEFINED
typedef uint32_t pe_plat_guest_addr;
#endif
int32_t pe_plat_bios_strcmp(pe_plat_guest_addr a, pe_plat_guest_addr b);
int32_t pe_plat_bios_strncmp(pe_plat_guest_addr a, pe_plat_guest_addr b, int32_t n);
pe_plat_guest_addr pe_plat_bios_strcpy(pe_plat_guest_addr dst, pe_plat_guest_addr src);
int32_t pe_plat_bios_strlen(pe_plat_guest_addr src);
pe_plat_guest_addr pe_plat_bios_memcpy(pe_plat_guest_addr dst, pe_plat_guest_addr src, int32_t n);
pe_plat_guest_addr pe_plat_bios_memset(pe_plat_guest_addr dst, int32_t value, int32_t n);
pe_plat_guest_addr pe_plat_bios_bzero(pe_plat_guest_addr dst, uint32_t n);
#define func_80072A54 pe_plat_bios_strcmp
#define func_80071A04 pe_plat_bios_strncmp
#define func_80071A14 pe_plat_bios_strcpy
#define func_80072314 pe_plat_bios_strlen
#define func_80071A34 pe_plat_bios_memcpy
#define func_80071A44 pe_plat_bios_memset
#define func_80071A24 pe_plat_bios_bzero
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
int32_t func_80072A54(uint32_t a, uint32_t b);
int32_t func_80071A04(uint32_t a, uint32_t b, int32_t n);
uint32_t func_80071A14(uint32_t dst, uint32_t src);
int32_t func_80072314(uint32_t src);
uint32_t func_80071A34(uint32_t dst, uint32_t src, int32_t n);
uint32_t func_80071A44(uint32_t dst, int32_t value, int32_t n);
uint32_t func_80071A24(uint32_t dst, uint32_t n);
#endif
