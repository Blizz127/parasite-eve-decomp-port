typedef struct {
    unsigned int f0, f4, f8, fC;
} Hdr;

extern unsigned char *D_8009D2C8;
extern unsigned int D_8009D2C4;
extern unsigned int D_8009D2DC;
extern unsigned int D_8009CDE8;
extern unsigned int D_800BCD50;
extern unsigned int D_800BCD5C;
extern unsigned char D_800BA560[];
extern unsigned char D_800B8AC0[];
extern unsigned char D_8009B8F4[];
extern unsigned int func_80089FE0(unsigned char *a0, unsigned int a1);
extern void func_8008F178(unsigned char *a0, int a1);
extern void func_80089960(void);
extern void func_80089B28(void);
extern void func_80089CF0(void);

#define W(p, o) (*(unsigned int *)((p) + (o)))
#define H(p, o) (*(unsigned short *)((p) + (o)))

void func_8008A068(unsigned int *a0)
{
    unsigned char *q;
    unsigned char *c;
    unsigned char *v;
    unsigned int mask;
    unsigned int rem;
    unsigned int bit;
    unsigned int r;
    unsigned int f;
    unsigned char *c2;

    q = (unsigned char *)a0;
    W(D_8009D2C8, 0x2C) = (unsigned int)a0;
    mask = *a0 & 0xFFFFFF;
    r = func_80089FE0(D_800BA560, W(D_8009D2C8, 0x6C));
    { unsigned int t = ~D_800BCD50 & 0xFFFFFF; __asm__("" : "=r"(t) : "0"(t)); D_800BCD5C |= ~r & t; }
    c = D_8009D2C8;
    f = D_8009D2DC & 1;
    W(c, 0x18) = 0;
    if (f) {
        W(c, 0x4) = 0;
        W(c, 0x1C) |= mask;
    } else {
        W(c, 0x1C) = 0;
        W(c, 0x4) |= mask;
    }
    q += 4;
    { register unsigned char *c3 asm("$4");
    c3 = D_8009D2C8;
    ((Hdr *)c3)->f8 = *(unsigned int *)q & 0xFFFFFF;
    q += 4;
    ((Hdr *)c3)->fC = *(unsigned int *)q & 0xFFFFFF;
    q += 8;
    ((Hdr *)c3)->f0 &= ~0x101;
    ((Hdr *)c3)->f0 |= D_8009CDE8 & 0x100;
    }
    bit = 1;
    v = D_800B8AC0;
    rem = 0xFFFFFF;
    do {
        if (mask & bit) {
            r = *(unsigned short *)q;
            q += 2;
            *(unsigned char **)v = q + r;
            W(v, 0xF0) = 0x18;
            H(v, 0x56) = 4;
            H(v, 0x58) = 2;
            H(v, 0x6C) = 0x7F00;
            W(v, 0x44) = 0x3FFF0000;
            H(v, 0xD8) = 0x4000;
            H(v, 0xE0) = 0;
            H(v, 0xDE) = 0;
            H(v, 0x82) = 0;
            W(v, 0x34) = 0;
            H(v, 0xE4) = 0;
            H(v, 0x7A) = 0;
            H(v, 0xD2) = 0;
            H(v, 0xD0) = 0;
            H(v, 0x76) = 0x8000;
            H(v, 0x78) = 0;
            H(v, 0x82) = 0;
            H(v, 0x74) = 0;
            H(v, 0x72) = 0;
            *(unsigned int **)(v + 0x14) = a0;
            H(v, 0x84) = 0;
            H(v, 0xEC) = 0;
            W(v, 0x38) = 0;
            H(v, 0xCE) = 0;
            H(v, 0xB4) = 0;
            H(v, 0xA6) = 0;
            H(v, 0x94) = 0;
            H(v, 0xB6) = 0;
            H(v, 0xA8) = 0;
            H(v, 0x96) = 0;
            H(v, 0xBC) = 0;
            H(v, 0xBA) = 0;
            func_8008F178(v, 0);
        } else {
            H(v, 0x56) = 3;
            H(v, 0x58) = 1;
            *(unsigned char **)v = D_8009B8F4;
            W(v, 0xF4) |= 0x4400;
            H(v, 0x116) = 5;
        }
        rem &= ~bit;
        mask &= ~bit;
        v += 0x11C;
        bit <<= 1;
    } while (rem != 0);
    c2 = D_8009D2C8;
    W(c2, 0x20) = 0xFFFF0000;
    W(c2, 0x28) = 1;
    H(c2, 0x52) = 0;
    W(c2, 0x40) = 0;
    H(c2, 0x58) = 0;
    W(c2, 0x44) = 0;
    D_8009D2C4 = 0;
    H(c2, 0x62) = 0;
    H(c2, 0x60) = 0;
    H(c2, 0x5E) = 0;
    H(c2, 0x64) = 0;
    W(c2, 0x34) = 0;
    W(c2, 0x38) = 0;
    W(c2, 0x3C) = 0;
    H(c2, 0x56) = 0;
    W(c2, 0x10) = 0;
    W(c2, 0x14) = 0xFFFFFF;
    W(c2, 0x30) = 0;
    func_80089960();
    func_80089B28();
    func_80089CF0();
}
