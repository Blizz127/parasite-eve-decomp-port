/* Phase 6C: func_8005FA3C — VRAM 0x8005FA3C, size 0x138, file 0x5023C-0x50374.
 * Two-digit signed decimal emitter: negate-and-prefix on negative input,
 * build divisor 10^(digits-1), then for each digit call func_8005F874 with
 * -1 for a suppressed leading zero and advance the gp text cursor pair
 * D_8009D124 (+5) / D_8009D128. The cursor self-store must survive: the
 * `int *p = &D_8009D128; *p = D_8009D128;` idiom keeps it in the output.
 * era_o2_g8_expand_div (-O2 -G8 + MASPSX_EXPAND_DIV=1) for the checked div. */
extern int D_8009D124;
extern int D_8009D128;
extern void func_8005EB64(int);
extern void func_8005F874(int);

void func_8005FA3C(int value)
{
    int digits = 2;
    int divisor = 1;
    int i;
    if (value < 0) {
        value = -value;
        func_8005EB64(0x52);
        digits = 1;
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
