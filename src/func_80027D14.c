typedef union Bits {
    unsigned int w;
    struct {
        unsigned int f0 : 1;
        unsigned int c1 : 3;
        unsigned int f4 : 1;
        unsigned int cnt : 5;
        unsigned int f10 : 1;
        unsigned int c2 : 2;
        unsigned int st : 2;
        unsigned int f15 : 9;
        unsigned int x : 6;
        unsigned int f30 : 2;
    } b;
} Bits;

typedef struct Rec {
    Bits w;
    unsigned char pad4;
    signed char f5;
    signed char f6;
    unsigned char pad7[0xC - 7];
    unsigned short fC;
    unsigned char padE[2];
    int f10;
    int f14;
    unsigned char *f18;
    unsigned char pad1C[0x88 - 0x1C];
    int f88;
    unsigned char pad8C[2];
    unsigned short f8E;
    unsigned char pad90[0x96 - 0x90];
    short f96;
    unsigned char pad98[0xA4 - 0x98];
    unsigned char fA4;
    unsigned char fA5;
    unsigned short fA6;
    short fA8;
    unsigned char padAA[0xB0 - 0xAA];
    unsigned short fB0;
    unsigned char padB2[0xBC - 0xB2];
    unsigned char fBC;
    unsigned char fBD;
    unsigned char fBE;
    unsigned char padBF;
    int fC0;
    int fC4;
    int fC8;
    unsigned char padCC[4];
    short fD0;
    short fD2;
    short fD4;
    unsigned char fD6;
    unsigned char fD7;
} Rec;

typedef struct Ent {
    Rec *f0;
    struct Ent *next;
    unsigned char pad8[0xF - 8];
    unsigned char fF;
    unsigned char pad10[4];
    int f14;
    int f18;
    int f1C;
    unsigned char pad20[0x28 - 0x20];
    int f28;
    unsigned char pad2C[4];
    int f30;
    unsigned char pad34[0x3A - 0x34];
    short f3A;
    unsigned char pad3C[0x68 - 0x3C];
    int f68;
    int f6C;
    int f70;
    unsigned char pad74[0x98 - 0x74];
    int f98;
    unsigned char pad9C[0x18C - 0x9C];
    struct Ent *f18C;
    unsigned char pad190[0x218 - 0x190];
    unsigned short f218;
    unsigned short f21A;
    unsigned char pad21C[0x250 - 0x21C];
    unsigned short f250;
    unsigned char pad252[0x268 - 0x252];
    short f268;
    short f26A;
    short f26C;
} Ent;

typedef struct St {
    unsigned char pad0[0x4C];
    unsigned int f4C;
} St;

extern unsigned int D_8009D1A0;
extern St *D_8009D278;
extern Ent *D_8009D20C;
extern Ent *D_8009D254;

extern void func_80036254(Ent *);
extern void func_80027A08(Ent *);
extern void func_80028574(Ent *);
extern void func_8006DCE4(int, int, int, int, int);
extern void func_8001A680(Ent *, unsigned short);
extern int func_80077CF4(int);
extern int func_80077DC4(int);
extern void func_80032B0C(int, short *);
extern void func_8002F970(Ent *);
extern void func_80028E94(Ent *);

void func_80027D14(Ent *e)
{
    register Rec *r asm("$17");
    Bits *b;
    Ent *o;
    register Ent *p asm("$5");
    int d;
    int k;
    unsigned int w;
    int v;

    r = e->f0;
    b = &r->w;
    if (r->f10 > r->f88) {
        r->f10 = r->f88;
    }
    if (!(D_8009D1A0 & 0x100)) {
        if (r->fC == 0) {
            if (r->w.b.c1 != 0) {
                r->w.b.c1--;
                if (!(r->w.w & 0x180E)) {
                    e->f98 &= ~0x1000;
                    func_80036254(e);
                    if (r->f18 != 0) {
                        *r->f18 = 4;
                        r->w.b.x = 0;
                    }
                }
            }
            if (b->b.c2 != 0) {
                b->b.c2--;
                if (!(b->w & 0x180E)) {
                    e->f98 &= ~0x1000;
                    func_80036254(e);
                    if (r->f18 != 0) {
                        *r->f18 = 4;
                        b->b.x = 0;
                    }
                }
            }
        }
        if (r->fC < 9000) {
            r->fC += r->f8E;
            if (b->w & 1) {
                r->fC -= r->f8E * 2 / 5;
            }
        } else {
            r->fC = 9000;
            if (e->f98 & 0x1000) {
                r->fC = 0;
            }
        }
        if (b->w & 0x10) {
            if (b->b.cnt >= 0x1E) {
                r->f10 -= r->f96;
                b->b.cnt = 0;
            } else {
                b->b.cnt++;
            }
        }
    }
    if (b->w & 0x6000) {
        func_80027A08(e);
        w = b->w;
        if ((w & 0x6000) == 0x2000) {
            func_80028574(e);
            func_8006DCE4(r->fB0, 0, e->f268, e->f26A, e->f26C);
        } else if ((w & 0x6000) == 0x4000) {
            if (r->f5 == 0 && !(w & 0xE)) {
                if (e->fF == *(unsigned short *)((char *)e + 0x1A)) {
                    if (r->fBC == 2 && !(r->w.w & 0x1800)) {
                        r->fBC--;
                        func_8001A680(e, r->fBD);
                        if (r->fBE != 0) {
                            e->f98 |= 0x200;
                        }
                        e->f14 = r->fC0;
                        e->f18 = r->fC4;
                        e->f1C = r->fC8;
                    } else {
                        func_8001A680(e, r->f6);
                    }
                    b->w &= ~0x6000;
                    if (!(D_8009D278->f4C & 0x80000) && !(r->w.w & 0x1800)) {
                        e->f98 &= ~0x1000;
                        func_80036254(e);
                    }
                } else if (r->fA4 >= 2 && r->fA5 != 0) {
                    e->f28 += (r->fA6 * func_80077CF4(r->fA8)) << 4;
                    e->f30 += (r->fA6 * func_80077DC4(r->fA8)) << 4;
                    r->fA5--;
                }
            } else if (r->f5 == 0 || (w & 0xE)) {
                b->w &= ~0x6000;
                if (r->f5 != 0 && r->f10 > 0) {
                    if (e->f0->f5 == 1) {
                        e->f18C->f250 |= 0x20;
                    } else if (e->f0->f5 == 4) {
                        for (o = D_8009D20C; o != 0; o = o->next) {
                            if (o != D_8009D254 && o->f0 != 0 && o->f0->f5 == 4) {
                                o->f250 |= 0x20;
                            }
                        }
                    } else {
                        e->f250 |= 0x20;
                    }
                }
            }
        }
        if (r->f5 != 1) {
            e->f68 = 0;
            e->f6C = 0;
            e->f70 = 0;
        }
    }
    if ((b->w & 0xE) && r->f10 > 0) {
        e->f68 = 0;
        e->f6C = 0;
        e->f70 = 0;
        if (!(D_8009D1A0 & 0x100)) {
            e->f3A = (e->f3A + 0x80) % 0x1000;
        }
    }
    if (b->w & 0x1800) {
        e->f68 = 0;
        e->f6C = 0;
        e->f70 = 0;
    }
    if (b->w & 0x400) {
        r->f10 = -1;
    }
    d = r->f14 - r->f10;
    if (d != 0) {
        if (d < 0) {
            r->fD0 = -d;
            r->fD7 = 1;
        } else {
            r->fD0 = d;
            if ((b->w & 0x38000) == 0x18000) {
                r->fD7 = 2;
            } else {
                r->fD7 = 0;
            }
        }
        r->fD2 = e->f218;
        r->fD4 = e->f21A - 0x14;
        r->fD6 = 0x1E;
    }
    if (r->fD6 != 0) {
        func_80032B0C(1, &r->fD0);
        r->fD6--;
    }
    if (r->f10 <= 0 && !(D_8009D1A0 & 0x100)) {
        v = r->f5;
        if (v == 1 && !(r->w.w & 0x6000)) {
            e->f98 |= 0x10;
            func_8002F970(e);
            v = r->f5;
        }
        if (v == 4) {
            if (r->w.w & 0x6000) {
                goto end;
            }
            goto kill;
        }
        if (v != 1 && v != 3) {
        kill:
            e->f68 = 0;
            e->f6C = 0;
            e->f70 = 0;
            func_80028E94(e);
            goto end;
        }
        p = D_8009D20C;
    top:
        if (p == 0) {
            if (r->f5 == 3) {
                goto kill;
            }
            if (r->f5 == 1) {
                e->f98 |= 0x10;
                func_8002F970(e);
                e->f18C->f68 = 0;
                e->f18C->f6C = 0;
                e->f18C->f70 = 0;
                func_80028E94(e->f18C);
            }
            goto end;
        }
        if (p->f0 != 0 && p != D_8009D254 && p->f0->f5 != 0 && p->f0->f5 != 2 && p->f0->f5 != 4 &&
            p->f0->f10 > 0) {
            goto end;
        }
        p = p->next;
        goto top;
    }
end:
    r->f14 = r->f10;
}
