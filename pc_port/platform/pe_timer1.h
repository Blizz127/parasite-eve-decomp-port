/* HBlank-clocked timer1: all VBlank sync modes, no IRQ delivery.
 * Counter/target writes do not create comparison events. Mode writes retain
 * target; cold reset clears it. Explicit GPU edges are inputs; reads never
 * invent time or dispatch IRQs. CPU-cycle write-hold timing is not modelled. */
#ifndef PE_TIMER1_H
#define PE_TIMER1_H
#include <stdint.h>
#include "pe_irq.h"
void PE_Timer1_Reset(void);
void PE_Timer1_InitializeVBlankCounter(void);
/* Returns 0 without mutation for unsupported clock/sync/IRQ configurations. */
int PE_Timer1_WriteMode(uint16_t value);
uint32_t PE_Timer1_ReadCounter(void);
void PE_Timer1_WriteCounter(uint16_t value);
uint32_t PE_Timer1_ReadTarget(void);
void PE_Timer1_WriteTarget(uint16_t value);
uint32_t PE_Timer1_ReadMode(void);
/* Same value as ReadMode without clearing the reached flags (pe_mmio shadow). */
uint32_t PE_Timer1_PeekMode(void);
/* GPU supplies its current VBlank level for level-gated sync0/2. */
int PE_Timer1_HBlankEdge(int vblank_level,PeIrqGeneration generation);
int PE_Timer1_VBlankEdge(PeIrqGeneration generation);
#endif
