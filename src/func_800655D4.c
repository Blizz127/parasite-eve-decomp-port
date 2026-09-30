extern unsigned char *volatile D_800B1624;

int func_800655D4(void)
{
    char pad[8];
    register unsigned char *p asm("$2");
    register unsigned char *q asm("$4");
    register unsigned char *base asm("$7");
    register unsigned char *a asm("$4");
    register unsigned char *tbl asm("$8");
    register unsigned int i asm("$5");
    register unsigned int n asm("$6");
    register unsigned int one asm("$9");
    register unsigned int c100 asm("$10");
    register unsigned int off asm("$3");
    register unsigned int v asm("$3");
    register unsigned int b asm("$3");
    register unsigned int o asm("$2");
    register unsigned char *s asm("$2");

    p = D_800B1624;
    q = D_800B1624;
    off = *(unsigned int *)(p + 0x10);
    i = 0;
    base = q + off;
    off = *(unsigned int *)(p + 0x14);
    n = *(unsigned short *)(p + 4);
    tbl = q + off;
    if (n != 0) {
        c100 = 0x100;
        one = 1;
        a = base;
        do {
            v = a[4];
            o = *(unsigned int *)(a + 0xC);
            *(unsigned short *)(a + 8) = c100;
            *(unsigned short *)(a + 0xA) = 0;
            *a = one;
            *(unsigned int *)(a + 4) = v;
            s = a + o;
            b = *s;
            s = (unsigned char *)(b * 56 + (unsigned int)tbl);
            b = *s;
            b = b | 2;
            *s = b;
            i++;
            a += 0x10;
        } while (i < n);
    }
    return 0;
}
