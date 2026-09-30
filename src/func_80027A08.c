typedef struct Inner {
    unsigned char pad0[6];
    short f6;
} Inner;

typedef struct Rec {
    unsigned char pad0[0x68];
    Inner *f68;
} Rec;

typedef struct St {
    unsigned int f0;
    unsigned char pad4;
    signed char f5;
    unsigned char pad6[0xA];
    int f10;
} St;

typedef struct Actor {
    St *f0;
    struct Actor *next;
    unsigned char pad8[0x184];
    struct Actor *f18C;
    unsigned char pad190[0x24];
    unsigned char f1B4[0x9C];
    unsigned short f250;
} Actor;

typedef struct Slot {
    Actor *a;
    short type;
    short grp;
} Slot;

extern Rec *D_8009D278;
extern Actor *D_8009D20C;
extern Actor *D_8009D254;
extern int D_8009D208;
extern Slot D_800BE830[];
extern unsigned char D_8009D1D4;
extern unsigned char D_8009D23C;
extern Actor *D_800B8A90[];
extern unsigned char D_8009CE68;
extern int func_8006F39C(int, Actor *);
extern void func_8003CAEC(unsigned char *, int, int, int);

int func_80027A08(Actor *a)
{
    St *st;
    Actor *t;
    register unsigned char v asm("$5");
    unsigned char i;
    register int r asm("$4");

    st = a->f0;
    r = 1;
    if ((st->f0 & 0x6000) == 0x2000) {
        if (D_800BE830[D_8009D1D4].type != 0x196 && D_800BE830[D_8009D1D4].type != 0x189) {
            if (D_8009D278->f68->f6 != 6) {
                if (D_8009D23C == 0) {
                    for (i = 0; D_800B8A90[i] != 0; i++) {
                        D_800B8A90[i] = 0;
                    }
                }
                D_800B8A90[D_8009D23C++] = a;
            } else if (a == D_800BE830[D_8009D1D4].a) {
                D_8009D208 = func_8006F39C(0x5B, a);
                D_8009D23C = 0;
            }
        }
        if (st->f5 == 0 || st->f10 <= 0) {
            r = 0;
        } else {
            D_8009CE68 = 0xFF;
            r = 0;
        }
    } else if ((st->f0 & 0x6000) == 0x4000 && st->f5 != 0) {
        {
            register unsigned char t asm("$2") = D_8009CE68;
            D_8009CE68 = v = t - 8;
        }
        if (st->f5 == 1) {
            if (v <= 0x80) {
                a->f18C->f250 |= 0x20;
                st->f0 &= ~0x6000;
            } else {
                func_8003CAEC(a->f18C->f1B4, v, v, v);
                r = 0;
            }
        } else if (st->f5 == 4) {
            for (t = D_8009D20C; t != 0; t = t->next) {
                if (t != D_8009D254 && t->f0 != 0 && t->f0->f5 == 4) {
                    if (D_8009CE68 <= 0x80) {
                        t->f250 |= 0x20;
                        st->f0 &= ~0x6000;
                    } else {
                        func_8003CAEC(t->f1B4, D_8009CE68, D_8009CE68, D_8009CE68);
                        r = 0;
                    }
                }
            }
        } else if (st->f10 > 0) {
            if (v <= 0x80) {
                a->f250 |= 0x20;
                st->f0 &= ~0x6000;
            } else {
                func_8003CAEC(a->f1B4, v, v, v);
                r = 0;
            }
        }
    }
    return r;
}
