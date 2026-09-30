typedef struct Model {
    unsigned char pad0[2];
    unsigned char nParts;
    unsigned char pad3[3];
    unsigned short nVerts;
    unsigned short n4a;
    unsigned short n4b;
    unsigned short n3a;
    unsigned short n3b;
    unsigned char pad10[4];
    short f14;
    unsigned char pad16[2];
    unsigned short f18;
    unsigned short f1A;
} Model;

typedef struct Obj {
    Model *f0;
    unsigned char *f4;
    unsigned char *f8;
    unsigned char *fC;
    unsigned char *f10;
    unsigned char *f14;
    unsigned char *f18;
    unsigned char *f1C;
    unsigned char *f20;
    int f24;
    short f28;
    short f2A;
    short f2C;
    short f2E;
    short f30;
    short f32;
    unsigned char f34[0x54 - 0x34];
    unsigned char *f54;
    unsigned char pad58[0x6E - 0x58];
    short f6E;
    unsigned short f70;
    unsigned short f72;
    unsigned char pad74[0x80 - 0x74];
    unsigned char *f80;
    unsigned char *f84;
    unsigned char pad88[0x8C - 0x88];
    signed char f8C;
    unsigned char pad8D[3];
    unsigned char f90;
    unsigned char f91;
    unsigned char f92;
    unsigned char pad93[0x9C - 0x93];
    short f9C;
    unsigned char f9E;
    unsigned char f9F;
    unsigned char padA0[6];
    unsigned char fA6[2][8];
    unsigned char padB6[4];
    short fBA;
    unsigned char padBC[4];
} Obj;

extern int D_8009CDDC;
extern void func_8003D94C(Obj *, short, short, short, short);
extern void func_800794C4(short *, void *);
extern void func_8003C5D8(Obj *, int);

int func_8003D050(Obj *o, unsigned char *c, unsigned char *p0, short a3, short a4, short a5, short a6, short a7,
                  unsigned char **out, int flag)
{
    Model *m;
    unsigned char *p;
    unsigned char *f;
    unsigned char *q;
    unsigned char *r;
    unsigned char *s;
    short i;
    short j;
    short n;

    i = 0;
    o->f0 = (Model *)c;
    m = (Model *)c;
    c += 0x1C;
    o->f4 = c;
    c += m->nParts * 12;
    o->f8 = c;
    c += m->nVerts * 8;
    o->fC = c;
    n = 0;
    c += m->nVerts * 4;
    p = p0;
    o->f54 = p;
    o->fBA = flag;
    o->f10 = c;
    f = c;
    if (flag != 0) {
        for (i = 0; i < m->n4a; i++, f += 12) {
            p[3] = 0xC;
            p[7] = 0x3C;
            if (f[3] == 0xB) {
                p[7] = 0x3E;
            }
            p += 0x34;
            p[3] = 0xC;
            p[7] = 0x3C;
            if (f[3] == 0xB) {
                p[7] = 0x3E;
            }
            p += 0x34;
        }
        for (i = 0; i < m->n4b; i++, f += 12) {
            p[3] = 9;
            p[7] = 0x34;
            if (f[3] == 0x10) {
                p[7] = 0x36;
            }
            p += 0x28;
            p[3] = 9;
            p[7] = 0x34;
            if (f[3] == 0x10) {
                p[7] = 0x36;
            }
            p += 0x28;
        }
        for (i = 0; i < m->n3a; i++, f += 12) {
            p[3] = 8;
            p[7] = 0x38;
            if (f[3] == 0x15) {
                p[7] = 0x3A;
            }
            p += 0x24;
            p[3] = 8;
            p[7] = 0x38;
            if (f[3] == 0x15) {
                p[7] = 0x3A;
            }
            p += 0x24;
        }
        for (i = 0; i < m->n3b; i++, f += 12) {
            p[3] = 6;
            p[7] = 0x30;
            if (f[3] == 0x1A) {
                p[7] = 0x32;
            }
            p += 0x1C;
            p[3] = 6;
            p[7] = 0x30;
            if (f[3] == 0x1A) {
                p[7] = 0x32;
            }
            p += 0x1C;
        }
    } else {
        f = c + (m->n4a + m->n4b + m->n3a + m->n3b) * 12;
    }
    c = f;
    o->f14 = c;
    c += 0x10;
    o->f18 = c;
    c += m->nParts * 16;
    o->f1C = c;
    c += 8;
    o->f20 = c;
    {
        int k = m->f18 >> 2;
        short r = m->f18 & 3;
        if (r > 0) {
            c += (k + 1) * 4;
        } else {
            c += k * 4;
        }
    }
    if (flag != 0) {
        *out = c;
        q = o->f54;
        for (i = 0; i < m->n4a; i++) {
            s = c;
            for (j = 0; j < 2; j++) {
                q[0xC] = s[0];
                q[0xD] = s[1];
                q[0x18] = s[4];
                q[0x19] = s[5];
                q[0x24] = s[8];
                q[0x25] = s[9];
                q[0x30] = s[0xC];
                q[0x31] = s[0xD];
                *(unsigned short *)(q + 0xE) = *(unsigned short *)(s + 2);
                *(unsigned short *)(q + 0x1A) = *(unsigned short *)(s + 6);
                q += 0x34;
            }
            c += 0x10;
        }
        for (i = 0; i < m->n4b; i++) {
            s = c;
            for (j = 0; j < 2; j++) {
                q[0xC] = s[0];
                q[0xD] = s[1];
                q[0x18] = s[4];
                q[0x19] = s[5];
                q[0x24] = s[8];
                q[0x25] = s[9];
                *(unsigned short *)(q + 0xE) = *(unsigned short *)(s + 2);
                *(unsigned short *)(q + 0x1A) = *(unsigned short *)(s + 6);
                q += 0x28;
            }
            c += 0xC;
        }
        for (i = 0; i < m->n3a; i++) {
        }
        for (i = 0; i < m->n3b; i++) {
        }
        asm("" : : "r"(a6));
        if (a7 > 0) {
            func_8003D94C(o, a3, a4, a5, a6);
        }
    }
    o->f70 = *(unsigned short *)(o->f14 + 6);
    o->f72 = *(unsigned short *)(o->f1C + 6);
    r = p;
    o->f80 = r;
    for (i = 0; i < o->f0->nParts; i++) {
        if (o->f4[i * 12 + 4] == 1) {
            unsigned char *b = o->f18 + i * 16;
            if (*(short *)(b + 0xE) >= 0) {
                *(unsigned short *)(r + 8) = *(unsigned short *)(b + 6);
                *(unsigned short *)(r + 10) = *(unsigned short *)(b + 0xE);
                *(short *)(r + 6) = i;
                r += 0xC;
            }
        } else {
            n++;
        }
    }
    o->f84 = r;
    for (i = 0; i < o->f0->nParts; i++, r += 0x20) {
        *(short *)(r + 0) = 0x1000;
        *(short *)(r + 2) = 0;
        *(short *)(r + 4) = 0;
        *(short *)(r + 6) = 0;
        *(short *)(r + 8) = 0x1000;
        *(short *)(r + 0xA) = 0;
        *(short *)(r + 0xC) = 0;
        *(short *)(r + 0xE) = 0;
        *(short *)(r + 0x10) = 0x1000;
        *(int *)(r + 0x14) = 0;
        *(int *)(r + 0x18) = 0;
        *(int *)(r + 0x1C) = 0;
    }
    o->f0->f14 = 0;
    o->f2C = 0;
    o->f2E = 0;
    o->f30 = 0;
    o->f32 = 1;
    o->f2C = 0;
    o->f2E = 0;
    o->f30 = 0;
    func_800794C4(&o->f2C, o->f34);
    o->f8C = -1;
    func_8003C5D8(o, 0x32);
    o->f90 = 0x80;
    o->f91 = 0x80;
    o->f92 = 0x80;
    o->f9E = 1;
    o->f9C = 0;
    o->f28 = 0;
    o->f24 = 0;
    o->f2A = 0;
    o->f9F = D_8009CDDC;
    o->f0->f1A = o->f0->nVerts - n;
    o->f6E = *(unsigned short *)(o->f1C + 2) + ((short)o->f72 >> 4);
    for (i = 0; i < 2; i++) {
        o->fA6[i][0] = 0;
        o->fA6[i][1] = 0;
    }
    *(int *)((char *)o + 0xB0) = 0;
    return 1;
}
