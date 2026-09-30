extern unsigned char *D_8009D2F0;
extern int *D_8009D300;
extern int D_8009CE00;

int func_800133E8(void)
{
    register unsigned char *o asm("$4");
    int *p;
    int w;
    register int sh asm("$6");
    int lim;
    register int b asm("$5");
    int hi;
    int divs;
    int scaled;
    int bshift;

    o = D_8009D2F0;
    p = D_8009D300;
    w = *(int *)(o + 0x14);
    sh = *(unsigned short *)(o + 0x12);
    p[4] = 1;
    if ((w >> 16) == sh) {
        return 0;
    }
    lim = sh << 16;
    {
        register int extra asm("$2");
        extra = *(int *)(o + 0x1C);
        o = (unsigned char *)(w + extra);
    }
    if (w < lim && lim < (int)o) {
        return 0;
    }
    if (lim < w && (int)o < lim) {
        return 0;
    }
    b = *(unsigned char *)(D_8009D2F0 + 0xF);
    hi = (int)o >> 16;
    divs = b + 1;
    if (b < hi) {
        hi = hi % divs;
        o = (unsigned char *)(hi << 16);
        if (lim < (int)o) {
            return 0;
        }
    } else if ((int)o < 0) {
        bshift = b << 16;
        hi = divs + (hi % divs);
        o = (unsigned char *)(hi << 16);
        asm volatile("" : "=r"(o), "=r"(bshift) : "0"(o), "1"(bshift));
        if (bshift < lim) {
            goto dec;
        }
        if ((int)o < lim) {
            return 0;
        }
    }
dec:
    D_8009CE00 -= 8;
    return 0;
}
