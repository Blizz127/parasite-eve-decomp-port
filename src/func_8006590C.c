extern unsigned char *volatile D_800B1624;

int func_8006590C(unsigned int a0, unsigned int a1)
{
    unsigned char *p = D_800B1624;
    unsigned char *q = D_800B1624;
    register unsigned int w asm("$4");
    register unsigned int b asm("$2");

    q += *(unsigned int *)(p + 0x10);
    q += a0 << 4;
    w = q[4];
    b = *q;
    *(unsigned short *)(q + 0xA) = 0;
    w = w | (a1 << 16);
    b = b | 2;
    *q = b;
    *(unsigned int *)(q + 4) = w;
    return 0;
}
