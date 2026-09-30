extern unsigned int D_800BCD50;
extern unsigned char D_800BC000[];

void func_8008B780(unsigned char *a0) {
    unsigned char *base = D_800BC000;
    unsigned int flags = D_800BCD50;
    unsigned int mask = 0x1000;
    unsigned int i;
    unsigned char *p;

    if (*(int *)(a0 + 8) != 0) {
        i = 0;
        p = base + 0x74;
        do {
            if (flags & mask) {
                if (*(volatile unsigned int *)(p - 0x48) & *(int *)(a0 + 8)) {
                    int t = *(int *)(a0 + 0xC);
                    register int n asm("$5") = 1;
                    short val;
                    if (t != 0) {
                        n = t;
                    }
                    val = (short)(((*(int *)(a0 + 0x10) & 0x7F) << 8) -
                                  *(volatile unsigned short *)(p + 0x64)) / (short)n;
                    *(volatile short *)p = n;
                    *(volatile short *)(p + 0x66) = val;
                }
            }
            i++;
            p += 0x11C;
            mask <<= 1;
        } while (i < 0xC);
    } else {
        i = 0;
        p = base + 0x74;
        do {
            if (flags & mask) {
                if (*(volatile unsigned int *)(p - 0x4C) == *(unsigned int *)(a0 + 4)) {
                    int t = *(int *)(a0 + 0xC);
                    register int n asm("$5") = 1;
                    short val;
                    if (t != 0) {
                        n = t;
                    }
                    val = (short)(((*(int *)(a0 + 0x10) & 0x7F) << 8) -
                                  *(volatile unsigned short *)(p + 0x64)) / (short)n;
                    *(volatile short *)p = n;
                    *(volatile short *)(p + 0x66) = val;
                }
            }
            i++;
            p += 0x11C;
            mask <<= 1;
        } while (i < 0xC);
    }
}
