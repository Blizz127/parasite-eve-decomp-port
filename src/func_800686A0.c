void func_800686A0(unsigned char *a0, int a1, int a2, int a3)
{
    register unsigned char *p asm("$8");
    register int i asm("$9");
    register int off asm("$4");
    unsigned char *base;
    unsigned char *q;

    p = a0;
    for (i = 0; i < *(unsigned short *)(p + 0x26); i++) {
        off = i << 4;
        base = *(unsigned char **)(p + 0x30);
        q = (unsigned char *)(off + (unsigned int)base);
        q[4] = a1;
        q[5] = a2;
        q[6] = a3;
        base = *(unsigned char **)(p + 0x30);
        base += *(unsigned short *)(p + 0x26) << 4;
        off = off + (int)base;
        *(unsigned char *)(off + 4) = a1;
        *(unsigned char *)(off + 5) = a2;
        *(unsigned char *)(off + 6) = a3;
    }
}
