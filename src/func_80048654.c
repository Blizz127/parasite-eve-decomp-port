typedef struct W {
    unsigned char pad0[0x2C];
    void *f2C;
    void *f30;
    unsigned char pad34[0xC];
    int f40;
    int f44;
    unsigned char pad48[0x1C];
    int f64;
    unsigned char pad68[0x10];
    struct W *f78;
    struct W *f7C;
    unsigned char pad80[4];
    void *f84;
    void *f88;
} W;

extern int D_8009CF94;
extern int D_8009CF8C;
extern int D_8009CF00;
extern W *func_80062D2C(int, W *, int, int);
extern W *func_8006322C(int, W *, W *);
extern void func_80062CB8(W *);
extern void func_80064C20(W *);
extern int func_80057ECC(void);
extern void func_800647D0(W *, int);
extern int func_80052F70(void);
extern int func_8005382C(int);
extern void func_80063D78(W *, int, int);
extern void func_8004C594(void);
extern void func_80048838();
extern void func_8004FCF8();
extern void func_80044444();
extern void func_8004FD68();
extern void func_80058030();
extern void func_8004F8D0();
extern void func_80050260();

void func_80048654(void)
{
    W *w;
    W *l;
    W *m;
    int n;
    register void *h1 asm("$17");
    register void *h2 asm("$16");

    w = func_80062D2C(12, 0, 0, 0);
    l = func_8006322C(12, w, w);
    w->f2C = func_80048838;
    l->f30 = func_8004FCF8;
    func_80062CB8(l);
    w = func_80062D2C(13, l, 0, 0);
    l = func_8006322C(13, w, w);
    h1 = func_80044444;
    h2 = func_80058030;
    w->f2C = h1;
    w->f40 = 1;
    l->f30 = func_8004FD68;
    l->f84 = h2;
    func_80064C20(l);
    l->f64 &= ~4;
    func_800647D0(l, func_80057ECC());
    w = func_80062D2C(14, l, 0, 0);
    m = func_8006322C(14, w, w);
    w->f2C = h1;
    m->f30 = func_8004F8D0;
    m->f88 = func_80050260;
    m->f84 = h2;
    m->f44 = -1;
    D_8009CF94 = -1;
    D_8009CF8C = -1;
    func_800647D0(m, func_80052F70());
    m->f78 = l;
    l->f7C = m;
    n = func_8005382C(func_80057ECC());
    if (n >= 0) {
        func_80063D78(m, 0, n + func_80057ECC() - 1);
    } else {
        n = func_8005382C(1);
        if (n >= 0) {
            func_80063D78(m, 0, n + 7);
        }
    }
    m->f44 = -1;
    func_8004C594();
    D_8009CF00 = 0;
}
