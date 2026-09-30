extern unsigned int *D_8009D2C8;

void func_8008AB9C(unsigned char *a0)
{
    unsigned int m;
    unsigned int bit;
    int *p;

    m = D_8009D2C8[1];
    if (m != 0) {
        bit = 1;
        p = (int *)(a0 + 0xF4);
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
