extern unsigned int D_800BCD50;
extern unsigned char D_800BC000[];

void func_8008BDC8(unsigned char *a0) {
    unsigned char *base = D_800BC000;
    unsigned int flags = D_800BCD50;
    unsigned int mask = 0x1000;
    unsigned int i;
    unsigned char *p;

    if (*(int *)(a0 + 8) != 0) {
        i = 0;
        p = base + 0xF4;
        do {
            if (flags & mask) {
                if (*(volatile unsigned int *)(p - 0xC8) & *(int *)(a0 + 8)) {
                    unsigned int v = *(unsigned char *)(a0 + 0xC);
                    unsigned int f = *(volatile unsigned int *)p;
                    *(volatile unsigned short *)(p - 0x84) = 0;
                    *(volatile unsigned int *)(p - 0xB8) = v << 8;
                    *(volatile unsigned int *)p = f | 0x10;
                }
            }
            i++;
            p += 0x11C;
            mask <<= 1;
        } while (i < 0xC);
    } else {
        i = 0;
        p = base + 0xF4;
        do {
            if (flags & mask) {
                if (*(volatile unsigned int *)(p - 0xCC) == *(unsigned int *)(a0 + 4)) {
                    unsigned int v = *(unsigned char *)(a0 + 0xC);
                    unsigned int f = *(volatile unsigned int *)p;
                    *(volatile unsigned short *)(p - 0x84) = 0;
                    *(volatile unsigned int *)(p - 0xB8) = v << 8;
                    *(volatile unsigned int *)p = f | 0x10;
                }
            }
            i++;
            p += 0x11C;
            mask <<= 1;
        } while (i < 0xC);
    }
}
