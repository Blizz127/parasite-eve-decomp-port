typedef struct Rec {
    unsigned char pad0[5];
    signed char f5;
} Rec;

typedef struct Ent {
    Rec *f0;
    struct Ent *next;
    unsigned char pad8[0x18C - 8];
    struct Ent *f18C;
    unsigned char pad190[0x1B4 - 0x190];
    unsigned char sub[4];
} Ent;

typedef struct Tgt {
    Ent *e;
    int f4;
    short f8;
} Tgt;

typedef struct Mdl {
    unsigned char pad0[0x10];
    unsigned int f10;
} Mdl;

typedef struct St {
    unsigned char pad0[0x68];
    Mdl *f68;
} St;

extern signed char D_8009D2B0;
extern St *D_8009D278;
extern Ent *D_8009D20C;
extern Ent *D_8009D254;
extern unsigned char D_8009CE68;
extern signed char D_8009CE6C;

extern void func_8003CAEC(void *, int, int, int);

static inline void hilite(Ent *e)
{
    Ent *o;

    if (e->f0->f5 == 1) {
        func_8003CAEC(e->f18C->sub, D_8009CE68, D_8009CE68, D_8009CE68);
    } else if (e->f0->f5 == 4) {
        for (o = D_8009D20C; o != 0; o = o->next) {
            if (o != D_8009D254 && o->f0 != 0 && o->f0->f5 == 4) {
                func_8003CAEC(o->sub, D_8009CE68, D_8009CE68, D_8009CE68);
            }
        }
    } else {
        func_8003CAEC(e->sub, D_8009CE68, D_8009CE68, D_8009CE68);
    }
}

void func_80026FF8(Tgt *list, signed char idx, signed char mode)
{
    unsigned char i;
    short c;
    short lo;
    short hi;

    if (D_8009D2B0 == 0) {
        return;
    }
    if (D_8009CE68 < 0x41) {
        D_8009CE6C = 8;
    } else if (D_8009CE68 >= 0xC0) {
        D_8009CE6C = -8;
    }
    D_8009CE68 += D_8009CE6C;
    if (mode < 4) {
        switch ((int)((D_8009D278->f68->f10 >> 6) & 3)) {
        case 0:
            hilite(list[idx].e);
            break;
        case 2:
            c = list[idx].f8;
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
            for (i = 0; list[i].e != 0; i++) {
                if (c < -0x600) {
                    if (list[i].f8 < hi && lo < list[i].f8) {
                        continue;
                    }
                } else if (c < 0x600) {
                    if (list[i].f8 < lo || hi < list[i].f8) {
                        continue;
                    }
                } else {
                    if (list[i].f8 < hi && lo < list[i].f8) {
                        continue;
                    }
                }
                hilite(list[i].e);
            }
            break;
        case 1:
        case 3:
            for (i = 0; list[i].e != 0; i++) {
                hilite(list[i].e);
            }
            break;
        }
    } else if (mode < 8) {
        hilite(list[idx].e);
    }
}
