/* Memory-card BIOS service entry points (kernel A0/B0 vectors):
 *   A(ABh) _card_info(chan)             guest 0x8007DD44
 *   A(ACh) _card_load(chan)             guest 0x8007DD54
 *   B(4Eh) _card_write(chan, sector, src)  guest 0x8007DDB4
 *   B(50h) _new_card()                  guest 0x8007DDC4
 * The thunks live in platform/pe_plat_bios_card.c and forward to the host
 * memory-card model (pe_memcard.c).  This header only DECLARES them; the
 * guest-name rebinding below is the BINDING (owner hold O8, see
 * docs/evidence/native-card-bios-20261125/REPORT.md): it exists only on
 * branch sw-card-bind-20261125 and is not applied on the main line.
 * The #ifndef guards let tool builds that map a guest name onto a test
 * double with -D (pc_port/tools/pe_spu_caller_check.c) keep doing so. */
#ifndef PE_PLAT_BIOS_CARD_H
#define PE_PLAT_BIOS_CARD_H
#include <stdint.h>
int pe_plat_bios_card_info(uint32_t chan);
int pe_plat_bios_card_load(uint32_t chan);
int pe_plat_bios_card_write(uint32_t chan, int sector, int src);
void pe_plat_bios_card_new_card(uint32_t chan);
#ifndef func_8007DD44
#define func_8007DD44 pe_plat_bios_card_info
#endif
#ifndef func_8007DD54
#define func_8007DD54 pe_plat_bios_card_load
#endif
#ifndef func_8007DDB4
#define func_8007DDB4 pe_plat_bios_card_write
#endif
#ifndef func_8007DDC4
#define func_8007DDC4 pe_plat_bios_card_new_card
#endif
/* Canonical guest prototypes (read by the port generators; they expand to
 * the pe_plat declarations above). */
int func_8007DD44(uint32_t chan);
int func_8007DD54(uint32_t chan);
int func_8007DDB4(uint32_t chan, int sector, int src);
void func_8007DDC4(uint32_t chan);
#endif
