extern int D_8009D124;
extern int D_8009D128;
extern void func_8005EB64(int);
extern void func_8005F874(int);

void func_8005FCAC(int value)
{
    int digits = 3;
    int divisor = 1;
    int i;
    if (value < 0) {
        value = -value;
        func_8005EB64(0x52);
        digits = 2;
        {
            int *p = &D_8009D128;
            D_8009D124 += 5;
            *p = D_8009D128;
        }
    } else if (value > 0) {
        func_8005EB64(0x89);
        digits = 2;
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
