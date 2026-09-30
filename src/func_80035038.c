typedef struct Model {
    unsigned short f0;
    unsigned char f2;
    unsigned char f3;
} Model;

typedef struct Sub {
    unsigned char *f0;
    unsigned char pad4[0x70 - 0x4];
    short f70;
    unsigned char pad72[0x88 - 0x72];
    unsigned char f88;
    unsigned char f89;
    unsigned char f8A;
} Sub;

typedef struct Ent {
    int f0;
    struct Ent *next;
    struct Ent *prev;
    unsigned char fC;
    unsigned char fD;
    unsigned char fE;
    unsigned char padF;
    short f10;
    unsigned char pad12[2];
    int f14;
    unsigned char pad18[4];
    int f1C;
    int f20;
    short f24;
    short f26;
    unsigned char pad28[0x38 - 0x28];
    short f38;
    short f3A;
    short f3C;
    unsigned char pad3E[0x58 - 0x3E];
    int f58;
    int f5C;
    int f60;
    unsigned char pad64[4];
    int f68;
    int f6C;
    int f70;
    unsigned char pad74[4];
    int f78;
    int f7C;
    int f80;
    unsigned char pad84[4];
    int f88;
    int f8C;
    int f90;
    unsigned char pad94[4];
    int f98;
    int f9C;
    int fA0[3];
    int fAC[0x38];
    int f18C;
    int f190;
    int f194;
    int f198;
    int f19C;
    int f1A0;
    int f1A4;
    unsigned char pad1A8[4];
    Model *f1AC;
    int f1B0;
    Sub sub;
    unsigned char pad[0x278 - 0x1B4 - sizeof(Sub)];
    int f278;
    unsigned char f27C;
    unsigned char f27D;
} Ent;

typedef struct Loc {
    unsigned char pad0[7];
    unsigned char f7;
    unsigned int f8;
} Loc;

extern Ent *D_8009D2AC;
extern Ent *D_8009D20C;
extern Ent *D_8009D254;
extern int D_8009D224;
extern unsigned short D_8009D2A6;
extern int D_800915DC[][2];
extern Model *D_800B0E70[];
typedef struct Tbl {
    int t[10][48];
    int pad;
    int *p;
} Tbl;
extern Tbl D_800B0E98;
extern char D_800B0CE2[];
extern unsigned char *D_800B0E64;
extern unsigned char D_800B89F8[];
extern unsigned char D_800BEA40[];

extern void func_8002F76C(Ent *);
extern int func_80012700(int, int);
extern void func_8001A680(Ent *, unsigned short);
extern int func_800362B8(int);
extern void func_8003D050(Sub *, Model *, int, int, int, int, int, int, void *, int);
extern void func_8006698C(Sub *);
extern void func_8003D834(Sub *, int, int, void *, void *);

Ent *func_80035038(unsigned char *desc, Ent *after, int flag)
{
    Ent *e;
    unsigned int i;
    int v;
    unsigned char buf[8];
    int **pp;

    if (D_8009D2AC == 0) {
        return 0;
    }
    e = D_8009D2AC;
    D_8009D2AC = e->next;
    if (after != 0) {
        e->next = after->next;
        e->prev = after;
        after->next = e;
        if (e->next != 0) {
            e->next->prev = e;
        }
    } else {
        if (D_8009D20C != 0) {
            D_8009D20C->prev = e;
        }
        e->prev = 0;
        e->next = D_8009D20C;
        D_8009D20C = e;
    }
    e->f88 = 0;
    e->f8C = 0x4000;
    e->f90 = 0;
    e->f68 = 0;
    e->f6C = 0;
    e->f70 = 0;
    e->f78 = 0;
    e->f7C = 0;
    e->f80 = 0;
    e->f58 = 0;
    e->f5C = 0;
    e->f60 = 0;
    e->f190 = D_800915DC[desc[0]][0];
    e->f194 = D_800915DC[desc[0]][1];
    if (desc[0] == 0) {
        D_8009D254 = e;
        e->f20 = 0x10000;
        func_8002F76C(e);
    } else {
        e->f20 = 0x50000;
        e->f0 = 0;
    }
    e->fC = desc[0];
    e->fD = desc[1];
    if (desc[0] < 10) {
        e->f1AC = D_800B0E70[desc[0]];
    } else {
        e->f1AC = 0;
    }
    e->f98 = 0x10000000;
    e->f1C = 0x10000;
    e->f10 = 200;
    e->f1B0 = 0;
    e->f14 = 0;
    e->f18C = 0;
    e->f1A4 = 0;
    e->f19C = 0;
    e->f1A0 = 0;
    e->f198 = 0;
    e->f26 = 0x1000;
    e->f24 = D_8009D224++;
    e->fE = 0;
    e->f27C = 0;
    e->f27D = 0x80;
    for (i = 0; i < 0x38; i++) {
        e->fAC[i] = 0;
    }
    for (i = 0; i < 3; i++) {
        e->fA0[i] = 0;
    }
    pp = &D_800B0E98.p;
    e->f9C = (*pp)[desc[0] + 2];
    e->fA0[2] = func_80012700(e->f9C, 0);
    e->f38 = 0;
    e->f3A = 0;
    e->f3C = 0;
    D_8009D2A6++;
    if (e->f1AC != 0) {
        if (e == D_8009D254) {
            func_8001A680(e, 0x15);
        } else {
            for (i = 0; i < 0x30; i++) {
                if (D_800B0E98.t[e->fC][i] != 0) {
                    func_8001A680(e, i);
                    break;
                }
            }
        }
        if (flag != 0) {
            v = func_800362B8(e->f1AC->f0 * 8 + e->f1AC->f3 * 12 + e->f1AC->f2 * 32 + 0x50);
        } else {
            v = func_800362B8(e->f1AC->f3 * 12 + e->f1AC->f2 * 32);
            e->f98 |= 0x600000A0;
        }
        e->f278 = v;
        if (desc[0] == 0 && (char *)D_800B0CE2 != 0) {
            func_8003D050(&e->sub, e->f1AC, v + 0x50, 0x3C0, 0x100, 0, 0x1C0, 2, buf, flag);
        } else {
            unsigned char *base = D_800B0E64;
            unsigned int *wp = (unsigned int *)(base + *(int *)(base + 4) + 0xC);
            Loc *tbl = (Loc *)(base + (*wp & 0x3FFFFF));
            for (i = 0; i < *wp >> 22; i++) {
                if (tbl[i].f7 == desc[0]) {
                    break;
                }
            }
            func_8003D050(&e->sub, e->f1AC, e->f278 + 0x50, (tbl[i].f8 >> 6) & 0x3C0, (tbl[i].f8 >> 9) & 0x180, 0,
                          ((tbl[i].f8 >> 18) & 0xFF) + 0x1C0, (tbl[i].f8 >> 8) & 0xF, buf, flag);
        }
        if (e->f1B0 != 0) {
            e->sub.f88 = 0x80;
            e->sub.f89 = 0xC;
            e->sub.f8A = 0x18;
            func_8006698C(&e->sub);
            func_8003D834(&e->sub, e->f1B0, *(short *)((char *)e + 0x16), D_800BEA40, D_800B89F8);
            *(short *)(e->sub.f0 + 0x14) = e->sub.f70 * 2;
        }
    } else {
        e->f98 |= 0xE0;
    }
    return e;
}
