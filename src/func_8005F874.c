extern unsigned int *D_8009D100;
extern unsigned int D_8009D104;
extern int D_8009D10C;
extern int D_8009D110;
extern int D_8009D114;
extern unsigned short D_8009D124;
extern unsigned short D_8009D128;
extern int D_8009D13C;
extern int D_8009D140;
extern int D_8009D144;
extern unsigned int *D_8009D11C;
extern void func_800527C0(int);

void func_8005F874(int d)
{
    unsigned char *q;
    unsigned int *p;
    unsigned int *r;
    int x;
    int y;
    register int g asm("$3");
    register int sev asm("$2");
    unsigned char c;
    register unsigned char b asm("$5");
    unsigned char e;
    unsigned char f;
    register int w asm("$4");

    p = 0;
    if (D_8009D100 + 10 < (unsigned int *)(D_8009D104 + 0x4000)) {
        D_8009D100 = D_8009D100 + 10;
        p = D_8009D100 - 10;
    } else {
        func_800527C0(1);
    }
    if (p != 0) {
        if (D_8009D10C != 0) {
            p[1] = D_8009D114;
        } else {
            p[1] = D_8009D110;
        }
        *(unsigned char *)((char *)p + 3) = 9;
        *(unsigned char *)((char *)p + 7) = 0x2C;
    }
    q = (unsigned char *)p;
    x = D_8009D124;
    y = D_8009D128;
    *(short *)(q + 0x18) = x;
    *(short *)(q + 0x8) = x;
    x = *(volatile unsigned short *)(q + 0x8);
    *(short *)(q + 0x12) = y;
    *(short *)(q + 0xA) = y;
    y = *(volatile unsigned short *)(q + 0xA);
    x += 5;
    y += 7;
    *(short *)(q + 0x20) = x;
    *(short *)(q + 0x10) = x;
    *(short *)(q + 0x22) = y;
    *(short *)(q + 0x1A) = y;
    if (d >= 0) {
        q[0x1C] = D_8009D140 + (d % 10) * 5;
    } else {
        q[0x1C] = 0x58;
    }
    c = *(volatile unsigned char *)(q + 0x1C);
    q[0x1C] = c;
    q[0xC] = c;
    if (d >= 0) {
        q[0x15] = D_8009D144;
    } else {
        q[0x15] = 0xA4;
    }
    w = *(volatile int *)q;
    b = *(volatile unsigned char *)(q + 0x15);
    e = *(volatile unsigned char *)(q + 0xC);
    *(volatile unsigned char *)(q + 0xD) = b;
    f = *(volatile unsigned char *)(q + 0xD);
    *(volatile unsigned char *)(q + 0x15) = b;
    q[0x24] = e + 5;
    q[0x14] = e + 5;
    q[0x25] = f + 7;
    q[0x1D] = f + 7;
    g = D_8009D13C;
    sev = 7;
    *(short *)(q + 0xE) = g;
    r = D_8009D11C;
    *(short *)(q + 0x16) = sev;
    *(unsigned int *)q = (w & 0xFF000000) | (*r & 0xFFFFFF);
    *r = (*r & 0xFF000000) | ((unsigned int)q & 0xFFFFFF);
}
