typedef struct Inner {
    unsigned char pad0[2];
    short f2;
    unsigned char pad4[2];
    short f6;
    unsigned char pad8[8];
    unsigned int f10;
} Inner;

typedef struct Rec {
    unsigned char pad0[0x4C];
    unsigned int f4C;
    unsigned char pad50[0x18];
    Inner *f68;
} Rec;

typedef struct Res {
    signed char f0;
    unsigned char f1;
} Res;

typedef struct Actor {
    unsigned char pad0[0x98];
    unsigned int f98;
} Actor;

extern Rec *D_8009D278;
extern void *D_8009D254;
extern int func_80030534(Actor *, void *);
extern int func_80071A54(void);

static inline int lim_of(int thr)
{
    int lim;

    lim = ((D_8009D278->f68->f10 >> 17) & 1) * 20;
    if (thr > 800) {
        lim += 5;
    } else if (thr > 500) {
        lim += 20;
    } else {
        lim += 50;
    }
    return lim;
}

void func_80021278(Actor *a, Res *r, int mode)
{
    int thr;
    short roll;

    r->f1 = 0;
    if (!(a->f98 & 0x4000)) {
        if (D_8009D278->f4C & 0x100000) {
            r->f0 = 1;
            r->f1 = 2;
            return;
        }
        r->f0 = 0;
        thr = 200;
        if ((D_8009D278->f4C & 0x30) != 0x10) {
            thr = D_8009D278->f68->f2;
        }
        thr = func_80030534(a, D_8009D254) * 1000 / thr;
        roll = func_80071A54() % 100;
        if (D_8009D278->f68->f6 == 8) {
            if (thr > 1000) {
                return;
            }
            if (lim_of(thr) > roll) {
                r->f1 = 3;
            }
            r->f0 = 1;
            return;
        }
        if (D_8009D278->f68->f6 == 6 && mode != 0) {
            r->f0 = 1;
            switch (mode) {
            case 2:
                r->f1 = 1;
                break;
            case 3:
                r->f1 = 2;
                break;
            default:
                r->f1 = 4;
                break;
            }
            return;
        }
        if (thr > 1800) {
            return;
        }
        if (thr > 1500) {
            if (roll < 25) {
                r->f0 = 1;
                r->f1 = 1;
            }
        } else if (thr > 1200) {
            if (roll < 50) {
                r->f0 = 1;
                r->f1 = 1;
            }
        } else if (thr > 1000) {
            if (roll < 80) {
                r->f0 = 1;
                r->f1 = 1;
            }
        } else {
            if (lim_of(thr) > roll) {
                r->f1 = 3;
            } else {
                r->f1 = 2;
            }
            r->f0 = 1;
        }
    } else {
        r->f0 = -1;
    }
}
