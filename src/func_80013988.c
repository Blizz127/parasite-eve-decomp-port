typedef union {
    int w;
    struct {
        unsigned short lo;
        short hi;
    } h;
} Fix;

typedef struct State {
    unsigned char pad0[0x20];
    int f20;
    unsigned short pad24;
    unsigned short f26;
    int f28;
    int pad2C;
    int f30;
    unsigned char pad34[6];
    short f3A;
    unsigned char pad3C[0x2C];
    Fix f68;
    int pad6C;
    Fix f70;
} State;

typedef struct Obj {
    unsigned int *pc;
    int f4;
    unsigned short f8;
    unsigned short padA;
    int fC;
    int f10;
    int f14;
    int f18;
    int f1C;
} Obj;

extern State *D_8009D2F0;
extern State *D_8009D254;
extern Obj *D_8009D300;
extern unsigned int *D_8009CE00;
extern int func_8003708C(int, int);
extern int func_80079FB4(int, int);
extern int func_80077CF4(int);
extern int func_80077DC4(int);

int func_80013988(int **args)
{
    State *st;
    int x;
    int z;
    int spd;
    int tx;
    int tz;
    register int tol asm("$18");
    register int ang asm("$17");
    int dx;
    int pad[2];

    st = D_8009D2F0;
    x = st->f28;
    z = st->f30;
    if (st != D_8009D254) {
        spd = st->f20;
    } else {
        spd = func_8003708C(0x50000, st->f20);
    }
    spd = func_8003708C(spd, D_8009D2F0->f26 << 4);
    if (!(D_8009D300->f8 & 0x20)) {
        tx = *args[0];
        tz = *args[1];
        if (x == tx && z == tz) {
            return 1;
        }
        tol = *args[2];
        if (D_8009D2F0->f3A < 0) {
            D_8009D2F0->f3A += 0x1000;
        }
        D_8009D300->f14 = tx;
        D_8009D300->f18 = tz;
        D_8009D300->f1C = tol;
        D_8009D300->f8 |= 0x20;
    } else {
        tx = D_8009D300->f14;
        tz = D_8009D300->f18;
        tol = D_8009D300->f1C;
    }
    {
    register int t asm("$3");
    t = 0x1400 - func_80079FB4(z - tz, x - tx);
    ang = t & 0xFFF;
    }
    D_8009D2F0->f68.w = func_8003708C(spd = -spd, func_80077CF4(ang) << 4);
    D_8009D2F0->f70.w = func_8003708C(spd, func_80077DC4(ang) << 4);
    {
    int dd;
    register int cc asm("$4");
    register int k asm("$2");

    dd = D_8009D2F0->f3A;
    cc = dd;
    if (dd < ang) {
        dd = ang - dd;
        if (dd > 0x800) {
            k = tol < dd;
            if (k) {
                k = cc - tol;
                D_8009D2F0->f3A = k;
            }
        } else {
            k = tol < dd;
            if (k) {
                k = cc + tol;
                D_8009D2F0->f3A = k;
            }
        }
    } else {
        dd = dd - ang;
        if (dd <= 0x800) {
            k = tol < dd;
            if (k) {
                k = cc - tol;
                D_8009D2F0->f3A = k;
            }
        } else {
            k = tol < dd;
            if (k) {
                k = cc + tol;
                D_8009D2F0->f3A = k;
            }
        }
    }
    }
    x = (tx - x) >> 16;
    z = (tz - z) >> 16;
    dx = x * x + z * z;
    x = D_8009D2F0->f68.h.hi;
    z = D_8009D2F0->f70.h.hi;
    D_8009D2F0->f3A &= 0xFFF;
    if (x * x + z * z >= dx) {
        D_8009D2F0->f28 = tx;
        D_8009D2F0->f30 = tz;
        D_8009D2F0->f68.w = 0;
        D_8009D2F0->f70.w = 0;
        D_8009D300->f8 &= ~0x20;
        return 1;
    }
    D_8009CE00 -= 5;
    D_8009D300->f10 = 1;
    return 0;
}
