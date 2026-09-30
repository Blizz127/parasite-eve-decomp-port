/* Phase 6C: func_8005FF28 — VRAM 0x8005FF28, size 0x144, file 0x50728-0x5086C.
 * Four-digit signed decimal emitter with a forced sign glyph: value < 0 emits
 * 0x52 after negation and value > 0 emits 0x89; zero skips the prefix and
 * keeps the full 4-digit field. era_o2_g8_expand_div
 * (-O2 -G8 + MASPSX_EXPAND_DIV=1). */
extern int D_8009D124;
extern int D_8009D128;
extern void func_8005EB64(int);
extern void func_8005F874(int);

void func_8005FF28(int value)
{
    int digits = 4;
    int divisor = 1;
    int i;
    if (value < 0) {
        value = -value;
        func_8005EB64(0x52);
        digits = 3;
        {
            int *p = &D_8009D128;
            D_8009D124 += 5;
            *p = D_8009D128;
        }
    } else if (value > 0) {
        func_8005EB64(0x89);
        digits = 3;
        {
            int *p = &D_8009D128;
            D_8009D124 += 5;
            *p = D_8009D128;
        }
    }
    for (i = 1; i < digits; i++)
        divisor *= 10;
    for (i = 0; i < digits; i++) {
        int digit = value / divisor;
        func_8005F874(i < digits - 1 && digit == 0 ? -1 : digit);
        {
            int *p = &D_8009D128;
            D_8009D124 += 5;
            *p = D_8009D128;
        }
        divisor /= 10;
    }
}
