typedef struct {
    unsigned char pad0[4];
    unsigned int f4;
} Arm;

typedef struct {
    unsigned char f0;
} Act;

typedef struct {
    unsigned int f0;
    unsigned char pad4[0xC];
    int f10;
    unsigned char pad14[4];
    Act *f18;
    unsigned char pad1C[0x76];
    unsigned char f92;
    unsigned char f93;
    unsigned char pad94[8];
    unsigned short f9C;
    unsigned char pad9E[0x18];
    unsigned short fB6[3];
} Body;

typedef struct {
    unsigned char pad0[8];
    int f8;
    short fC;
    short fE;
    unsigned short f10;
    unsigned char f12;
    unsigned char pad13[9];
    short f1C;
    unsigned char pad1E[6];
    unsigned short f24;
    unsigned char pad26[2];
    int f28;
    int f2C;
    int f30;
    int f34;
    unsigned char pad38[0x10];
    signed char f48;
    unsigned char f49;
    short f4A;
    unsigned int f4C;
    short f50;
    unsigned short f52;
    unsigned short f54;
    unsigned char f56;
    unsigned char f57;
    short f58;
    unsigned short f5A;
    unsigned short f5C;
    unsigned char f5E;
    unsigned char f5F;
    unsigned char f60[6];
    unsigned char f66;
    unsigned char pad67[5];
    Arm *f6C;
} St;

typedef union {
    int w;
    struct {
        short lo;
        short hi;
    } h;
} Fix;

typedef struct Ent {
    Body *f0;
    struct Ent *next;
    unsigned char pad8[6];
    unsigned char fE;
    unsigned char padF[0x19];
    Fix f28;
    Fix f2C;
    Fix f30;
    unsigned char pad34[0xC];
    int f40;
    unsigned char pad44[4];
    int f48;
    unsigned char pad4C[0x1C];
    int f68;
    int f6C;
    int f70;
    unsigned char pad74[0x24];
    unsigned int f98;
    unsigned char pad9C[0x174];
    unsigned short f210;
    unsigned short f212;
    unsigned char pad214[0x54];
    short f268;
    short f26A;
    short f26C;
} Ent;

typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char r1, g1, b1, p1;
    short x1, y1;
    unsigned char r2, g2, b2, p2;
    short x2, y2;
    unsigned char r3, g3, b3, p3;
    short x3, y3;
} PolyG4;

typedef struct {
    PolyG4 a;
    PolyG4 b;
} G4Pair;

typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    unsigned char pad8[0x14];
} Prim28;

extern St *D_8009D278;
extern Ent *D_8009D254;
extern Ent *D_8009D20C;
extern unsigned int D_8009D1A0;
extern unsigned int D_8009D2E8;
extern int D_800B0E08[];
extern unsigned int D_8009D1E8;
extern int D_8009CDDC;
extern int D_8009CDDC_b asm("D_8009CDDC");
extern PolyG4 D_800B00E8[];
extern G4Pair D_800B0130[];
extern Prim28 D_800B6928[];
extern signed char D_8009CE30;
extern unsigned char D_8009CE34[1];
extern unsigned char D_8009CE34s asm("D_8009CE34");
extern unsigned char D_8009D234;
extern unsigned char D_8009D1D4;
extern int D_8009D200;
extern int D_8009D2FC;
extern int D_8009D28C;
extern unsigned int D_8009D1AC;
extern unsigned char D_8009D1CE;
extern unsigned char D_8009D244;
extern unsigned int D_800BCF88;

extern void func_8006DF50(int, int, int, int, int);
extern signed char func_80021054(void);
extern void func_8001A680(Ent *, int);
extern void func_8006DCE4(int, int, int, int, int);
extern void func_8001F4D4(Ent *);
extern short func_8001F814(Ent *);
extern int func_80077CF4(int);
extern int func_80077DC4(int);
extern void func_8001F9C4(void);
extern void func_80032B0C(int, void *);
extern void func_8006F6D4(int, int, int, int, int, int);
extern void func_80021D4C(void);
extern void func_80020CE4(void);
extern void func_800374E8(void);
extern void func_8006DE80(int, int, int, int, int);
extern void func_80062F9C(void);
extern void func_80067CBC(void);

#define RGB(p, n, _r, _g, _b) (p).r##n = (_r), (p).g##n = (_g), (p).b##n = (_b)
#define GA D_800B0130[D_8009CDDC].a
#define GB D_800B0130[D_8009CDDC].b

static inline void dmg_num(void)
{
    if (D_8009D278->fC <= D_8009D278->f1C) {
        D_8009D278->f50 = D_8009D278->fE - D_8009D278->fC;
        D_8009D278->f52 = D_8009D254->f210;
        D_8009D278->f54 = D_8009D254->f212;
        D_8009D278->f56 = 30;
        if (D_8009D278->f50 == 0) {
            D_8009D278->f57 = 0;
        } else {
            D_8009D278->f57 = (D_8009D278->f4C >> 14) & 2;
        }
        D_8009D278->f4C &= ~0x8000;
    }
}

static inline void stop_motion(void)
{
    D_8009D254->f68 = 0;
    D_8009D254->f6C = 0;
    D_8009D254->f70 = 0;
}

static inline void cancel_attacks(void)
{
    if (func_80021054() != D_8009D1D4) {
        if (D_8009D200 != -1) {
            func_8006F6D4(D_8009D200, 0, 0, 2, 0, 0);
            D_8009D200 = -1;
        }
        if (D_8009D2FC != -1) {
            func_8006F6D4(D_8009D2FC, 0, 0, 2, 0, 0);
            D_8009D2FC = -1;
        }
    }
    func_80021D4C();
}

typedef struct { unsigned int v; } W1;
#define FLS (((W1 *)fl)->v)
extern unsigned int D_8009D1AC_a[1] asm("D_8009D1AC");
void func_8001D340(unsigned char mode)
{
    char pad[0x2C0];
    unsigned int *fl;
    St *r;
    Ent *e;
    Body *b;
    int k;

    register St *r0 asm("$4") = D_8009D278;
    fl = &r0->f4C;
    if (mode != 0) {
        if (!(D_8009D1A0 & 0x100)) {
            unsigned short at;

            r0->f10 = at = r0->f10 + r0->f24;
            if ((r0->f4C & 0xC0) == 0x40 || (r0->f4C & 0xC0) == 0x80) {
                r0->f10 = at - (r0->f24 * 2) / 5;
            } else if (r0->f4C & 0x100) {
                r0->f10 = at + ((unsigned int)r0->f24 >> 1);
            }
            r = D_8009D278;
            if (r->f8 < r->f28 && !(*fl & 0x2600)) {
                r->f30 -= 3;
                if (r->f30 <= 0) {
                    r->f30 = 1;
                }
                r->f2C -= r->f28 / (r->f30 * 100);
                if (r->f2C < 0x1999) {
                    r->f2C = 0x1999;
                }
                D_8009D278->f8 += r->f2C;
                if (D_8009D278->f8 >= r->f28) {
                    r->f34 = 0xF0;
                    if (D_800B0E08[0] != 0) {
                        func_8006DF50(D_800B0E08[0], 0x455, 0, 0x80, 0x7F);
                    }
                }
            }
        }
        if (mode == 1) {
            if ((D_8009D254->fE == D_8009D278->f12 || D_8009D254->fE == 5)
                && !((D_8009D278->f6C->f4 & 0x4000) && (D_8009D278->f4C & 0x4000)
                     && func_80021054() > 0)) {
                D_8009D2E8 &= ~1;
            } else if (D_8009D254->fE >= 14) {
                D_8009D2E8 &= ~1;
            } else {
                D_8009D2E8 |= 1;
            }
        }
    } else {
        D_8009D2E8 |= 1;
    }
    if (D_8009D278->f8 < 0) {
        D_8009D278->f8 = 0;
    }
    if (*fl & 0x2000) {
        if ((D_8009D1E8 & 3) == 0) {
            RGB(GB, 0, 0xFF, 0x3D, 0x81); RGB(GB, 1, 0x83, 0x13, 1);
            RGB(GB, 2, 0xFF, 0x3D, 0x81); RGB(GB, 3, 0x83, 0x13, 1);
        } else if ((D_8009D1E8 & 3) == 1) {
            RGB(GB, 0, 0xC1, 0x28, 0x41); RGB(GB, 1, 0xC1, 0x28, 0x41);
            RGB(GB, 2, 0xC1, 0x28, 0x41); RGB(GB, 3, 0xC1, 0x28, 0x41);
        } else if ((D_8009D1E8 & 3) == 2) {
            RGB(GB, 0, 0x83, 0x13, 1); RGB(GB, 1, 0xFF, 0x3D, 0x81);
            RGB(GB, 2, 0x83, 0x13, 1); RGB(GB, 3, 0xFF, 0x3D, 0x81);
        } else if ((D_8009D1E8 & 3) == 3) {
            RGB(GB, 0, 0xC1, 0x28, 0x41); RGB(GB, 1, 0xC1, 0x28, 0x41);
            RGB(GB, 2, 0xC1, 0x28, 0x41);
            GB.r3 = 0xC1; GB.g3 = 0x28; D_800B0130[D_8009CDDC_b].b.b3 = 0x41;
        }
        if (D_8009D254->fE != 0x12) {
            func_8001A680(D_8009D254, 0x12);
        }
        if (D_8009CE30 == 90) {
            func_8001A680(D_8009D254, D_8009D278->f12);
            D_8009CE30 = 0;
            FLS &= ~0x2000;
            D_8009D278->f8 = 0x10000;
            RGB(D_800B0130[0].b, 0, 0xFF, 0x3D, 0x81); RGB(D_800B0130[0].b, 1, 0x83, 0x13, 1);
            RGB(D_800B0130[0].b, 2, 0xFF, 0x3D, 0x81); RGB(D_800B0130[0].b, 3, 0x83, 0x13, 1);
            RGB(D_800B0130[1].b, 0, 0xFF, 0x3D, 0x81); RGB(D_800B0130[1].b, 1, 0x83, 0x13, 1);
            RGB(D_800B0130[1].b, 2, 0xFF, 0x3D, 0x81); RGB(D_800B0130[1].b, 3, 0x83, 0x13, 1);
        } else {
            D_8009CE30 = D_8009CE30 + 1;
            stop_motion();
        }
    }
    if (!(D_8009D1A0 & 0x100)) {
        if (D_8009D278->f34 > 0) {
            if (--D_8009D278->f34 != 0) {
                if ((D_8009D1E8 & 3) == 0) {
                    RGB(GA, 0, 0, 0x82, 0x36); RGB(GA, 1, 0x4A, 0xFF, 0x3B);
                    RGB(GA, 2, 0, 0x82, 0x36); RGB(GA, 3, 0x4A, 0xFF, 0x3B);
                } else if ((D_8009D1E8 & 3) == 1) {
                    RGB(GA, 0, 0x25, 0xC1, 0x39); RGB(GA, 1, 0x25, 0xC1, 0x39);
                    RGB(GA, 2, 0x25, 0xC1, 0x39); RGB(GA, 3, 0x25, 0xC1, 0x39);
                } else if ((D_8009D1E8 & 3) == 2) {
                    RGB(GA, 0, 0x4A, 0xFF, 0x3B); RGB(GA, 1, 0, 0x82, 0x36);
                    RGB(GA, 2, 0x4A, 0xFF, 0x3B); RGB(GA, 3, 0, 0x82, 0x36);
                } else if ((D_8009D1E8 & 3) == 3) {
                    RGB(GA, 0, 0x25, 0xC1, 0x39); RGB(GA, 1, 0x25, 0xC1, 0x39);
                    RGB(GA, 2, 0x25, 0xC1, 0x39);
                    GA.r3 = 0x25; GA.g3 = 0xC1; D_800B0130[D_8009CDDC_b].a.b3 = 0x39;
                }
                if (D_8009D278->f8 < D_8009D278->f28) {
                    D_8009D278->f34 = 0;
                }
                if (D_8009D278->f34 != 0) {
                    goto after_pe;
                }
            }
            RGB(D_800B0130[0].a, 0, 0, 0x82, 0x36); RGB(D_800B0130[0].a, 1, 0x4A, 0xFF, 0x3B);
            RGB(D_800B0130[0].a, 2, 0, 0x82, 0x36); RGB(D_800B0130[0].a, 3, 0x4A, 0xFF, 0x3B);
            RGB(D_800B0130[1].a, 0, 0, 0x82, 0x36); RGB(D_800B0130[1].a, 1, 0x4A, 0xFF, 0x3B);
            RGB(D_800B0130[1].a, 2, 0, 0x82, 0x36); RGB(D_800B0130[1].a, 3, 0x4A, 0xFF, 0x3B);
        }
    after_pe:
        if (*fl & 0x800000) {
            if ((signed char)--D_8009D234 <= 0) {
                *fl &= ~0x800000;
            }
        } else if (!(*fl & 0x180000)) {
            for (e = D_8009D20C; e != 0; e = e->next) {
                if (e == D_8009D254) {
                    continue;
                }
                if (e->f0 == 0) {
                    continue;
                }
                if (e->f98 & 0x10) {
                    continue;
                }
                b = e->f0;
                if (b->f18 != 0 && (*fl & 0x4000)) {
                    if ((int)b->f0 < 0 && (unsigned int)(b->f18->f0 - 1) < 2) {
                        k = (b->f0 >> 21) & 7;
                        if (k < 3) {
                            func_8006DCE4(b->fB6[k], 0, e->f268, e->f26A, e->f26C);
                        }
                        *fl |= 0x10000000;
                        func_8001F4D4(e);
                        stop_motion();
                        b->f0 &= 0x7FFFFFFF;
                        b->f9C++;
                        D_8009CE34s = 90;
                        FLS |= 0x1000000;
                    }
                    dmg_num();
                }
                if ((e->f98 & 0x2000000) && !(*fl & 0x1000000) && b->f10 > 0) {
                    if (!(*fl & 0x200)) {
                        D_8009D278->fC -= b->f92;
                    }
                    D_8009CE34[0] = 90;
                    FLS |= 0x1000000;
                    dmg_num();
                    stop_motion();
                    D_8009D254->f98 &= ~0xC0000;
                    D_8009D278->f4A = func_8001F814(e);
                    D_8009D278->f49 = b->f93;
                    D_8009D278->f48 = 6;
                }
            }
            if (D_8009D278->fC < D_8009D278->fE) {
                dmg_num();
                D_8009D278->fE = D_8009D278->fC;
            }
        }
        *fl &= ~0x4000;
        if (D_8009D278->f48 != 0) {
            if ((D_8009D254->f98 & 0xC0000) || D_8009D278->f49 == 0) {
                D_8009D254->f98 &= ~0xC0000;
                D_8009D278->f48 = 0;
                D_8009D278->f49 = 0;
            } else {
                D_8009D254->f28.w = D_8009D254->f40
                    + ((D_8009D278->f49 * func_80077CF4(D_8009D278->f4A)) << 4);
                D_8009D254->f30.w = D_8009D254->f48
                    + ((D_8009D278->f49 * func_80077DC4(D_8009D278->f4A)) << 4);
                D_8009D278->f49 = D_8009D278->f49 - D_8009D278->f49 / D_8009D278->f48;
                D_8009D278->f48--;
            }
        }
        func_8001F9C4();
    }
    if (D_8009D278->f56 != 0) {
        func_80032B0C(0, &D_8009D278->f50);
        D_8009D278->f56--;
    }
    if (D_8009D278->fE < D_8009D278->fC) {
        D_8009D278->f58 = D_8009D278->fC - D_8009D278->fE;
        D_8009D278->f5A = D_8009D254->f210;
        D_8009D278->f5C = D_8009D254->f212 - 8;
        D_8009D278->f5E = 30;
        D_8009D278->f5F = 1;
        D_8009D278->fE = D_8009D278->fC;
    }
    if (D_8009D278->f5E != 0) {
        func_80032B0C(0, &D_8009D278->f58);
        D_8009D278->f5E--;
    }
    if (D_8009D278->f66 != 0) {
        func_80032B0C(0, D_8009D278->f60);
        D_8009D278->f66--;
    }
    D_8009D278->fE = D_8009D278->fC;
    if (D_8009D278->f8 <= 0) {
        *fl |= 0x2000;
        cancel_attacks();
    }
    if (D_8009D28C == 1) {
        RGB(D_800B00E8[0], 0, 0, 0x46, 0x82); RGB(D_800B00E8[0], 1, 0x9F, 0xFF, 0xF9);
        RGB(D_800B00E8[0], 2, 0, 0x46, 0x82); RGB(D_800B00E8[0], 3, 0x9F, 0xFF, 0xF9);
        RGB(D_800B6928[0], 0, 0x9F, 0xFF, 0xF9);
        RGB(D_800B00E8[1], 0, 0, 0x46, 0x82); RGB(D_800B00E8[1], 1, 0x9F, 0xFF, 0xF9);
        RGB(D_800B00E8[1], 2, 0, 0x46, 0x82); RGB(D_800B00E8[1], 3, 0x9F, 0xFF, 0xF9);
        RGB(D_800B6928[1], 0, 0x9F, 0xFF, 0xF9);
        RGB(D_800B0130[0].a, 0, 0, 0x82, 0x36); RGB(D_800B0130[0].a, 1, 0x4A, 0xFF, 0x3B);
        RGB(D_800B0130[0].a, 2, 0, 0x82, 0x36); RGB(D_800B0130[0].a, 3, 0x4A, 0xFF, 0x3B);
        RGB(D_800B0130[1].a, 0, 0, 0x82, 0x36); RGB(D_800B0130[1].a, 1, 0x4A, 0xFF, 0x3B);
        RGB(D_800B0130[1].a, 2, 0, 0x82, 0x36); RGB(D_800B0130[1].a, 3, 0x4A, 0xFF, 0x3B);
    }
    if (D_8009D278->fC > 0) {
        return;
    }
    stop_motion();
    RGB(D_800B00E8[0], 0, 0, 0x46, 0x82); RGB(D_800B00E8[0], 1, 0x9F, 0xFF, 0xF9);
    RGB(D_800B00E8[0], 2, 0, 0x46, 0x82); RGB(D_800B00E8[0], 3, 0x9F, 0xFF, 0xF9);
    RGB(D_800B6928[0], 0, 0x9F, 0xFF, 0xF9);
    RGB(D_800B00E8[1], 0, 0, 0x46, 0x82); RGB(D_800B00E8[1], 1, 0x9F, 0xFF, 0xF9);
    RGB(D_800B00E8[1], 2, 0, 0x46, 0x82); RGB(D_800B00E8[1], 3, 0x9F, 0xFF, 0xF9);
    RGB(D_800B6928[1], 0, 0x9F, 0xFF, 0xF9);
    RGB(D_800B0130[0].a, 0, 0, 0x82, 0x36); RGB(D_800B0130[0].a, 1, 0x4A, 0xFF, 0x3B);
    RGB(D_800B0130[0].a, 2, 0, 0x82, 0x36); RGB(D_800B0130[0].a, 3, 0x4A, 0xFF, 0x3B);
    RGB(D_800B0130[1].a, 0, 0, 0x82, 0x36); RGB(D_800B0130[1].a, 1, 0x4A, 0xFF, 0x3B);
    RGB(D_800B0130[1].a, 2, 0, 0x82, 0x36); RGB(D_800B0130[1].a, 3, 0x4A, 0xFF, 0x3B);
    RGB(D_800B0130[0].b, 0, 0xFF, 0x3D, 0x81); RGB(D_800B0130[0].b, 1, 0x83, 0x13, 1);
    RGB(D_800B0130[0].b, 2, 0xFF, 0x3D, 0x81); RGB(D_800B0130[0].b, 3, 0x83, 0x13, 1);
    RGB(D_800B0130[1].b, 0, 0xFF, 0x3D, 0x81); RGB(D_800B0130[1].b, 1, 0x83, 0x13, 1);
    RGB(D_800B0130[1].b, 2, 0xFF, 0x3D, 0x81); RGB(D_800B0130[1].b, 3, 0x83, 0x13, 1);
    if (*fl & 0x10000) {
        func_80020CE4();
    }
    func_800374E8();
    cancel_attacks();
    D_8009D1AC_a[0] &= ~0x300;
    D_8009D28C = 3;
    D_8009D1CE = 0;
    { register Ent *p asm("$8") = D_8009D254; func_8006DE80(0x46B, 0, p->f28.h.hi, p->f2C.h.hi, p->f30.h.hi); }
    D_8009D244 = 0;
    func_80062F9C();
    if (D_800BCF88 & 0x2000) {
        func_80067CBC();
    }
    D_8009D1A0 &= ~4;
    D_8009D2E8 |= 1;
    func_8001A680(D_8009D254, 0x13);
    D_8009D278->fC = 0;
}
