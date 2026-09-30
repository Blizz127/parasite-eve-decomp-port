typedef struct Rec {
    unsigned int b0 : 13;
    unsigned int st : 2;
    unsigned int c3 : 3;
    unsigned int d2 : 2;
    unsigned int rest : 12;
    unsigned char pad4[0x10 - 4];
    int f10;
    unsigned char pad14[0x9F - 0x14];
    unsigned char f9F;
    unsigned char padA0[0xCC - 0xA0];
    unsigned int fCC;
    short fD0;
    short fD2;
    short fD4;
    unsigned char fD6;
} Rec;

typedef struct Ent {
    Rec *f0;
    struct Ent *next;
    unsigned char pad8[0x2A - 8];
    short f2A;
    unsigned char pad2C[0x32 - 0x2C];
    short f32;
    unsigned char pad34[0x3A - 0x34];
    unsigned short f3A;
    unsigned char pad3C[0x98 - 0x3C];
    int f98;
    unsigned char pad9C[0x218 - 0x9C];
    unsigned short f218;
    unsigned short f21A;
    unsigned char pad21C[0x268 - 0x21C];
    short f268;
    short f26A;
    short f26C;
} Ent;

typedef struct Mdl {
    unsigned char pad0[6];
    short f6;
    int f8;
    unsigned int fC;
    unsigned int f10;
} Mdl;

typedef struct St {
    unsigned char pad0[0x4C];
    unsigned int f4C;
    unsigned char pad50[0x68 - 0x50];
    Mdl *f68;
} St;

typedef struct Q {
    Ent *who;
    short cmd;
    short n;
} Q;

typedef struct Hit {
    signed char mode;
    signed char f1;
} Hit;

extern St *D_8009D278;
extern Ent *D_8009D254;
extern Ent *D_8009D20C;
extern Q D_800BE830[];
extern unsigned char D_8009D274;
extern unsigned char D_8009D1D4;
extern unsigned char D_8009D294;
extern Hit D_8009CE54;
extern unsigned char D_8009D1CE;
extern int D_8009D1F8;

extern void func_80021278(Ent *, Hit *, short);
extern void func_8006DE80(int, int, int, int, int);
extern int func_80071A54(void);
extern int func_80053D2C(int);
extern int func_80054A88(int, int);
extern int func_80079FB4(int, int);

static inline void apply(Ent *o)
{
    Rec *r = o->f0;

    if (D_8009CE54.mode == 1) {
        r->st = 1;
        r->d2 = D_8009D278->f68->fC >> 20;
        r->c3 = D_8009CE54.f1;
    } else if (D_8009CE54.mode == 0) {
        r->fD0 = -1;
        r->fD6 = 0x1E;
        r->fD2 = o->f218;
        r->fD4 = o->f21A - 0x14;
    }
}

void func_800236E8(void)
{
    Q *q;
    Mdl *m;
    Ent *o;
    Ent *e;
    short k;
    short c;
    short lo;
    short hi;

    q = &D_800BE830[D_8009D1D4];
    m = D_8009D278->f68;
    if (++D_8009D274 != m->f8) {
        return;
    }
    if (m->f6 == 8) {
        func_80021278(q->who, &D_8009CE54, 0);
        if (D_8009CE54.mode == 1) {
            func_8006DE80(0x46D, 0, q->who->f268, q->who->f26A, q->who->f26C);
            if (D_8009D278->f68->f10 & 0x6000) {
                switch ((q->who->f0->fCC >> 12) & 3) {
                case 0:
                    if (func_80071A54() & 1) {
                        break;
                    }
                case 2:
                    if (func_80053D2C(q->who->f0->f9F) != 0) {
                        D_8009D1CE = 1;
                        D_8009D1F8 = func_80054A88(q->who->f0->f9F, 2);
                    } else {
                        D_8009D1CE = 1;
                        D_8009D1F8 = func_80054A88(q->who->f0->f9F, 1);
                        q->who->f0->f9F = 0;
                    }
                    break;
                }
            }
        }
        apply(q->who);
    } else if (m->f6 == 6) {
        func_80021278(q->who, &D_8009CE54, 0);
        apply(q->who);
        k = D_8009CE54.f1;
        if (D_8009CE54.mode == 1) {
            for (o = D_8009D20C; o != 0; o = o->next) {
                if (o == D_8009D254) {
                    continue;
                }
                if (o->f0 == 0) {
                    continue;
                }
                if ((o->f98 & 0x2040) == 0x40) {
                    continue;
                }
                if (o->f98 & 0x4000) {
                    continue;
                }
                if (o->f0->f10 <= 0) {
                    continue;
                }
                if (o == q->who) {
                    continue;
                }
                func_80021278(o, &D_8009CE54, k);
                apply(o);
            }
        }
    } else if ((m->f10 & 0xC0) == 0x80 && !(D_8009D278->f4C & 0x100000)) {
        c = D_8009D254->f3A - 0x800;
        if (c < -0x600) {
            lo = c + 0x200;
            hi = c + 0xE00;
        } else if (c < 0x600) {
            lo = c - 0x200;
            hi = c + 0x200;
        } else {
            lo = c - 0xE00;
            hi = c - 0x200;
        }
        for (o = D_8009D20C; o != 0; o = o->next) {
            short v;

            e = D_8009D254;
            if (o == e) {
                continue;
            }
            if (o->f0 == 0) {
                continue;
            }
            if ((o->f98 & 0x2040) == 0x40) {
                continue;
            }
            if (o->f98 & 0x4000) {
                continue;
            }
            if (o->f0->f10 <= 0) {
                continue;
            }
            v = func_80079FB4(o->f268 - e->f2A, o->f26C - e->f32);
            if (c < -0x600) {
                if (v < hi && lo < v) {
                    continue;
                }
            } else if (c < 0x600) {
                if (v < lo || hi < v) {
                    continue;
                }
            } else {
                if (v < hi && lo < v) {
                    continue;
                }
            }
            func_80021278(o, &D_8009CE54, 0);
            apply(o);
        }
    } else {
        if (q->who == 0) {
            goto done;
        }
        func_80021278(q->who, &D_8009CE54, 0);
        apply(q->who);
    }
done:
    D_8009D294 = 0;
    D_8009D274 = 0;
}
