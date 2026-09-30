unsigned char *func_8006E514(unsigned char *a0, int a1)
{
    char pad[8];
    register unsigned int i asm("$8");
    register unsigned int m asm("$9");
    register unsigned char *p asm("$6");
    register unsigned char *r asm("$7");
    register unsigned char *base asm("$2");
    unsigned int w;
    unsigned int n;

    w = *(unsigned int *)(a0 + *(int *)(a0 + 4) + 0x2C);
    base = a0 + (w & 0x3FFFFF);
    i = 0;
    r = 0;
    n = w >> 22;
    if (n != 0) {
        m = 0xFFFFFF;
        p = base;
        do {
            if (*(unsigned short *)(p + 0xA) == a1) {
                return a0 + (*(unsigned int *)(p + 4) & m);
            }
            i++;
            p += 0xC;
        } while (i < n);
    }
    return r;
}
