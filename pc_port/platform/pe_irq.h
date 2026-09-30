/*
 * Phase 6E-B53I-B1 — single 16-bit I_STAT/I_MASK platform authority.
 *
 * Retail evidence (SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b):
 * I_STAT (0x1F801070) and I_MASK (0x1F801074) are both accessed with
 * halfword loads/stores.  This module is their only native storage.
 *
 * I_STAT writes are write-zero-to-clear: a written zero clears that bit and
 * a written one retains it.  Hardware assertion ORs status independently of
 * I_MASK.  None of these primitives dispatches an interrupt or invokes a
 * callback; B53I-B2 owns delivery.
 */
#ifndef PE_IRQ_H
#define PE_IRQ_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t PeIrqGeneration;

/* Host/platform lifecycle reset.  This is distinct from retail
 * ResetCallback: it clears both registers and advances the stale-event
 * generation.  It does not install retail callback identities. */
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

/* Exact func_80073E10 semantics: return the previous 16-bit mask
 * (zero-extended lhu), then replace it with the new 16-bit value (sh).
 * No IRQ dispatch, no I_STAT mutation, no callback invocation. */
uint16_t PE_IRQ_ExchangeMask(uint16_t new_mask);

#ifdef __cplusplus
}
#endif

#endif /* PE_IRQ_H */
