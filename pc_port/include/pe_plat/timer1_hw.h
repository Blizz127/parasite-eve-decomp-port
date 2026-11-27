/* Timer 1 model: horizontal-retrace counter reads.
 * Declaration-only header (no function bodies): the port's host model API for timer 1.
 * Declarations were moved unchanged from the port's own host-model header platform/pe_timer1.h
 * (commit e15e3ee7c); comments describe hardware behaviour as documented publicly (psx-spx)
 * in the project's own words.  No SDK source, tables or disassembly are reproduced here. */
#ifndef PE_PLAT_TIMER1_HW_H
#define PE_PLAT_TIMER1_HW_H
#include <stdint.h>
#include "pe_plat/irq_hw.h"
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
