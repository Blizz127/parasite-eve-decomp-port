typedef struct Obj {
    unsigned char pad0[0x10];
    int f10;
} Obj;

typedef struct Ent {
    Obj *f0;
    unsigned char pad4[0xE - 4];
    unsigned char fE;
    unsigned char fF;
    unsigned char pad10[4];
    int f14;
    unsigned short f16_pad;
    unsigned short f1A_pad;
    unsigned char pad1C[0x2A - 0x1C];
    short f2A;
    unsigned char pad2C[2];
    short f2E;
    unsigned char pad30[2];
    short f32;
    unsigned char pad34[0x98 - 0x34];
    int f98;
    unsigned char pad9C[0x26A - 0x9C];
    short f26A;
} Ent;

typedef struct Mdl {
    unsigned char pad0[6];
    short f6;
    unsigned char pad8[4];
    int fC;
    int f10;
    unsigned char f14;
    unsigned char f15;
} Mdl;

typedef struct St {
    unsigned char pad0[0x12];
    unsigned char f12;
    unsigned char pad13;
    unsigned char f14[3];
    unsigned char f17[3];
    unsigned char pad1A[0x4C - 0x1A];
    int f4C;
    unsigned char pad50[0x68 - 0x50];
    Mdl *f68;
} St;

typedef struct Q {
    Ent *who;
    short cmd;
    short n;
} Q;

extern Ent *D_8009D254;
extern St *D_8009D278;
extern Q D_800BE830[];
extern int D_8009D2FC;
extern unsigned char D_8009CE38;
extern unsigned char D_8009CE39;
extern unsigned char D_8009CE3A;
extern unsigned char D_8009CE3C;
extern unsigned char D_8009D1D4;
extern unsigned char D_8009D1DC;
extern int D_8009D200;
extern unsigned char D_8009D274;
extern short D_8009D27C;

extern int func_80030534(Ent *, Ent *);
extern int func_80079FB4(int, int);
extern void func_8001A680(Ent *, unsigned short);
extern void func_80023008(void);
extern int func_800518A8(void);
extern void func_8006DD38(int, int, int, int, int);
extern void func_8006F6D4(int, int, int, int, int, int);

static inline int facing(Ent **tgt)
{
    int s;
    int a;

    Ent *p = *tgt;
    s = -D_8009D27C + p->f26A;
    a = func_80079FB4(s, func_80030534(p, D_8009D254));
    if (a < -0xAB) {
        return 0;
    }
    if (a < 0xE4) {
        return 1;
    }
    return 2;
}

int func_8002312C(Ent **tgt)
{
    int r;
    register Ent *e asm("$6");
    register St *st asm("$7");
    register Mdl *m asm("$4");
    unsigned char d;
    unsigned char v;
    unsigned char t;

    r = 0;
    switch (D_8009D254->fE) {
    case 6:
    case 8:
    case 10:
        {
        register Ent *e6 asm("$5") = D_8009D254;
        if (e6->fF != *(unsigned short *)((char *)e6 + 0x16)) {
            return r;
        }
        if (D_8009CE38 != 0) {
            D_8009CE38--;
        } else if (D_8009CE39 != 0) {
            D_8009CE39--;
        } else {
            goto reset;
        }
        e6->f98 |= 0x100;
        break;
        }
    reset:
        D_8009D274 = 0;
        D_8009CE39 = D_8009D278->f68->f15;
        func_8001A680(D_8009D254, *(unsigned char *)(facing(tgt) + (int)D_8009D278 + 0x17));
        D_8009D254->f98 &= ~0x100;
        v = D_8009D278->f68->f10 & 0xF;
        goto tail;
    case 7:
    case 9:
    case 11:
        e = D_8009D254;
        if (e->fF != *(unsigned short *)((char *)e + 0x1A)) {
            break;
        }
        t = D_8009D1D4;
        if (D_800BE830[t].cmd == 0x189) {
            func_8001A680(e, D_8009D278->f12);
            r = 1;
            break;
        }
        st = D_8009D278;
        m = st->f68;
        if ((m->f10 & 0xC0) != 0xC0 || (d = D_8009D1DC) == 1) {
            D_8009D1DC = 0;
            if (D_800BE830[t + 1].cmd >= 3 || t + 1 == D_8009CE3C) {
                func_8001A680(e, st->f12);
                D_8009D278->f4C |= 0x200000;
                D_8009CE38 = D_8009D278->f68->f14;
                r = 1;
                break;
            }
            if (D_800BE830[t + 1].who->f0->f10 <= 0 || D_800BE830[t + 1].who->f0 == 0) {
                func_8001A680(e, st->f12);
                D_8009D1D4++;
                D_8009D278->f4C |= 0x200000;
                goto r1;
            }
            asm("" :: "r"(m));
            m = st->f68;
            if (m->fC & 0x3FF) {
                func_8001A680(D_8009D254, *(unsigned char *)(facing(tgt) + (int)D_8009D278 + 0x14));
                r = 1;
                D_8009D254->f14 = D_8009D254->fF << 16;
                asm volatile("");
                D_8009D254->f98 |= 0x100;
                { Mdl *mm = D_8009D278->f68; D_8009CE39 += D_8009CE3A; D_8009D1DC = mm->f10 & 0xF; }
                break;
            }
            { int x = func_800518A8(); asm volatile(""); r = 1; if (x == 0) break; }
            func_8001A680(D_8009D254, 0xC);
            func_8006DD38(1, 0, D_8009D254->f2A, D_8009D254->f2E, D_8009D254->f32);
        r1:
            r = 1;
            asm volatile("");
            break;
        }
        if (m->fC & 0x3FF) {
            if (D_800BE830[t].who->f0->f10 <= 0 || D_800BE830[t].who->f0 == 0) {
                func_8001A680(e, st->f12);
                D_8009D278->f4C |= 0x200000;
                D_8009D1DC = D_8009D278->f68->f10 & 0xF;
                r = 1;
                break;
            } else {
                v = d - 1;
            tail:
                D_8009D1DC = v;
                func_80023008();
                return r;
            }
        }
        if (func_800518A8() != 0) {
            func_8001A680(D_8009D254, 0xC);
            func_8006DD38(1, 0, D_8009D254->f2A, D_8009D254->f2E, D_8009D254->f32);
            break;
        }
        func_8001A680(D_8009D254, D_8009D278->f12);
        r = 1;
        break;
    default:
        t = D_8009D1D4;
        if (D_800BE830[t].cmd == 0x189) {
            break;
        }
        D_8009D1D4 = t + 1;
        if ((unsigned char)(t + 1) != D_8009CE3C) {
            break;
        }
        if (D_8009D200 < 0) {
            break;
        }
        if (D_8009D278->f68->f6 != 8) {
            func_8006F6D4(D_8009D200, 0, 0, 2, 0, 0);
        }
        func_8006F6D4(D_8009D2FC, 0, 0, 2, 0, 0);
        break;
    }
    return r;
}
