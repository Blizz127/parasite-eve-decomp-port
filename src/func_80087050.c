/*
 * func_80087050 — wait for / test the low bit of a status word.
 *
 * VRAM 0x80087050 / file 0x77850 / size 0x40 (16 words).
 * ROM: asm/disc1/77850.s.
 *
 * Retail layout:
 *   if (a0 == 0) { while (D_8009D2E0 & 1) ; return 0; }
 *   return D_8009D2E0 & 1;
 * The a0==0 loop is emitted first (lower addresses) with an explicit `j` past
 * the a0!=0 arm to the shared epilogue. D_8009D2E0 must be volatile: without
 * it cc1 hoists the second load out of the loop and the test reuses a stale
 * value.
 */
extern volatile unsigned int D_8009D2E0;

int func_80087050(int a0) {
    if (a0 == 0) {
        while (D_8009D2E0 & 1)
            ;
        return 0;
    }
    return D_8009D2E0 & 1;
}
