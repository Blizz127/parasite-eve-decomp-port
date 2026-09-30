/*
 * Root counter 2 (0x1F801120 count / 0x1F801124 mode / 0x1F801128 target).
 *
 * Retail programs it once through libapi SetRCnt (src/func_80085814.c with
 * D_8009B7D0 = 0x1F801100, called as SetRCnt(0xF2000002, 0x44E8, 0x1000)
 * from func_80085644: mode 0x248 | 0x10 = 0x258, target 0x44E8) and polls
 * it in func_80084FC4 / func_80084FE4 (start a timeout / has it elapsed).
 * Hardware contract: https://psx-spx.consoledev.net/timers/
 *   mode bit 0     sync enable; counter 2 sync modes 0/3 stop the counter
 *   mode bit 3     reset counter to 0 when it reaches the target
 *   mode bit 4/5   IRQ at target / at 0xFFFF (IRQ delivery NOT modeled:
 *                  the ported readers poll; see PE_Rcnt2_Advance)
 *   mode bits 8-9  counter 2 clock: 0/1 system clock, 2/3 system clock / 8
 *   mode bit 10    IRQ request (1 = none), set by a mode write
 *   mode bit 11/12 reached target / 0xFFFF, cleared by reading the mode
 *   writing the mode resets the counter to 0.
 * Time is the port's deterministic CPU-cycle clock (HostFB_DeviceTime,
 * 33868800 cycles/s), never host wall time.
 */
#ifndef PE_RCNT2_H
#define PE_RCNT2_H
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
