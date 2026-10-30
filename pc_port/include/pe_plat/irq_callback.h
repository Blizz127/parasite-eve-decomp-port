/* ResetCallback / InterruptCallback (libetc) -> clean-room host models in
 * platform/pe_plat_irq_callback.c. */
#ifndef PE_PLAT_IRQ_CALLBACK_H
#define PE_PLAT_IRQ_CALLBACK_H
#include <stdint.h>
uint32_t pe_plat_reset_callback(void);
uint32_t pe_plat_interrupt_callback(uint32_t source, uint32_t handler);
#define func_80073C94 pe_plat_reset_callback
#define func_80073CC4 pe_plat_interrupt_callback
#endif
