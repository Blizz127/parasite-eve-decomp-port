typedef struct Item {
    unsigned char b0;
    unsigned char pad1[8];
    unsigned char b9;
    unsigned short hA;
    unsigned short hC;
    unsigned char padE[4];
    short h12;
    unsigned char pad14[0xB];
    unsigned char b1F;
} Item;

typedef struct Pl {
    unsigned char pad0[0x20];
    signed char b20;
    unsigned char pad21[0x8B];
    Item items[128];
} Pl;

typedef struct Rep {
    int f0;
    Item *f4;
    Item *f8;
    Item *fC;
    int f10;
    int f14;
    int f18;
    int f1C;
    int f20;
} Rep;

extern Item *D_8009D070;
extern Item *D_8009D074;
extern Item D_800A1F94;
extern Item D_800A1FB4;
extern Item D_800A1E44[];
extern Pl D_800C0E00;
extern short *D_8009D048;
extern int D_8009D050;
extern unsigned char D_8009DE64[];
extern Rep *func_80051098(void);
extern Item *func_8005DB44(int);
extern void func_800512AC(int, Item **);

#define CAP(a) ((a) >= 1000 ? 999 : (a))

void func_80057094(void)
{
    Rep *r;
    Item *e;
    Item *p;
    Item *v;
    int w;
    int k;
    int x;
    Item *res;

    if (D_800A1F94.hA == D_8009D070->hA && D_800A1FB4.hA == D_8009D074->hA) {
        return;
    }
    r = func_80051098();
    r->f0 = 4;
    r->fC = 0;
    r->f4 = D_8009D070;
    r->f8 = D_8009D074;
    if (D_800A1F94.hC != 0) {
        e = &D_800A1E44[D_800A1F94.b1F];
        r->fC = e;
        if (e != 0) {
            r->f20 = e->hA;
            e->hA = (CAP(e->b9 + e->h12) < e->hA + D_800A1F94.hC) ? CAP(e->b9 + e->h12) : e->hA + D_800A1F94.hC;
        }
    } else if (D_800A1FB4.hC != 0) {
        e = &D_800A1E44[D_800A1FB4.b1F];
        r->fC = e;
        if (e != 0) {
            r->f20 = e->hA;
            e->hA = (CAP(e->b9 + e->h12) < e->hA + D_800A1FB4.hC) ? CAP(e->b9 + e->h12) : e->hA + D_800A1FB4.hC;
        }
    }
    r->f14 = D_8009D070->hA;
    r->f1C = D_8009D074->hA;
    *D_8009D070 = D_800A1F94;
    *D_8009D074 = D_800A1FB4;
    k = D_800C0E00.b20;
    if (k >= 0 && k < D_8009D050) {
        x = D_8009D048[k];
        w = x;
        if ((unsigned int)(x - 0x100) < 0x80) {
            res = &D_800C0E00.items[x - 0x100];
            goto done;
        }
        if ((unsigned int)(x - 1) < 0xFF) {
            res = func_8005DB44(x - 1);
            goto done;
        }
        if ((unsigned int)(w - 0x200) < 9) {
            res = (Item *)((w << 5) + (int)D_8009DE64);
            goto done;
        }
        res = 0;
    done:
        p = res;
    } else {
        p = 0;
    }
    if (p == D_8009D070) {
        v = p;
        func_800512AC(5, &v);
    } else if (p == D_8009D074) {
        v = p;
        func_800512AC(5, &v);
    } else {
        func_800512AC(7, &v);
    }
}
