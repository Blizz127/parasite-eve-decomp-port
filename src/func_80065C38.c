extern unsigned int D_800BCF88;
extern unsigned char *volatile D_800B1624;
extern unsigned char D_800BCFFD;
extern short D_800BCF8C;
extern short D_800BCF8E;

int func_80065C38(int a0, int a1)
{
    char pad[32];
    register unsigned char *p asm("$2");
    register unsigned char *r asm("$4");
    register unsigned char *h asm("$4");
    register int xc asm("$6");
    register int xo asm("$7");
    register int v2 asm("$4");
    register int t asm("$2");
    register int x asm("$3");
    register int y asm("$3");
    register int ys asm("$3");

    xc = a0;
    __asm__ __volatile__("" ::: "memory");
    if ((D_800BCF88 & 0x40) != 0) {
        xo = xc;
        __asm__ __volatile__("" : "=r"(xo) : "0"(xo));
        p = D_800B1624;
        r = D_800B1624;
        h = r + *(unsigned int *)(p + 0x1C);
        h = h + D_800BCFFD * 52;
        x = (short)xc;
        t = *(short *)(h + 0x2C);
        xc = t;
        __asm__ __volatile__("" : "=r"(xc) : "0"(xc));
        if (x < t) {
            goto setc;
        }
        t = *(short *)(h + 0x2E);
        xc = t;
        __asm__ __volatile__("" : "=r"(xc) : "0"(xc));
        t = (t < x);
        ys = a1 << 16;
        if (t == 0) {
            goto selse;
        }
    setc:
        D_800BCF8C = xc;
        ys = a1 << 16;
        goto join;
    selse:
        D_800BCF8C = xo;
    join:
        y = ys >> 16;
        t = *(short *)(h + 0x30);
        xc = t;
        __asm__ __volatile__("" : "=r"(xc) : "0"(xc));
        if (y < t) {
            D_800BCF8E = xc;
            return 0;
        }
        t = *(short *)(h + 0x32);
        v2 = t;
        __asm__ __volatile__("" : "=r"(v2) : "0"(v2));
        if (t < y) {
            D_800BCF8E = v2;
            return 0;
        }
        D_800BCF8E = a1;
    }
    return 0;
}
