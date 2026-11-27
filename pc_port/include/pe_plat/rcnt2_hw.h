/* Root counter 2 model: the tick source used for timing.
 * Declaration-only header (no function bodies): the port's host model API for root counter 2.
 * Declarations were moved unchanged from the port's own host-model header platform/pe_rcnt2.h
 * (commit e15e3ee7c); comments describe hardware behaviour as documented publicly (psx-spx)
 * in the project's own words.  No SDK source, tables or disassembly are reproduced here. */
#ifndef PE_PLAT_RCNT2_HW_H
#define PE_PLAT_RCNT2_HW_H
#include <stdint.h>

void PE_Rcnt2_Reset(void);
void PE_Rcnt2_WriteCounter(uint16_t value);
void PE_Rcnt2_WriteMode(uint16_t value);
void PE_Rcnt2_WriteTarget(uint16_t value);
uint16_t PE_Rcnt2_ReadCounter(void);
uint16_t PE_Rcnt2_ReadMode(void);   /* clears the reached flags (bits 11/12) */
uint16_t PE_Rcnt2_PeekMode(void);   /* same value, no side effect (tests)   */
uint16_t PE_Rcnt2_ReadTarget(void);
/* Advance by `cycles` CPU cycles. */
void PE_Rcnt2_Advance(uint32_t cycles);

#endif
