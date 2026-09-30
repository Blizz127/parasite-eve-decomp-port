typedef struct Inner {
    unsigned char pad0[6];
    short f6;
    unsigned char pad8[4];
    unsigned int fC;
} Inner;

typedef struct Rec {
    unsigned char pad0[0x14];
    unsigned char f14[0x38];
    unsigned int f4C;
    unsigned char pad50[0x18];
    Inner *f68;
} Rec;

typedef struct Sub {
    unsigned char pad0[0x10];
    int f10;
} Sub;

typedef struct Actor {
    Sub *f0;
    unsigned char pad4[0x1B0];
    unsigned char f1B4[0xB6];
    short f26A;
} Actor;

typedef struct Cur {
    unsigned char pad0[0xE];
    unsigned char fE;
    unsigned char fF;
    unsigned char pad10[0xA];
    unsigned short f1A;
    unsigned char pad1C[0xC];
    unsigned char f28[0x12];
    short f3A;
} Cur;

typedef struct Slot {
    Actor *a;
    int f4;
} Slot;

extern Cur *D_8009D254;
extern Rec *D_8009D278;
extern int D_8009D2FC;
extern Slot D_800BE830[];
extern unsigned char D_8009D1D4;
extern short D_8009D27C;
extern unsigned char D_8009CE38[8];
extern int D_8009D200;
extern int func_80030534(void);
extern int func_80079FB4(int, int);
extern void func_8001A680(Cur *, int);
extern short func_80030584(unsigned char *, unsigned char *);
extern unsigned char func_8002312C(Slot *);
extern void func_80022D7C(Actor *);
extern void func_8006F6D4(int, int, int, int, int, int);

static inline void face(Actor *p)
{
    register Actor *a asm("$4") = p;
    int d;
    int ang;
    int k;

    d = a->f26A - D_8009D27C;
    ang = func_80079FB4(d, func_80030534());
    if (ang < -0xAB) {
        k = 0;
    } else if (ang < 0xE4) {
        k = 1;
    } else {
        k = 2;
    }
    func_8001A680(D_8009D254, *(k + D_8009D278->f14));
}

void func_80021F38(void)
{
    Slot *s;
    register Actor *a asm("$4");
    register Cur *c asm("$5");

    s = &D_800BE830[D_8009D1D4];
    c = D_8009D254;
    if (c->fE != 0xC) {
        if (D_8009D278->f4C & 0x200000) {
            a = s->a;
            if (a->f0 != 0 && a->f0->f10 > 0) {
                face(a);
                D_8009D278->f4C &= ~0x200000;
            }
        }
        D_8009D254->f3A = func_80030584(s->a->f1B4, D_8009D254->f28);
        switch (func_8002312C(s)) {
        case 0:
            break;
        case 1:
            if (s->a != D_800BE830[D_8009D1D4 + 1].a || s->a->f0->f10 <= 0) {
                D_8009CE38[1] += D_8009CE38[3];
                if (D_8009D278->f68->fC & 0x3FF) {
                    func_80022D7C(s->a);
                }
            }
            if (++D_8009D1D4 == D_8009CE38[4]) {
                if (D_8009D278->f68->f6 != 8) {
                    func_8006F6D4(D_8009D200, 0, 0, 2, 0, 0);
                }
                if (D_8009D2FC != -1) {
                    func_8006F6D4(D_8009D2FC, 0, 0, 2, 0, 0);
                }
            }
            break;
        }
    } else if (c->fF == c->f1A) {
        face(s->a);
    }
}
