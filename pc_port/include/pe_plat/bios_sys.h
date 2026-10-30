/* Bind four BIOS trampoline addresses to existing host models (psx-spx
 * "Kernel (BIOS)"): B(38h) at 0x800719E4 (platform/func_800719E4_port.c),
 * A(44h) FlushCache, B(5Bh) ChangeClearPAD and C(0Ah) ChangeClearRCnt
 * (platform/pe_libetc.c), and the SYSCALL(01h)/(02h) Enter/ExitCriticalSection
 * veneers at 0x80072714/0x80072724 (platform/pe_libetc.c).  Also the
 * libetc host models in pe_libetc.c: SetIntrMask (I_MASK exchange through the
 * PE_IRQ authority, psx-spx 1F801074h) and the VBlank callback-slot setters
 * VSyncCallback/VSyncCallbacks (PE_Callback slot table). */
#ifndef PE_PLAT_BIOS_SYS_H
#define PE_PLAT_BIOS_SYS_H
#include <stdint.h>
uint32_t pe_plat_bios_exit(uint32_t mode);
void pe_plat_bios_flush_cache(void);
void pe_plat_bios_change_clear_pad(uint32_t flag);
uint32_t pe_plat_bios_change_clear_rcnt(uint32_t counter, uint32_t flag);
int pe_plat_bios_enter_critical(void);
void pe_plat_bios_exit_critical(void);
uint16_t pe_plat_irq_set_mask(uint16_t new_mask);
uint32_t pe_plat_vsync_callback(uint32_t handler);
uint32_t pe_plat_vsync_callbacks(uint32_t slot, uint32_t handler);
#define func_800719E4 pe_plat_bios_exit
#define func_800726C4 pe_plat_bios_flush_cache
#define func_80073C74 pe_plat_bios_change_clear_pad
#define func_80073C84 pe_plat_bios_change_clear_rcnt
#define func_80072714 pe_plat_bios_enter_critical
#define func_80072724 pe_plat_bios_exit_critical
#define func_80073E10 pe_plat_irq_set_mask
#define func_80073D24 pe_plat_vsync_callback
#define func_80073D58 pe_plat_vsync_callbacks
#endif
