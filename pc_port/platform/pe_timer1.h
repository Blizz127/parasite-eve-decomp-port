/* Timer1 lane used by the original VBlank initializer (mode 0x107).
 * Explicit GPU edges are inputs; reads never invent time or dispatch IRQs. */
#ifndef PE_TIMER1_H
#define PE_TIMER1_H
#include <stdint.h>
#include "pe_irq.h"
void PE_Timer1_Reset(void);
void PE_Timer1_InitializeVBlankCounter(void);
uint32_t PE_Timer1_ReadCounter(void);
uint32_t PE_Timer1_ReadMode(void);
/* Same value as ReadMode without clearing the reached flags (pe_mmio shadow). */
uint32_t PE_Timer1_PeekMode(void);
int PE_Timer1_HBlankEdge(PeIrqGeneration generation);
int PE_Timer1_VBlankEdge(PeIrqGeneration generation);
#endif
