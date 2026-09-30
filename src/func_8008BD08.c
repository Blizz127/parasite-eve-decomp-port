extern unsigned int D_800BCD50;
extern unsigned char D_800BC078[];

void func_8008BD08(unsigned char *a0) {
    unsigned int mask = 0x1000;
    unsigned int flags = D_800BCD50;
    unsigned int i = 0;
    unsigned int bit = 0x2000000;
    unsigned char *p = D_800BC078;

    do {
        if (flags & mask) {
            if ((*(volatile unsigned int *)(p - 0x4C) & bit) == 0) {
                int t = *(int *)(a0 + 4);
                int n = 1;
                short val;
                if (t != 0) {
                    n = t;
                }
                val = (short)((*(unsigned char *)(a0 + 8) << 8) -
                              *(volatile unsigned short *)(p - 2)) / (short)n;
                *(volatile short *)p = n;
                *(volatile short *)(p + 0x64) = val;
            }
        }
        i++;
        p += 0x11C;
        mask <<= 1;
    } while (i < 0xC);
}
