extern int D_800C0E24[];
extern short D_800A1D9C[];
extern int D_8009D068;
extern int D_8009D040;

void func_80055610(void)
{
    int mask;
    short *p;
    int i;

    mask = D_800C0E24[0];
    p = D_800A1D9C;
    i = 0;
    do {
        if (mask & 1) {
            *p = i;
            p++;
        }
        i++;
        mask >>= 1;
    } while (i < 0x14);
    D_8009D068 = 0;
    D_8009D040 = p - D_800A1D9C;
}
