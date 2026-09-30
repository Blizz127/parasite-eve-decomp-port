typedef struct {
    unsigned char f0;
    unsigned char f1;
    unsigned short f2;
    short f4;
} Move;

typedef struct {
    unsigned int f0;
    unsigned char pad4;
    signed char f5;
    unsigned char pad6[0x16];
    unsigned char f1C[0x88];
    Move fA4;
    unsigned char padAA[0x12];
    unsigned char fBC;
    unsigned char fBD;
    unsigned char fBE;
    unsigned char padBF;
    int fC0;
    int fC4;
    int fC8;
} Sub;

typedef struct {
    Sub *f0;
    unsigned char pad4[0xA];
    unsigned char fE;
    unsigned char padF[5];
    int f14;
    int f18;
    int f1C;
    unsigned char pad20[8];
    int f28;
    int pad2C;
    int f30;
    unsigned char pad34[0x64];
    unsigned int f98;
} Actor;

typedef struct {
    unsigned char pad0[0x3A];
    unsigned short f3A;
} Obj;

typedef struct {
    unsigned char pad0[0x4C];
    unsigned int f4C;
} Rec;

extern Rec *D_8009D278;
extern Obj *D_8009D254;
extern int func_800305C8(Obj *, Actor *);
extern void func_8001A680(Actor *, int);
extern int func_80077CF4(short);
extern int func_80077DC4(short);

void func_80028C48(Actor *a)
{
    Sub *s;
    Move *m;
    short r;

    s = a->f0;
    if (s->f5 != 0) {
        return;
    }
    if (s->f0 & 0xE) {
        return;
    }
    if (!(D_8009D278->f4C & 0x80000) || s->f1C[(s->f0 >> 17) & 0x70] == 0) {
        if (s->fBC == 1) {
            s->fBC++;
            s->fBD = a->fE;
            s->fBE = (a->f98 >> 9) & 1;
            s->fC0 = a->f14;
            s->fC4 = a->f18;
            s->fC8 = a->f1C;
        }
    }
    r = func_800305C8(D_8009D254, a);
    if (r > 0x400) {
        if (r < 0xC00) {
            func_8001A680(a, 1);
            goto next;
        }
    }
    func_8001A680(a, 0);
next:
    a->f1C = 0x10000;
    a->f98 |= 0x1000;
    if (D_8009D278->f4C & 0x80000) {
        return;
    }
    if (!(s->f0 & 0x100000)) {
        return;
    }
    if (s->fA4.f0 == 0) {
        return;
    }
    m = &s->fA4;
    switch (m->f0) {
    case 1:
        s->fA4.f2 = 0x190;
        break;
    case 2:
        s->fA4.f1 = 2;
        s->fA4.f2 = 0x46;
        break;
    case 3:
        s->fA4.f1 = 5;
        s->fA4.f2 = 0x14;
        break;
    }
    m->f4 = D_8009D254->f3A + 0x800;
    a->f28 += (m->f2 * func_80077CF4(m->f4)) << 4;
    a->f30 += (m->f2 * func_80077DC4(m->f4)) << 4;
}
