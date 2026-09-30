/* Phase 6C: func_8005FDF0 — VRAM 0x8005FDF0, size 0x138, file 0x505F0-0x50728.
 * Four-digit signed decimal emitter; same body as func_8005FA3C with
 * digits = 4 and the negative prefix reducing the field to 3 digits.
 * era_o2_g8_expand_div (-O2 -G8 + MASPSX_EXPAND_DIV=1). */
extern int D_8009D124;
extern int D_8009D128;
extern void func_8005EB64(int);
extern void func_8005F874(int);

void func_8005FDF0(int value)
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
