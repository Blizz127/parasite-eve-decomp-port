extern unsigned char *volatile D_800B1624;

int func_800659F8(unsigned int a0, unsigned char a1)
{
    char pad[8];
    unsigned char *p = D_800B1624;
    unsigned char *q = D_800B1624;
    register unsigned char *base asm("$2");
    register unsigned char *r asm("$3");
    int i;
    int n;

    q += *(unsigned int *)(p + 0x10);
    q += a0 << 4;
    base = q + *(unsigned int *)(q + 0xC);
    n = *(unsigned int *)q >> 8;
    i = 0;
    if (n != 0) {
        r = base;
        do {
            r[1] = a1;
            i++;
            r += 2;
        } while (i < n);
    }
    return 0;
}
