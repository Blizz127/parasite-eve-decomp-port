typedef struct Ent {
    unsigned char f0;
    unsigned char pad1;
    unsigned char f2;
    unsigned char f3;
    int f4;
    int f8;
    unsigned char padC[2];
    unsigned char fE;
    unsigned char fF;
} Ent;

typedef struct St {
    union {
        unsigned int w;
        struct {
            unsigned int lo : 21;
            unsigned int slot : 3;
            unsigned int mid : 6;
            unsigned int b30 : 1;
            unsigned int b31 : 1;
        } b;
    } u0;
    unsigned char pad4;
    signed char f5;
    signed char f6;
    unsigned char pad7[0x11];
    Ent *f18;
    Ent ent[9];
    unsigned char padAC[6];
    unsigned short fB2;
} St;

typedef struct Actor {
    St *f0;
    unsigned char pad4[0xA];
    unsigned char fE;
    unsigned char fF;
    unsigned char pad10[4];
    union {
        unsigned int w;
        struct {
            unsigned short lo;
            unsigned short hi;
        } h;
    } f14;
    union {
        unsigned int w;
        struct {
            unsigned short lo;
            unsigned short hi;
        } h;
    } f18;
    int f1C;
    unsigned char pad20[0x16C];
    struct Actor *f18C;
    unsigned char pad190[0xD8];
    short f268;
    short f26A;
    short f26C;
} Actor;

extern unsigned int D_8009D1A0;
extern void func_8001A680(Actor *, unsigned short);
extern void func_8006DCE4(int, int, int, int, int);

int func_8002FAF8(Actor *a0, unsigned char k)
{
    register Actor *a asm("$6") = a0;
    St *st;
    register Actor *b asm("$17");
    unsigned int t;
    int r;
    Ent *e;
    St *x;
    register Actor *y asm("$4");
    int i;

    r = 0;
    x = a->f0;
    if (x == 0 || !(D_8009D1A0 & 2)) {
        return 1;
    }
    st = x;
    if ((st->f5 == 2 || st->f5 == 4) && (y = a->f18C) != 0) {
        b = y;
    } else {
        b = a;
    }
    if (k >= 6) {
        return r;
    }
    if (b->f18.w > b->f14.w) {
        t = (b->f14.w >> 16) + b->fF + 1;
    } else {
        t = b->f14.w >> 16;
    }
    switch (b->fE) {
    case 0:
    case 1:
        break;
    case 6:
    case 8:
    case 10:
    case 12:
    case 14:
        if (b->fF >= b->f18.h.hi && b->fF < t) {
            func_8001A680(b, st->f18->f3);
            func_8006DCE4(st->fB2, 0, b->f268, b->f26A, b->f26C);
            b->f1C = st->f18->f8;
            st->f18->f0 = 1;
        }
        break;
    case 7:
    case 9:
    case 11:
    case 13:
    case 15:
        if (b->fF >= b->f18.h.hi && b->fF < t && !(st->u0.w & 0x40000000)) {
            func_8001A680(b, st->f6);
            b->f1C = 0x10000;
            if (k < 3) {
                r = 1;
                st->f18->f0 = 4;
            }
        }
        if (st->f5 == 0 && (st->u0.w & 0x6000)) {
            if (st->f18->f0 < 2) {
                st->f18->f0 = 4;
            }
            r = 2;
        }
        if (k < 3) {
            e = st->f18;
            if ((e->fE == 1 || e->fE == 3) && e->fF < b->f14.h.hi && e->fF >= b->f18.h.hi) {
                e->f0 = 3;
            }
        }
        break;
    default:
        i = k;
        if (st->ent[i].f0 == 0) {
            st->f18 = &st->ent[i];
            st->u0.b.slot = i;
            st->u0.b.b30 = 0;
            st->u0.b.b31 = 0;
            func_8001A680(b, st->f18->f2);
            b->f1C = st->f18->f4;
        }
        break;
    }
    if (st->f18->f0 == 4 || st->ent[k].f0 == 4) {
        func_8001A680(b, st->f6);
        b->f1C = 0x10000;
        r = 1;
        st->f18->f0 = 0;
        st->f18 = 0;
    }
    return r;
}
