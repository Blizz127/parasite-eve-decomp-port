/* Bind the libetc veneers and VSync-family leaves of batch cdgen43 (family cdx-etc; the nine guest
 * functions below) to the clean-room host implementations in platform/pe_plat2_cdx_etc.c.
 * Written clean-room from the black-box vector sets cdgen43 (docs/evidence/native-cdx-20261126),
 * the recorded seam contracts and psx-spx; no SDK source, decompiled source, tables or disassembly
 * were read. */
#ifndef PE_PLAT_CDX_ETC_H
#define PE_PLAT_CDX_ETC_H
#include <stdint.h>

void pe_plat_cdxetc_flush_cache(void);
void pe_plat_cdxetc_change_clear_pad(uint32_t flag);
uint32_t pe_plat_cdxetc_change_clear_rcnt(uint32_t counter, uint32_t flag);
int pe_plat_cdxetc_enter_critical(void);
void pe_plat_cdxetc_exit_critical(void);
uint16_t pe_plat_cdxetc_set_intr_mask(uint16_t new_mask);
uint32_t pe_plat_cdxetc_vsync_callback(uint32_t handler);
uint32_t pe_plat_cdxetc_vsync_callbacks(uint32_t slot, uint32_t handler);
uint32_t pe_plat_cdxetc_vsync_table_write(uint32_t slot, uint32_t handler);

/* Host-side BIOS model state (psx-spx kernel: COP0 SR bits 2 and 10, root-counter clear policy,
 * pad/card clear policy, I-cache flush count).  pe_plat_cdxetc_reset_state() returns the model to its
 * power-on state (interrupts enabled, all policies 0); PE_Sdk_ResetState calls it. */
void pe_plat_cdxetc_reset_state(void);
int pe_plat_cdxetc_interrupts_enabled(void);
uint32_t pe_plat_cdxetc_rcnt_policy(uint32_t counter);
uint32_t pe_plat_cdxetc_pad_policy(void);
uint32_t pe_plat_cdxetc_flush_count(void);

#define func_800726C4 pe_plat_cdxetc_flush_cache
#define func_80073C74 pe_plat_cdxetc_change_clear_pad
#define func_80073C84 pe_plat_cdxetc_change_clear_rcnt
#define func_80072714 pe_plat_cdxetc_enter_critical
#define func_80072724 pe_plat_cdxetc_exit_critical
#define func_80073E10 pe_plat_cdxetc_set_intr_mask
#define func_80073D24 pe_plat_cdxetc_vsync_callback
#define func_80073D58 pe_plat_cdxetc_vsync_callbacks
#define func_80074478 pe_plat_cdxetc_vsync_table_write
/* Canonical guest prototypes (read by the port generators; they expand to
 * the pe_plat declarations above). */
void func_800726C4(void);
void func_80073C74(uint32_t flag);
uint32_t func_80073C84(uint32_t counter, uint32_t flag);
int func_80072714(void);
void func_80072724(void);
uint16_t func_80073E10(uint16_t new_mask);
uint32_t func_80073D24(uint32_t handler);
uint32_t func_80073D58(uint32_t slot, uint32_t handler);
uint32_t func_80074478(uint32_t slot, uint32_t handler);
#endif
