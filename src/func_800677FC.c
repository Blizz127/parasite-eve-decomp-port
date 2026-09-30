extern unsigned char *D_800B1624;
extern unsigned char D_800BCFFD;
extern unsigned char D_800BCFFA;
extern unsigned char D_800BCFFB;
extern unsigned int D_800BCF88;
extern short D_800BD028;
extern short D_800BCF8C;
extern unsigned short D_800BD02A;
extern volatile unsigned short D_800BCF8E;
extern unsigned short D_800BCFAC;
extern unsigned short D_800BCFAE;
extern unsigned short D_800BCFB0;
extern unsigned short D_800BCFB2;

extern int func_80066800(unsigned int);
extern int func_80066F60(unsigned char *, int, int *);

int func_800677FC(int arg0, int *arg1) {
    register unsigned char *base asm("$16");
    register int r1 asm("$17");
    register int r2 asm("$18");
    register unsigned int n asm("$19");
    register int neg asm("$20");
    register int *out asm("$21");
    unsigned int idx;
    register unsigned char *rec asm("$5");
    register int x asm("$4");
    register int y asm("$2");
    unsigned int u;
    unsigned int *flags;

    r2 = arg0;
    r1 = (int)&D_800BCFFD;
    out = arg1;
    base = D_800B1624;
    func_80066800(*(unsigned char *)r1);
    idx = *(unsigned char *)r1;
    rec = base + *(int *)(base + 0x1C);
    rec = rec + idx * 52;
    x = *(short *)(rec + 0x2C);
    x = x + *(short *)(rec + 0x2E);
    x = x / 2;
    y = x;
    D_800BD028 = (short)y;
    D_800BCF8C = (short)y;
    y = *(short *)(rec + 0x30);
    y = y + *(short *)(rec + 0x32);
    u = (unsigned int)y;
    u = (u + (u >> 31)) >> 1;
    D_800BD02A = (unsigned short)u;
    D_800BCF8E = (unsigned short)u;
    D_800BCFAC = *(unsigned short *)(rec + 0x2C);
    D_800BCFAE = *(unsigned short *)(rec + 0x2E);
    D_800BCFB0 = *(unsigned short *)(rec + 0x30);
    r1 = 0;
    D_800BCFB2 = *(unsigned short *)(rec + 0x32);
    x = x - 0xA0;
    y = *(unsigned short *)(base + 0x2C);
    n = *(unsigned short *)(base + 6);
    *(unsigned short *)(base + 0x38) = (unsigned short)(y - x);
    asm volatile("" : "=r"(base) : "0"(base));
    {
        register int h asm("$3");
        int diff;
        h = (int)D_800BCF8E;
        asm volatile("" : "=r"(h) : "0"(h));
        y = *(unsigned short *)(base + 0x2E);
        asm volatile("" : "=r"(y) : "0"(y));
        x = *(int *)(base + 0x14);
        asm volatile("" : "=r"(x) : "0"(x));
        h = h - 0x70;
        asm volatile("" : "=r"(h) : "0"(h));
        diff = y - h;
        asm volatile("" : "=r"(diff) : "0"(diff));
        x = (int)base + x;
        asm volatile("" : "=r"(x) : "0"(x));
        *(unsigned short *)(base + 0x3A) = (unsigned short)diff;
    }
    *out = r2;
    if (n != 0) {
        neg = -0x8000;
        r2 = 0x7FFF;
        base = (unsigned char *)x;
        do {
            if (func_80066F60(base, *out, out) != 0) {
                return -0x12;
            }
            *(short *)(base + 0x10) = (short)neg;
            *(short *)(base + 0x12) = (short)r2;
            *(short *)(base + 0x14) = (short)neg;
            *(short *)(base + 0x16) = (short)r2;
            asm volatile("" : "=r"(base) : "0"(base));
            r1 = r1 + 1;
            base = base + 0x38;
        } while (r1 < n);
    }
    flags = &D_800BCF88;
    D_800BCFFB = 0;
    D_800BCFFA = 0;
    *flags = *flags & ~0xC00u;
    return 0;
}
