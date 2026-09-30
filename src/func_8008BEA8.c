extern unsigned int D_800BCD50;
extern unsigned char D_800BC000[];

void func_8008BEA8(unsigned char *a0) {
    unsigned char *base = D_800BC000;
    unsigned int flags = D_800BCD50;
    unsigned int mask = 0x1000;
    unsigned int i;
    unsigned char *p;

    if (*(int *)(a0 + 8) != 0) {
        i = 0;
        p = base + 0x70;
        do {
            if (flags & mask) {
                if (*(volatile unsigned int *)(p - 0x44) & *(int *)(a0 + 8)) {
                    int t = *(int *)(a0 + 0xC);
                    int n = 1;
                    short val;
                    if (t != 0) {
                        n = t;
                    }
                    val = (short)((*(unsigned char *)(a0 + 0x10) << 8) -
                                  *(volatile int *)(p - 0x34)) / (short)n;
                    *(volatile short *)p = n;
                    *(volatile int *)(p - 0x30) = val;
                }
            }
            i++;
            p += 0x11C;
            mask <<= 1;
        } while (i < 0xC);
    } else {
        i = 0;
        p = base + 0x70;
        do {
            if (flags & mask) {
                if (*(volatile unsigned int *)(p - 0x48) == *(unsigned int *)(a0 + 4)) {
                    int t = *(int *)(a0 + 0xC);
                    int n = 1;
                    short val;
                    if (t != 0) {
                        n = t;
                    }
                    val = (short)((*(unsigned char *)(a0 + 0x10) << 8) -
                                  *(volatile int *)(p - 0x34)) / (short)n;
                    *(volatile short *)p = n;
                    *(volatile int *)(p - 0x30) = val;
                }
            }
            i++;
            p += 0x11C;
            mask <<= 1;
        } while (i < 0xC);
    }
}
