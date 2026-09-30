/*
 * Phase 6E-B2 — func_80070D6C: lagged-Fibonacci RNG advance.
 *
 * Retail evidence (asm/disc1/5F3E4.s:2468-2496, exe 0x80070D6C-0x80070DCF,
 * 25 words / 0x64, file offset 0x6156C):
 *   t0 = 0x80070E0C (table base);  t7 = 0x80070E04;  t8 = 0x80070E08
 *   t1 = [t7];  t2 = [t8]                (index words, signed)
 *   t3 = t0 + t1;  t4 = t0 + t2          (addu — 32-bit wrap)
 *   t5 = [t3] + [t4];  [t3] = t5;  v0 = t5
 *   t1 -= 4;  t2 -= 4                    (t2 in the first bgez delay slot)
 *   if (t1 < 0) t1 = 0x40;               (ori $t1, $zero, 0x40)
 *   if (t2 < 0) t2 |= 0x40;              (ori $t2, $t2, 0x40 — NOT +0x40)
 *   [t7] = t1 (always, delay slot);  [t8] = t2 (always, delay slot)
 *
 * The `t2 |= 0x40` wrap is verbatim-correct, not a bug to repair: for the
 * reachable values (multiples of 4 from -4 down) bit 6 is already set until
 * t2 = -68 = 0xFFFFFFBC, where the OR maps to -4.  The read cursor t2
 * therefore CYCLES with period 17 over offsets {-4..-68}, i.e. addresses
 * 0x80070E08 down to 0x80070DCC — the two index words and 14 words of
 * retail CODE (tail of func_80070D6C + all of func_80070DD0) below the
 * table.  Proven by pc_port/tools/rng_oracle.py against the retail exe
 * (SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b): over 2000 calls
 * i2 in [-64,12] union the cycle, addr2 in [0x80070DCC, 0x80070E1C],
 * writes confined to 0x80070E04..0x80070E4C.  The earlier handoff's
 * "index drifts negative forever" claim was wrong.
 *
 * Contract: unsigned int(void) — full 32-bit $v0.  Consumers: discarded by
 * the func_8003E680 x2000 warm-up and the func_8003E91C reset callback
 * (advance-only); consumed as a full word by func_800176FC; masked to
 * 16 bits by func_80070DD0.  Retail-exact output requires the retail exe
 * bytes at 0x80070DCC..0x80070E00 in guest RAM (pe_guest_image); without
 * an image those words are zero — a documented fixture-mode divergence
 * that nothing on the boot-to-black path consumes.
 *
 * Classification: TRANSLATED retail RNG logic.
 */
#include "psx_compat.h"

#define GA_RNG_INDEX1  0x80070E04u
#define GA_RNG_INDEX2  0x80070E08u
#define GA_RNG_TABLE   0x80070E0Cu

unsigned int func_80070D6C(void)
{
    int t1 = (int)PE_LoadU32(GA_RNG_INDEX1);
    int t2 = (int)PE_LoadU32(GA_RNG_INDEX2);
    pe_addr_t t3 = GA_RNG_TABLE + (pe_addr_t)t1;   /* addu */
    pe_addr_t t4 = GA_RNG_TABLE + (pe_addr_t)t2;   /* addu */
    unsigned int t5 = PE_LoadU32(t3) + PE_LoadU32(t4);  /* addu wrap */
    PE_StoreU32(t3, t5);
    t1 -= 4;
    t2 -= 4;
    if (t1 < 0) t1 = 0x40;
    if (t2 < 0) t2 = (int)((unsigned int)t2 | 0x40u);   /* ori, verbatim */
    PE_StoreU32(GA_RNG_INDEX1, (unsigned int)t1);
    PE_StoreU32(GA_RNG_INDEX2, (unsigned int)t2);
    return t5;
}
