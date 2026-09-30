/*
 * Phase 6E-B17 — func_800527C8: multi-subsystem bootstrap dispatcher.
 *
 * Raw body: 49 words / 0xC4, exe 0x800527C8–0x8005288B, file 0x42FC8,
 * sole live split asm/disc1/42FC8.s:12–62; all 49 instruction words
 * verified exact against the SHA-exact retail executable.
 *
 * Sole call site in the executable: func_8006A9E4 @0x8006AAD0, inside
 * the cycle-B poll loop.  Retail guards it with $s1: cleared ONCE per
 * func_8006A9E4 execution (0x8006AA90, before the cycle-B issue loop)
 * and set to 1 in the call's delay slot — so func_800527C8 runs AT MOST
 * ONCE per streaming load, even across poll -1 restarts and issue
 * retries (the restart branch .L8006AA94 does not reset $s1).  The
 * return value is never referenced after the call → void(void).
 *
 * ROM-order operation map (17 calls, 16 distinct callees):
 *   1. func_8005B890(0)          TRANSLATED leaf (B17): D_8009D028 = 0
 *   2. func_8005BC98(a0=0)       TRANSLATED leaf (B17): D_8009D218 = 1
 *                                (leaf ignores a0 — both delay-slot
 *                                argument setups are dead)
 *   3. sw $zero → 0x2BC($gp)     direct: D_8009D02C = 0
 *      sw $zero → 0x2C0($gp)     direct: D_8009D030 = 0
 *      sw $zero → 0x2C4($gp)     direct: D_8009D034 = 0
 *   4. func_8004F808()           TRANSLATED leaf (B17): ten-word clear
 *   5. func_800528F0()           TRANSLATED (B18): PRNG table generator,
 *                                the previous strict-mode frontier
 *   6. func_8005E588()           TRANSLATED (B19): display env setup
 *   7. func_80062568()           TRANSLATED (B20): free-list pool init
 *   8. func_80064964()           TRANSLATED (B21): A(28h) bzero + flags
 *   9. func_8005DE88()           TRANSLATED (B22): resource-list/state init
 *  10. func_80042B38()           TRANSLATED leaf (B17): D_800A1870/1874 = 0
 *  11. func_80051084()           TRANSLATED leaf (B17): D_8009D014 =
 *                                0x800A1AA0 (guest address, not host ptr)
 *  12. func_80052C6C()           TRANSLATED (B23): resource-table search + init
 *  13. func_8005BCBC(0)          TRANSLATED (B24): resource-state
 *                                pointer/count selector (a0=0 in the
 *                                delay slot; D_8009D218 = 1 at entry →
 *                                D_8009D0C0 = 0x800C0DF0, D_8009D0C4 = 8,
 *                                D_8009D0C8 = 0; return 8 unconsumed)
 *  14. func_8005D6F4()           TRANSLATED (B25): resource-buffer +
 *                                display-state initializer (147 words;
 *                                bzero 0x800C0DE0 len 0x12E4, buffer
 *                                fills/terminators; its complete direct
 *                                dependency chain is translated through B38)
 *  15. func_80051CC4()           TRANSLATED (B39): resource-command state
 *                                initialization; B40 translates its first
 *                                dependency func_8005332C; B43 translates
 *                                func_8005218C's prefix and B44 supplies all
 *                                seven exact func_8005B91C output words
 *  16. func_80042C78()           TRANSLATED (B30 prefix); its
 *                                func_80042CC4 is translated in B31
 *  17. func_8005BC98(a0=1)       TRANSLATED leaf: D_8009D218 = 1 again
 *      a0=1; v1 = D_800B0CD8 | 0x40000000;
 *      sw $v1 → D_800B0CD8        direct RMW in the jal DELAY SLOT —
 *                                committed BEFORE func_800371A4 runs
 *  18. func_800371A4(1)          REAL (B7): D_8009CE94 = 1
 *
 * Direct write footprint (this body): D_8009D02C/D_8009D030/D_8009D034
 * word clears + D_800B0CD8 |= 0x40000000.  All other effects belong to
 * callees.  Guest state visible on entry (produced by func_8006A9E4
 * cycle B): the issue has set D_800B0CD8 |= 0x01004000; the OR with
 * 0x40000000 preserves it.  v0 on return = func_800371A4's previous-
 * byte return; never consumed.
 *
 * No callee return values are consumed.  The dispatcher is NOT
 * transactional: on an interrupted first invocation (strict stop at
 * func_800528F0) the direct clears, the four translated leaves and the
 * final RMW/byte store stay exactly as retail committed them.
 *
 * Classification: 1 — translated retail logic (multi-subsystem
 * bootstrap dispatcher); unresolved callees routed through the
 * centralized bootstrap boundary in retail order.
 */
#include "psx_compat.h"
#include "pe_sdk.h"
#include "pe_bootstrap.h"

extern void func_80042C78(void);

#define GA_D_8009D02C  0x8009D02Cu   /* $gp+0x2BC */
#define GA_D_8009D030  0x8009D030u   /* $gp+0x2C0 */
#define GA_D_8009D034  0x8009D034u   /* $gp+0x2C4 */

void func_800527C8(void)
{
    func_8005B890(0);
    func_8005BC98();                 /* a0 ignored by the retail leaf */
    PE_StoreU32(GA_D_8009D02C, 0u);
    PE_StoreU32(GA_D_8009D030, 0u);
    PE_StoreU32(GA_D_8009D034, 0u);
    func_8004F808();
    func_800528F0();
    func_8005E588();
    func_80062568();
    func_80064964();
    func_8005DE88();
    func_80042B38();
    func_80051084();
    func_80052C6C();
    func_8005BCBC(0u);               /* a0=0 in the jal delay slot */
    func_8005D6F4();
    func_80051CC4();
    func_80042C78();
    func_8005BC98();                 /* a0 ignored by the retail leaf */
    D_800B0CD8 |= 0x40000000u;       /* jal delay slot: before 371A4 */
    func_800371A4(1);
}
