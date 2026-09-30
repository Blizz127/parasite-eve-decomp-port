void func_80089724(unsigned char *a0, unsigned int *a1, unsigned int a2, unsigned int a3)
{
    unsigned int bit;
    unsigned int v;

    bit = 1;
    do {
        if (a2 & bit) {
            v = *(unsigned int *)(a0 + 0xF0);
            if (v < 0x18) {
                *a1 |= 1 << v;
            }
        }
        a2 &= ~bit;
        a0 += 0x11C;
        bit <<= 1;
    } while (a2 != 0);
    *a1 &= a3;
}
