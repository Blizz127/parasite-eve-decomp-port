/*
 * Read-only names for the two retail .data authorities the DMA callback path
 * uses (own host code; re-homed 2026-11-27 from the retired translated
 * game/boot/func_800746A0_port.c, whose function itself is bound to the
 * clean-room pe_plat_dma_callback in pe_plat/irq_callback.h).
 *
 *   0x800956C0  eight 32-bit guest callback identities (one per DMA channel)
 *   0x800956BC  .data word holding the DICR MMIO address literal 0x1F8010F4
 */
#include "psx_compat.h"

#define GA_DMA_CALLBACK_TABLE   0x800956C0u
#define GA_DMA_DICR_POINTER     0x800956BCu
#define PE_DMA_DICR_ADDRESS     0x1F8010F4u

pe_addr_t PE_DMA_CallbackSlotAddress(uint32_t dma_channel)
{
    return (pe_addr_t)(GA_DMA_CALLBACK_TABLE + (uint32_t)(dma_channel << 2));
}

pe_addr_t PE_DMA_DicrPointerAddress(void)
{
    return (pe_addr_t)GA_DMA_DICR_POINTER;
}

uint32_t PE_DMA_DicrMmioAddress(void)
{
    return PE_DMA_DICR_ADDRESS;
}
