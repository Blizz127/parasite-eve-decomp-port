extern unsigned char *volatile D_800B1624;

int func_80065AD4(unsigned int a0, unsigned int a1, unsigned int a2)
{
    unsigned char *p = D_800B1624;
    unsigned char *q = D_800B1624;
    unsigned char *r;
    unsigned char *s;
    short h;
    register unsigned int w asm("$3");
    register unsigned int b asm("$2");
    register unsigned int sh asm("$2");
    register unsigned int off asm("$4");

    q += *(unsigned int *)(p + 0x10);
    r = q + (a0 << 4);
    w = r[4];
    off = *(unsigned int *)(r + 0xC);
    sh = a1 << 16;
    *(unsigned short *)(r + 0xA) = 0;
    w = w | sh;
    b = r[0];
    s = r + off;
    *(unsigned int *)(r + 4) = w;
    b = b | 6;
    *r = b;
    *(signed char *)((a2 << 1) + (unsigned int)s + 1) = -1;
    h = *(short *)(r + 8);
    if ((h > 0 && a2 < a1) || (h < 0 && a1 < a2)) {
        *(unsigned short *)(r + 8) = -*(unsigned short *)(r + 8);
    }
    return 0;
}
