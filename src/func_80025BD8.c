typedef struct Inner {
    unsigned char pad0[0x10];
    unsigned int f10;
    unsigned char f14[4];
} Inner;

typedef struct Rec {
    unsigned char pad0[0x4C];
    unsigned int f4C;
    unsigned char pad50[0x18];
    Inner *f68;
} Rec;

typedef struct Sub {
    unsigned char pad0[5];
    signed char f5;
    unsigned char pad6[0xA];
    int f10;
    unsigned char pad14[0x9B];
    unsigned char fAF;
} Sub;

typedef struct Actor {
    Sub *f0;
    struct Actor *next;
    unsigned char pad8[5];
    unsigned char fD;
    unsigned char padE[0x8A];
    unsigned int f98;
    unsigned char pad9C[0xF0];
    struct Actor *f18C;
} Actor;

typedef struct Slot {
    Actor *a;
    short type;
    short grp;
} Slot;

extern Rec *D_8009D278;
extern Actor *D_8009D20C;
extern Actor *D_8009D254;
extern Slot D_800BE830[];
extern unsigned char D_8009D294;
extern unsigned char D_8009D1DC;
extern unsigned char D_8009CE38[4];
extern unsigned char D_8009CE3C;
extern unsigned char D_8009D2D8;
extern void func_80021128(void);

void func_80025BD8(Actor *act)
{
    Actor *t;
    int k;
    Slot *s;

    D_8009D294 = 0;
    D_8009D1DC = D_8009D278->f68->f10 & 0xF;
    D_8009CE38[0] = D_8009D278->f68->f14[0];
    D_8009CE38[1] = D_8009D278->f68->f14[1];
    D_8009CE38[2] = D_8009D278->f68->f14[2];
    D_8009CE38[3] = D_8009D278->f68->f14[3];
    t = 0;
    if ((act->f98 & 0x40000000) && act->f0->fAF == 0) {
        for (t = D_8009D20C; t != 0; t = t->next) {
            if (t != D_8009D254 && t != act && t->f0 != 0 && t->f0->f10 > 0
                && t->fD == act->fD && !(act->f98 & 0x40000000)) {
                break;
            }
        }
    } else if ((act->f98 & 0x6000) && act->f0->f5 == 3) {
        for (t = D_8009D20C; t != 0; t = t->next) {
            if (t != D_8009D254 && t != act && t->f0 != 0 && t->f0->f10 > 0
                && t->f18C == act && t->f0->f5 == 1) {
                break;
            }
        }
    } else if (!(act->f98 & 0x4000)) {
        t = act;
    }
    if (t == 0) {
        return;
    }
    k = D_8009D278->f68->f10 & 0xC0;
    if (k == 0xC0 || k == 0x40) {
        D_8009D1DC = 0;
        {
        Slot *s = &D_800BE830[D_8009CE3C];
        s->a = t;
        s->type = 2;
        s->grp = (signed char)D_8009D2D8;
        D_8009CE3C++;
        }
    } else {
        while (D_8009D1DC != 0) {
            short g = (signed char)D_8009D2D8;
            D_8009D1DC--;
            s = &D_800BE830[D_8009CE3C];
            s->a = t;
            s->type = 1;
            s->grp = g;
            D_8009CE3C++;
        }
    }
    func_80021128();
    D_8009D278->f4C |= 0x200000;
}

