/* Bind the BIOS trampoline address B(38h) at 0x800719E4 to its host model
 * (psx-spx "Kernel (BIOS)", platform/pe_plat2_bios_exit.c).  The libetc BIOS
 * veneers (A(44h) FlushCache, B(5Bh) ChangeClearPAD, C(0Ah) ChangeClearRCnt,
 * SYSCALL(01h)/(02h) Enter/ExitCriticalSection, SetIntrMask and the VBlank
 * callback-slot setters) are the clean-room cdx-etc provider
 * (platform/pe_plat2_cdx_etc.c, pe_plat/cdx_etc.h); the legacy pe_plat_* names
 * of those entries below are redirected to it.  The five pe_plat_bios_* host
 * seams are defined in platform/pe_plat2_cdx_etc_seams.c. */
#ifndef PE_PLAT_BIOS_SYS_H
#define PE_PLAT_BIOS_SYS_H
#include <stdint.h>
uint32_t pe_plat_bios_exit(uint32_t mode);
void pe_plat_bios_flush_cache(void);
void pe_plat_bios_change_clear_pad(uint32_t flag);
uint32_t pe_plat_bios_change_clear_rcnt(uint32_t counter, uint32_t flag);
int pe_plat_bios_enter_critical(void);
void pe_plat_bios_exit_critical(void);
#define func_800719E4 pe_plat_bios_exit
/* Canonical guest prototypes (read by the port generators; they expand to
 * the pe_plat declarations above). */
uint32_t func_800719E4(uint32_t mode);
/* Host callers of the five BIOS veneers (IRQ provider, pad bring-up, save manager, ...) reach the clean-room BIOS model so the guest
 * and host views of ChangeClearRCnt/Enter/ExitCriticalSection state stay one model (pe_plat2_cdx_etc.c forwards to the raw
 * host seams below, defined in pe_plat2_cdx_etc_seams.c, for the IRQ-gate/diagnostics bookkeeping).  The TUs that need the raw
 * names define PE_CDXETC_HOST_SEAMS before including this header. */
#ifndef PE_CDXETC_HOST_SEAMS
#define pe_plat_bios_flush_cache pe_plat_cdxetc_flush_cache
#define pe_plat_bios_change_clear_pad pe_plat_cdxetc_change_clear_pad
#define pe_plat_bios_change_clear_rcnt pe_plat_cdxetc_change_clear_rcnt
#define pe_plat_bios_enter_critical pe_plat_cdxetc_enter_critical
#define pe_plat_bios_exit_critical pe_plat_cdxetc_exit_critical
#endif
/* Former libetc host models, now the clean-room cdx-etc entries. */
#define pe_plat_irq_set_mask pe_plat_cdxetc_set_intr_mask
#define pe_plat_vsync_callback pe_plat_cdxetc_vsync_callback
#define pe_plat_vsync_callbacks pe_plat_cdxetc_vsync_callbacks
/* The nine libetc/VSync leaves re-derived clean-room (cdx-etc, cdgen43) bind in pe_plat/cdx_etc.h. */
#include "pe_plat/cdx_etc.h"
#endif
