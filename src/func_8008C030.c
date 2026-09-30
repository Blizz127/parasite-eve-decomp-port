extern unsigned int D_800BCD50;
extern unsigned char D_800BC0F4[];

void func_8008C030(unsigned char *a0) {
    unsigned int mask = 0x1000;
    unsigned int flags = D_800BCD50;
    unsigned int i = 0;
    unsigned int bit = 0x2000000;
    unsigned char *p = D_800BC0F4;

    do {
        if (flags & mask) {
            if ((*(volatile unsigned int *)(p - 0xC8) & bit) == 0) {
                unsigned int v = *(unsigned char *)(a0 + 4);
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
