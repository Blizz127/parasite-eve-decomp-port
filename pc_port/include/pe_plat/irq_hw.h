/* Interrupt controller model: status/mask registers, callback slots and host lifecycle.
 * Declaration-only header (no function bodies): the port's host model API for the interrupt status and mask registers.
 * Declarations were moved unchanged from the port's own host-model header platform/pe_irq.h
 * (commit e15e3ee7c); comments describe hardware behaviour as documented publicly (psx-spx)
 * in the project's own words.  No SDK source, tables or disassembly are reproduced here. */
#ifndef PE_PLAT_IRQ_HW_H
#define PE_PLAT_IRQ_HW_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t PeIrqGeneration;

/* Host/platform lifecycle reset: clears both registers and advances the
 * stale-event generation.  It does not install guest callback identities. */
void PE_IRQ_Reset(void);

uint16_t PE_IRQ_ReadStatus(void);

/* I_STAT = I_STAT & retain_bits (16-bit W0C). */
void PE_IRQ_WriteStatus(uint16_t retain_bits);

/* Immediate hardware-source latch in the current generation.  Masked
 * sources remain pending and no callback is executed. */
void PE_IRQ_AssertSources(uint16_t sources);

/* Generation-checked form for future asynchronous producers.  Returns 1
 * when the assertion was admitted, 0 for a stale generation. */
int PE_IRQ_AssertSourcesForGeneration(uint16_t sources,
                                     PeIrqGeneration generation);

PeIrqGeneration PE_IRQ_Generation(void);

uint16_t PE_IRQ_GetMask(void);

/* Returns the previous 16-bit mask, then stores the new 16-bit value.
 * No IRQ dispatch, no I_STAT change, no callback invocation. */
uint16_t PE_IRQ_ExchangeMask(uint16_t new_mask);

#ifdef __cplusplus
}
#endif

#endif /* PE_PLAT_IRQ_HW_H */
