typedef struct Rec {
    unsigned char pad0[5];
    signed char f5;
    unsigned char pad6[2];
    int f8;
    unsigned char padC[2];
    unsigned short fE;
    unsigned char pad10[0x91 - 0x10];
    unsigned char f91;
    unsigned char pad92[0x98 - 0x92];
    unsigned short f98;
    unsigned short f9A;
    unsigned short f9C;
    unsigned char f9E;
    unsigned char pad9F;
    short fA0;
    short fA2;
    unsigned char padA4[0xAC - 0xA4];
    unsigned char fAC;
    unsigned char fAD;
    unsigned char fAE;
    unsigned char fAF;
    unsigned char padB0[4];
    unsigned short fB4;
} Rec;

typedef struct Ent {
    Rec *f0;
    struct Ent *next;
    unsigned char pad8[6];
    unsigned char fE;
    unsigned char padF[0x16 - 0xF];
    unsigned short f16;
    unsigned char pad18[0x68 - 0x18];
    int f68;
    int f6C;
    int f70;
    unsigned char pad74[0x98 - 0x74];
    int f98;
    unsigned char pad9C[0x18C - 0x9C];
    struct Ent *f18C;
    unsigned char pad190[0x1B4 - 0x190];
    unsigned char sub[0x250 - 0x1B4];
    unsigned short f250;
    unsigned char f252;
    unsigned char pad253[0x268 - 0x253];
    short f268;
    short f26A;
    short f26C;
} Ent;

typedef struct Drop {
    short a;
    short b;
} Drop;

typedef struct Stat {
    unsigned char padC[0xC];
    short fC;
} Stat;

extern signed char D_8009D2A0;
extern Ent *D_8009D20C;
extern Ent *D_8009D254;
extern Stat *D_8009D278;
extern int D_8009D304;
extern unsigned short D_8009D21C;
extern Drop D_800A7FF0[];

extern void func_8003C5D8(void *, int);
extern void func_800866A4(int, int);
extern void func_8003CAEC(void *, int, int, int);
extern void func_8006DCE4(int, int, int, int, int);
extern void func_8002F970(Ent *);
extern void func_8002F300(void);

void func_80028E94(Ent *e)
{
    Rec *r;
    Ent *o;
    int v;
    int t;
    unsigned char i;
    unsigned char f;

    r = e->f0;
    switch (r->fAC) {
    case 0:
        D_8009D2A0--;
        r->fAC++;
    case 1:
        if (r->fAF != 0) {
            if (e->f16 < r->f91 && e->fE == 0 && r->f5 < 2) {
                break;
            }
            e->f98 |= 0x100;
            func_8003C5D8(e->sub, 0x16);
            r->fAD = 0;
            r->fAC++;
            e->f68 = 0;
            e->f6C = 0;
            e->f70 = 0;
            e->f98 |= 0x1000;
            func_800866A4(0, r->f8);
            for (o = D_8009D20C; o != 0; o = o->next) {
                if (o->f0 == 0 && o->f18C == e) {
                    func_8003C5D8(o->sub, 0x16);
                }
            }
        } else {
            func_800866A4(0, r->f8);
            r->fAC = 3;
        }
        break;
    case 2:
        v = -0x80 - r->fAD * 16;
        func_8003CAEC(e->sub, (unsigned char)v, 0x80, (unsigned char)v);
        if ((unsigned char)v == 0) {
            e->f250 |= 2;
            r->fAD = 0;
            r->fAC++;
        } else {
            if (r->fAD == 0 && r->fAE != 0) {
                func_8006DCE4(r->fB4, 0, e->f268, e->f26A, e->f26C);
            }
            r->fAD++;
        }
        for (o = D_8009D20C; o != 0; o = o->next) {
            if (o->f0 == 0 && o->f18C == e) {
                func_8003CAEC(o->sub, (unsigned char)v, 0x80, (unsigned char)v);
                if ((unsigned char)v == 0) {
                    o->f250 |= 2;
                }
            }
        }
        break;
    case 3:
        if (e->f252 != 0 && r->fAF != 0) {
            break;
        }
        e->f98 |= 0x410;
        func_8002F970(e);
        for (o = D_8009D20C; o != 0; o = o->next) {
            if (o->f0 == 0 && o->f18C == e) {
                o->f98 |= 0x10;
            }
        }
        D_8009D304 += r->fE;
        t = r->f98 - r->f9A * r->f9C;
        if (t > 0) {
            D_8009D21C += t;
        }
        for (i = 0; i < 10; i++) {
            if (D_800A7FF0[i].a == 0) {
                D_800A7FF0[i].a = r->f9E;
                break;
            }
        }
        if (r->fA0 != 0) {
            for (i = 0; i < 10; i++) {
                if (D_800A7FF0[i].a == 0) {
                    D_800A7FF0[i].a = r->fA0;
                    if (r->fA2 >= 0) {
                        D_800A7FF0[i].b = r->fA2;
                    }
                    break;
                }
            }
        }
        if (D_8009D2A0 == 0) {
            f = 1;
            for (o = D_8009D20C; o != 0; o = o->next) {
                if (o != D_8009D254 && o->f0 != 0) {
                    f = 0;
                }
            }
            if (f && D_8009D278->fC > 0) {
                func_8002F300();
            }
        }
        break;
    }
}
