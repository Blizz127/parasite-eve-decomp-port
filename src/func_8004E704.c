typedef struct W {
    unsigned char pad0[0x2C];
    void *f2C;
    void *f30;
    unsigned char pad34[0xC];
    int f40;
    int f44;
    unsigned char pad48[0x30];
    struct W *f78;
    struct W *f7C;
    int f80;
    void *f84;
    void *f88;
} W;

extern int D_8009CF0C;
extern int D_8009CF94;
extern int D_8009CF8C;
extern int D_8009CF00;
extern int D_8009CEFC;
extern W *func_80062A34(int, int);
extern W *func_80062D2C(int, W *, int, int);
extern W *func_8006322C(int, W *, W *);
extern void func_800647D0(W *, int);
extern void func_80062CB8(W *);
extern int func_800588EC(int);
extern void func_80051510(void);
extern void func_8005B890(int);
extern int func_80058C4C(int);
extern void func_80063158(W *, int, int);
extern void func_8005DE88(void);
extern void func_8004ECB4();
extern void func_8004EBCC();
extern void func_80044444();
extern void func_8004EC3C();
extern void func_80058FEC();
extern void func_8004EC78();
extern void func_80050260();
extern void func_8004C608();

void func_8004E704(int mode)
{
    W *w;
    W *l;
    W *m;
    int n;
    int first;

    l = func_80062A34(2, 50);
    if (l == 0) {
        w = func_80062D2C(50, 0, 0, 0);
        l = func_8006322C(50, w, w);
        w->f2C = func_8004ECB4;
        l->f30 = func_8004EBCC;
        if (mode == 0) {
            func_800647D0(l, 3);
        }
        func_80062CB8(l);
    }
    w = func_80062D2C(51, l, 0, 0);
    l = func_8006322C(51, w, w);
    w->f2C = func_80044444;
    w->f40 = 1;
    l->f30 = func_8004EC3C;
    l->f84 = func_80058FEC;
    l->f44 = -1;
    if (mode != 0) {
        D_8009CF0C = 2;
    } else {
        D_8009CF0C = 1;
    }
    n = func_800588EC(mode);
    func_80051510();
    func_8005B890(0);
    func_800647D0(l, func_80058C4C(mode != 0 ? 0x3803FE : 0xF400));
    first = l->f80 == 0;
    w = func_80062D2C(52, l, 0, 0);
    m = func_8006322C(52, w, w);
    w->f2C = func_80044444;
    if (first) {
        func_80063158(w, -6, 0);
    }
    m->f30 = func_8004EC78;
    m->f84 = func_80058FEC;
    m->f88 = func_80050260;
    m->f44 = -1;
    D_8009CF94 = -1;
    D_8009CF8C = -1;
    func_800647D0(m, n);
    m->f78 = l;
    l->f7C = m;
    if (func_80062A34(1, 19) == 0) {
        func_80062D2C(19, 0, 0, 0)->f30 = func_8004C608;
    }
    D_8009CF00 = 0;
    D_8009CEFC = 0;
    func_8005DE88();
}
