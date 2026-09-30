typedef struct Inner {
    unsigned char pad0[0x10];
    unsigned int f10;
} Inner;

typedef struct Rec {
    unsigned char pad0[0x68];
    Inner *f68;
} Rec;

typedef struct Sub {
    unsigned char pad0[5];
    signed char f5;
} Sub;

typedef struct Actor {
    Sub *f0;
    struct Actor *next;
    unsigned char pad8[0x184];
    struct Actor *f18C;
    unsigned char pad190[0xC0];
    unsigned short f250;
} Actor;

typedef struct Ent {
    Actor *a;
    int f4;
    short f8;
    short padA;
} Ent;

extern signed char D_8009D2B0;
extern Rec *D_8009D278;
extern Actor *D_8009D20C;
extern Actor *D_8009D254;
extern unsigned char D_8009CE6C;

static inline void mark(Actor *a)
{
    Actor *t;

    if (a->f0->f5 == 1) {
        a = a->f18C;
    } else if (a->f0->f5 == 4) {
        for (t = D_8009D20C; t != 0; t = t->next) {
            if (t != D_8009D254 && t->f0 != 0 && t->f0->f5 == 4) {
                t->f250 |= 0x20;
            }
        }
        return;
    }
    a->f250 |= 0x20;
}

void func_800275CC(Ent *list, signed char sel)
{
    unsigned char i;
    short y;
    short lo;
    short hi;

    if (D_8009D2B0 == 0) {
        return;
    }
    D_8009CE6C = 0;
    switch ((int)((D_8009D278->f68->f10 >> 6) & 3)) {
    case 0:
        mark(list[sel].a);
        break;
    case 2:
        y = list[sel].f8;
        if (y < -0x600) {
            lo = y + 0x200;
            hi = y + 0xE00;
        } else if (y < 0x600) {
            lo = y - 0x200;
            hi = y + 0x200;
        } else {
            lo = y - 0xE00;
            hi = y - 0x200;
        }
        for (i = 0; list[i].a != 0; i++) {
            if (y < -0x600) {
                if (list[i].f8 < hi && lo < list[i].f8) {
                    continue;
                }
            } else if (y < 0x600) {
                if (list[i].f8 < lo || hi < list[i].f8) {
                    continue;
                }
            } else {
                if (list[i].f8 < hi && lo < list[i].f8) {
                    continue;
                }
            }
            mark(list[i].a);
        }
        break;
    case 1:
    case 3:
        for (i = 0; list[i].a != 0; i++) {
            mark(list[i].a);
        }
        break;
    }
}
