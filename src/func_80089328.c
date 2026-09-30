typedef struct {
    unsigned int f0;
    unsigned int f4;
    unsigned int f8;
    unsigned int fC;
    unsigned int f10;
    unsigned int f14;
    unsigned char pad18[0x2A];
    short f42;
    unsigned char pad44[0x16];
    unsigned short f5A;
    unsigned char pad5C[0xC];
} Ctl;

extern unsigned int D_800BCD50;
extern unsigned int D_800BCD54;
extern unsigned int D_800BCD58;
extern unsigned int D_800BCD60;
extern unsigned short D_800BCD78;
extern Ctl *D_8009D2C8;
extern unsigned int D_8009D2C4;
extern unsigned char D_800BA560[];
extern unsigned char D_800B8AC0[];
extern unsigned char D_800BC000[];
extern int D_800C0DD0, D_800C0DD4, D_800C0DD8;

extern void func_80089250(unsigned int);
extern void func_8008900C(unsigned char *, unsigned int, unsigned int, unsigned int *);
extern void func_80088980(unsigned char *, unsigned int);
extern void func_800878F0(int, unsigned char *, int);
extern void func_80089F28(int, int);
extern void func_80089EB8(int);
extern void func_80089B48(void);
extern void func_80089980(void);
extern void func_80089D10(void);
extern void func_80087744(int);
extern void func_80087760(int);
extern void func_8008777C(int);
extern void func_8008770C(unsigned int);

void func_80089328(void)
{
    unsigned int flags;
    unsigned int *p;
    unsigned int m;
    unsigned int a, c, d;
    unsigned int b;
    unsigned int bit;
    unsigned char *v;
    Ctl *t, *u, *w, *x;

    flags = 0;
    p = &D_800BCD50;
    m = *p | D_800BCD60;
    if ((D_8009D2C8[0].f4 & D_8009D2C8[0].f10) | (D_8009D2C8[1].f4 & D_8009D2C8[1].f10)) {
        func_80089250(m);
    }
    t = D_8009D2C8;
    a = t[1].f4 & t[1].f14 & ~(t[1].fC & m);
    b = a & t[1].fC & ~m;
    if (a & t[1].f8) {
        D_8009D2C8 = t + 1;
        func_8008900C(D_800BA560, a & t[1].f8, b, &flags);
        u = D_8009D2C8;
        a &= ~u->f8;
        D_8009D2C8 = u - 1;
        u->f10 &= ~u->f8;
    }
    w = D_8009D2C8;
    asm("" : : "r"(b));
    asm("" : : "r"(b));
    asm("" : : "r"(b));
    c = w->f4 & w->f14 & ~(w->fC & (b | m));
    d = c & w->fC & ~(b | m);
    if (c & w->f8) {
        func_8008900C(D_800B8AC0, c & w->f8, d, &flags);
        x = D_8009D2C8;
        c &= ~x->f8;
        x->f10 &= ~*(volatile unsigned int *)&x->f8;
    }
    if (a != 0) {
        D_8009D2C8 = D_8009D2C8 + 1;
        func_8008900C(D_800BA560, a, b & ~d, &flags);
        D_8009D2C8->f10 = 0;
        D_8009D2C8 = D_8009D2C8 - 1;
    }
    if (c != 0) {
        func_8008900C(D_800B8AC0, c, d, &flags);
        D_8009D2C8->f10 = 0;
    }
    c = *p & D_800BCD58;
    if (c != 0) {
        bit = 0x1000;
        v = D_800BC000;
        flags |= D_800BCD54;
        do {
            if (c & bit) {
                func_80088980(v, bit);
                if (*(int *)(v + 0xF4) != 0) {
                    func_800878F0(*(int *)(v + 0xF0), v + 0xF0, *(int *)(v + 0x38));
                }
                c &= ~bit;
            }
            asm("" : : "r"(v));
            bit <<= 1;
            v += 0x11C;
        } while (c != 0);
        D_800BCD54 = 0;
    }
    c = D_8009D2C4;
    if (c & 0x80) {
        func_80089F28(D_8009D2C8->f42, D_8009D2C8->f42);
        D_8009D2C4 &= ~0x80;
    }
    if (c & 0x10) {
        if (D_800BCD50 != 0) {
            func_80089EB8(D_800BCD78);
        } else {
            func_80089EB8(D_8009D2C8->f5A);
        }
        D_8009D2C4 &= ~0x10;
    }
    if (c & 0x100) {
        func_80089B48();
        func_80089980();
        func_80089D10();
        func_80087744(D_800C0DD0);
        func_80087760(D_800C0DD4);
        func_8008777C(D_800C0DD8);
        D_8009D2C4 &= ~0x100;
    }
    if (flags != 0) {
        func_8008770C(flags);
    }
}
