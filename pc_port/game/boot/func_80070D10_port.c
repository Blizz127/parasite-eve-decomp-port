/*
 * Phase 6E-B1 — func_80070D10: lagged-Fibonacci RNG table init.
 *
 * Retail evidence (asm/disc1/5F3E4.s:2439-2464, exe 0x80070D10-0x80070D6B,
 * 23 words / 0x5C, file offset 0x61510):
 *   t0 = 0x80070E0C;                 // table base
 *   [t0+0x40] = 1;                   // w[16] seed
 *   [t0+0x3C] = 2;                   // w[15] seed
 *   t5 = 0xE;
 *   do {
 *       t3 = [t0+0x40]; t4 = [t0+0x3C];
 *       t0 -= 4;
 *       [t0+0x3C] = t3 + t4;         // w[k] = w[k+1] + w[k+2]
 *   } while (t5-- != 0);             // bnez on pre-decrement: 15 bodies
 *   [0x80070E04] = 0x40;             // index1 byte offset
 *   [0x80070E08] = 0x10;             // index2 byte offset (delay-slot store)
 *
 * Result: 17-word table 0x80070E0C..0x80070E4C =
 *   { 2584, 1597, 987, 610, 377, 233, 144, 89, 55, 34, 21, 13, 8, 5, 3, 2, 1 }
 * (descending Fibonacci), index words 0x80070E04 = 0x40, 0x80070E08 = 0x10.
 *
 * Contract: void(void); no arguments (delay slot at sole call site
 * 0x8003E6B4 is `addu $s0,$zero,$zero`), no return value ($v0 never
 * written).  Sole caller: func_8003E680.  The block 0x80070E04..0x80070E4C
 * is touched only by this function and func_80070D6C (the RNG advance, next
 * strict-mode frontier); func_80070DD0 reaches it only through 70D6C.
 * No SDK calls, no callbacks, no hardware access, cannot block.  Idempotent:
 * every call deterministically re-seeds the identical state.
 *
 * Classification: TRANSLATED retail game logic — the seed half of the game's
 * lagged-Fibonacci RNG.  (The shim_inventory note labeling sibling
 * func_80070D6C a "hardware poll loop" is disproven at instruction level:
 * its only loads/stores are inside this block.)
 */
#include "psx_compat.h"

#define GA_RNG_INDEX1  0x80070E04u   /* write-slot byte offset, seeded 0x40 */
#define GA_RNG_INDEX2  0x80070E08u   /* read-slot byte offset,  seeded 0x10 */
#define GA_RNG_TABLE   0x80070E0Cu   /* 17 words, through 0x80070E4C        */

void func_80070D10(void)
{
    pe_addr_t t0 = GA_RNG_TABLE;
    unsigned int t3, t4;
    int t5;

    PE_StoreU32(t0 + 0x40, 1);
    PE_StoreU32(t0 + 0x3C, 2);
    t5 = 0xE;
    do {
        t3 = PE_LoadU32(t0 + 0x40);
        t4 = PE_LoadU32(t0 + 0x3C);
        t0 -= 4;
        PE_StoreU32(t0 + 0x3C, t3 + t4);
    } while (t5-- != 0);
    PE_StoreU32(GA_RNG_INDEX1, 0x40);
    PE_StoreU32(GA_RNG_INDEX2, 0x10);
}
