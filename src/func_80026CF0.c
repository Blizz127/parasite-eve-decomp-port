typedef struct Inner {
    unsigned char pad0[0x10];
    unsigned int f10;
} Inner;

typedef struct Rec {
    unsigned char pad0[0x68];
    Inner *f68;
} Rec;

typedef struct Slot {
    void *a;
    short type;
    short grp;
} Slot;

extern Rec *D_8009D278;
extern Slot D_800BE830[];
extern unsigned char D_8009D2B0[];
extern unsigned char D_8009CE3C;
extern unsigned char D_8009CE44;
extern unsigned char D_8009D1DC;
extern signed char D_8009D2D8;
extern unsigned char D_8009CE60;
extern void func_8005112C(void);

void func_80026CF0(void)
{
    unsigned char n;
    short type;
    short i;
    register int c asm("$2");

    n = D_8009CE3C;
    D_8009CE44 = 0;
    type = D_800BE830[n - 1].type;
    if (type == 1) {
        if (D_8009D1DC == (D_8009D278->f68->f10 & 0xF)) {
{ register unsigned char x asm("$3"); x = n - 1; D_8009CE3C = x; { register unsigned char t asm("$2") = D_8009D2D8; t = t + 1; D_8009D2D8 = t; } c = x; }
            goto t1;
        b1:
c = D_8009CE3C - 1; D_8009CE3C = c; c &= 0xFF;
        t1:
            if (c != 0 && D_800BE830[c].grp == D_800BE830[c - 1].grp) {
                goto b1;
            }
            goto check;
        }
c = D_8009CE3C = n - 1;
        goto t2;
    b2:
c = D_8009CE3C - 1; D_8009CE3C = c; c &= 0xFF;
    t2:
        if (c != 0 && D_800BE830[c].grp == D_800BE830[c - 1].grp) {
            goto b2;
        }
        goto done;
    } else if (type == 2) {
        if ((D_8009D278->f68->f10 & 0xC0) == 0xC0) {
            D_8009D2D8++;
            D_8009CE3C = n - D_8009D2B0[0];
        } else if ((D_8009D278->f68->f10 & 0xC0) == 0x40) {
            D_8009D2D8++;
            D_8009CE3C = n - (((int)(D_8009D278->f68->f10 & 0xF) * 3) >> 1);
        }
        goto done;
    } else if (type < 0x197) {
        D_8009D2D8++;
        if (D_8009CE60 == 0) {
            func_8005112C();
        }
        n = D_8009CE3C;
        D_8009CE3C = n - 1;
    t3:
        if (D_8009CE3C != 0 && D_800BE830[D_8009CE3C].grp == D_800BE830[D_8009CE3C - 1].grp) {
            D_8009CE3C--;
            goto t3;
        }
    check:
        asm("" ::: "$4");
        if (D_8009CE60 != 0) {
            D_8009CE60 = 0;
            D_8009CE3C = n;
        }
    done:
        D_8009D1DC = D_8009D278->f68->f10 & 0xF;
    }
    for (i = 0; i < 45; i++) {
        if (D_800BE830[i].grp == D_8009D2D8) {
            D_800BE830[i].a = 0;
            (&D_800BE830[i])->grp = 0;
            D_800BE830[i].type = 0;
        }
    }
}
