/*
 * Phase 6E-B53I-B1 — single 16-bit I_STAT/I_MASK authority.
 *
 * Basic register operations are deliberately inert with respect to retail
 * callback tables, DMA, DICR, scheduling, and dispatch.  The generation is
 * host-only and invalidates asynchronous work captured before reset.
 */
#include "pe_irq.h"

static uint16_t g_i_status;
static uint16_t g_i_mask;
static PeIrqGeneration g_irq_generation = 1u;

void PE_IRQ_Reset(void)
{
    g_irq_generation++;
    if (g_irq_generation == 0u) {
        /* Zero is kept out of the token space.  Reaching this requires
         * 2^64-1 host resets and is practically impossible. */
        g_irq_generation = 1u;
    }
    g_i_status = 0;
    g_i_mask = 0;
}

uint16_t PE_IRQ_ReadStatus(void)
{
    return g_i_status;
}

void PE_IRQ_WriteStatus(uint16_t retain_bits)
{
    g_i_status = (uint16_t)(g_i_status & retain_bits);
}

void PE_IRQ_AssertSources(uint16_t sources)
{
    g_i_status = (uint16_t)(g_i_status | sources);
}

int PE_IRQ_AssertSourcesForGeneration(uint16_t sources,
                                     PeIrqGeneration generation)
{
    if (generation != g_irq_generation) {
        return 0;
    }
    PE_IRQ_AssertSources(sources);
    return 1;
}

PeIrqGeneration PE_IRQ_Generation(void)
{
    return g_irq_generation;
}

uint16_t PE_IRQ_GetMask(void)
{
    return g_i_mask;
}

uint16_t PE_IRQ_ExchangeMask(uint16_t new_mask)
{
    uint16_t previous = g_i_mask;
    g_i_mask = new_mask;
    return previous;
}
