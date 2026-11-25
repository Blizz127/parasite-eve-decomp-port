/* Reset/Interrupt/DMA/Stop/RestartCallback (libetc) -> clean-room host models in
 * platform/pe_plat_irq_callback.c. */
#ifndef PE_PLAT_IRQ_CALLBACK_H
#define PE_PLAT_IRQ_CALLBACK_H
#include <stdint.h>
uint32_t pe_plat_reset_callback(void);
uint32_t pe_plat_interrupt_callback(uint32_t source, uint32_t handler);
uint32_t pe_plat_dma_callback(uint32_t channel, uint32_t handler);
uint32_t pe_plat_stop_callback(void);
uint32_t pe_plat_restart_callback(void);
uint32_t pe_plat_vsync_handler(void);
uint32_t pe_plat_dma_handler(void);
uint32_t pe_plat_start_vsync(void);
uint32_t pe_plat_start_dma(void);
uint32_t pe_plat_irq_dispatch(void);
/* Host services the handlers use (pe_callback.c, pe_irq_delivery.c). */
int pe_plat_invoke_guest_callback(uint32_t fn);
uint32_t pe_plat_read_dma_madr(uint32_t channel);
#define func_80073C94 pe_plat_reset_callback
#define func_80073E28 pe_plat_reset_callback      /* same behaviour on every vector */
#define func_80073CC4 pe_plat_interrupt_callback
#define func_800740D0 pe_plat_interrupt_callback   /* same behaviour on every vector */
#define func_80073CF4 pe_plat_dma_callback
#define func_800746A0 pe_plat_dma_callback         /* same behaviour on every vector */
#define func_80073D88 pe_plat_stop_callback
#define func_80074218 pe_plat_stop_callback        /* same behaviour on every vector */
#define func_80073DB8 pe_plat_restart_callback
#define func_800742B8 pe_plat_restart_callback     /* same behaviour on every vector */
#define func_8007440C pe_plat_vsync_handler
#define func_800743B4 pe_plat_start_vsync
#define func_800744D4 pe_plat_start_dma
#define func_80074520 pe_plat_dma_handler
#define func_80073F00 pe_plat_irq_dispatch
/* Canonical guest prototypes (read by the port generators; they expand to
 * the pe_plat declarations above). */
uint32_t func_80073C94(void);
uint32_t func_80073E28(void);
uint32_t func_80073CC4(uint32_t source, uint32_t handler);
uint32_t func_800740D0(uint32_t source, uint32_t handler);
uint32_t func_80073CF4(uint32_t channel, uint32_t handler);
uint32_t func_800746A0(uint32_t channel, uint32_t handler);
uint32_t func_80073D88(void);
uint32_t func_80074218(void);
uint32_t func_80073DB8(void);
uint32_t func_800742B8(void);
uint32_t func_8007440C(void);
uint32_t func_800743B4(void);
uint32_t func_800744D4(void);
uint32_t func_80074520(void);
uint32_t func_80073F00(void);
#endif
