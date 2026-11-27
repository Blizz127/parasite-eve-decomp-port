/* Host seams behind the clean-room libetc veneers (family cdx-etc, platform/pe_plat2_cdx_etc_seams.c).
 *
 * Host contracts written clean-room from the port's consumers and tests; no original or translated code
 * read.  The guest-visible results of the libetc veneers come from platform/pe_plat2_cdx_etc.c alone; the five
 * raw seams below only keep the host bookkeeping other host code gates on, and their results are ignored:
 *  - critical-section nesting depth (PE_Irq_LockDepth: the deferred-IRQ delivery gate and the audio-timer
 *    gate consult it; Enter increments it, Exit decrements it, never below zero);
 *  - the source-0 BIOS diagnostics snapshot (PE_Irq_GetSource0BiosState, pe_sdk.h), recorded when the
 *    ChangeClearPAD / ChangeClearRCnt veneers run (counter 3 = VBlank root counter);
 *  - FlushCache: no host state (no instruction cache); the seam exists so link-time wraps keep a hook.
 * The raw names are kept (not renamed) because test_transition_loader.c wraps three of them (-Wl,--wrap).
 * The BIOS event table (PE_Event_*), PE_Irq_LockDepth, PE_Irq_GetSource0BiosState and the test/host reset
 * owner PE_Sdk_ResetState are declared in pe_sdk.h and defined in pe_plat2_cdx_etc_seams.c.
 * Include after defining PE_CDXETC_HOST_SEAMS if pe_plat/bios_sys.h is also included, otherwise the names
 * are macros for the pe_plat_cdxetc_* veneers (see bios_sys.h). */
#ifndef PE_PLAT_CDX_ETC_SEAMS_H
#define PE_PLAT_CDX_ETC_SEAMS_H
#include <stdint.h>

void pe_plat_bios_flush_cache(void);
void pe_plat_bios_change_clear_pad(uint32_t flag);          /* flag: the full word the caller passed */
uint32_t pe_plat_bios_change_clear_rcnt(uint32_t counter, uint32_t flag);   /* counter 0..3; returns 0 (ignored) */
int pe_plat_bios_enter_critical(void);                      /* returns 0 (ignored) */
void pe_plat_bios_exit_critical(void);
#endif
