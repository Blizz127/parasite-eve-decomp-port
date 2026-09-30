extern unsigned int D_8009D1A0;
extern unsigned char *volatile D_800B1624;
extern short D_800BCF8C;
extern unsigned int D_800BCF88;
extern unsigned short D_800BCF8E;
extern short D_800BCF90;
extern short D_800BCF92;

int func_80067E1C(void)
{
    char pad[8];
    register unsigned char *p0 asm("$2");
    register unsigned char *base asm("$4");
    register unsigned char *v asm("$2");
    register unsigned char *e asm("$8");
    register unsigned char *g asm("$10");
    unsigned int *f;
    unsigned int s;
    register int i asm("$9");
    register int n asm("$11");
    register int w asm("$7");
    register int w2 asm("$7");
    register int m1 asm("$6");
    register int m2 asm("$3");
    register int lo1 asm("$5");
    register int t asm("$12");
    register int hi asm("$2");
    register int lo asm("$3");
    register int ad asm("$4");

    if ((D_8009D1A0 & 0x104) != 0) {
        return 0;
    }
    i = 0;
    p0 = D_800B1624;
    base = D_800B1624;
    n = *(unsigned short *)(p0 + 6);
    v = base + *(unsigned int *)(p0 + 0x14);
    if (n != 0) {
        g = (unsigned char *)&D_800BCF8C;
        e = v;
        do {
            if ((*e & 4) != 0) {
                hi = *(short *)(e + 0xC);
                lo = e[0x20];
                ad = *(short *)(e + 0x1C);
                w = (hi << 8) | lo;
                w = w + ad;
                m1 = (w >> 8) % *(unsigned short *)(e + 4);
                lo1 = w & 0xFF;
                ad = *(short *)(e + 0x1E);
                hi = *(short *)(e + 0xE);
                lo = e[0x22];
                w2 = (hi << 8) | lo;
                w2 = w2 + ad;
                m2 = (w2 >> 8) % *(unsigned short *)(e + 6);
                *(short *)(e + 0x20) = lo1;
                hi = w2 & 0xFF;
                *(short *)(e + 0x22) = hi;
                *(short *)(e + 0xC) = m1;
                *(short *)(e + 0xE) = m2;
            }
            if ((*e & 8) != 0) {
                hi = *(short *)g;
                lo = *(short *)(g + 0x9C);
                ad = *(short *)(e + 0x1C);
                hi = hi - lo;
                t = hi * ad;
                hi = *(short *)(e + 8);
                w = (hi << 8) + t;
                hi = w >> 8;
                *(short *)(e + 0xC) = hi;
                hi = w & 0xFF;
                *(short *)(e + 0x20) = hi;
                hi = *(short *)(g + 2);
                lo = *(short *)(g + 0x9E);
                ad = *(short *)(e + 0x1E);
                hi = hi - lo;
                t = hi * ad;
                hi = *(short *)(e + 0xA);
                w = (hi << 8) + t;
                hi = w >> 8;
                *(short *)(e + 0xE) = hi;
                hi = w & 0xFF;
                *(short *)(e + 0x22) = hi;
            }
            i++;
            e += 0x38;
        } while (i < n);
    }
    f = &D_800BCF88;
    s = *f;
    if ((s & 0x80) != 0) {
        *f = s & ~0x80;
        D_800BCF90 = D_800BCF8C;
        D_800BCF92 = D_800BCF8E;
    }
    return 0;
}
