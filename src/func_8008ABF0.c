extern unsigned int D_800BCD50;
extern unsigned char D_800BC000[];

void func_8008ABF0(void)
{
    unsigned int m;
    unsigned int bit;
    unsigned char *base;
    int *p;

    m = D_800BCD50;
    base = D_800BC000;
    if (m != 0) {
        bit = 0x1000;
        p = (int *)(base + 0xF4);
        do {
            if (m & bit) {
                *p |= 3;
                m ^= bit;
            }
            p = (int *)((char *)p + 0x11C);
            bit <<= 1;
        } while (m != 0);
    }
}
