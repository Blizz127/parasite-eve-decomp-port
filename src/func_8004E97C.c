typedef struct W {
    unsigned char pad0[0x2C];
    void *f2C;
    void *f30;
    unsigned char pad34[0x10];
    int f44;
    int f48;
    unsigned char pad4C[0x10];
    int f5C;
    unsigned char pad60[0x24];
    void *f84;
    void *f88;
} W;

extern int D_8009CF98;
extern int D_8009CF9C;
extern int D_8009CF0C;
extern int D_8009CEF0;
extern int D_8009CEFC;
extern int D_8009CEF8;
extern int D_8009CF00;
extern int D_8009CF94;
extern int D_8009CF8C;
extern int D_8009D008;
extern short D_800C0E46[];
extern void func_80052EC0(void);
extern void func_800512AC(int, int *);
extern void func_80062F9C(void);
extern W *func_80062A34(int, int);
extern W *func_80062D2C(int, W *, int, int);
extern W *func_8006322C(int, W *, W *);
extern void func_80062CB8(W *);
extern int func_8005B89C(void);
extern void func_800647D0(W *, int);
extern void func_800439D8(void);
extern void func_8004C594(void);
extern void func_80062F3C(int);
extern W *func_80062CC4(void);
extern void func_80055760(void);
extern int func_80052F70(void);
extern void func_80043DA4();
extern void func_8004F838();
extern void func_800447F0();
extern void func_80044444();
extern void func_8004F8D0();
extern void func_80057C54();
extern void func_80050260();

void func_8004E97C(void)
{
    W *w;
    W *l;
    W *top;
    W *nw;
    int m;
    int n;
    int i;
    int v;

    func_80052EC0();
    D_8009CF0C = 0;
    if (D_8009CF98 != 0) {
        func_80062F9C();
        if (func_80062A34(1, 0) == 0) {
            w = func_80062D2C(0, 0, 0, 0);
            l = func_8006322C(0, w, w);
            w->f2C = func_80043DA4;
            l->f30 = func_8004F838;
            func_80062CB8(l);
            if (func_8005B89C() != 0) {
                m = D_8009CEF0 & 0x1F;
            } else {
                m = D_8009CEF0 & 0x1EF;
            }
            n = 0;
            for (i = 8; i >= 0; i--) {
                n += m & 1;
                m >>= 1;
            }
            func_800647D0(l, n);
            D_8009CEFC = 0;
            D_8009CEF8 = 1;
            func_800439D8();
            func_8004C594();
        }
        func_80062F3C(45);
        func_80062F3C(24);
        func_80062F3C(18);
        top = func_80062CC4();
        func_80062D2C(27, 0, 0, 0)->f30 = func_800447F0;
        nw = func_80062D2C(1, top, 0, 0);
        l = func_8006322C(1, nw, nw);
        nw->f2C = func_80044444;
        l->f30 = func_8004F8D0;
        func_80062CB8(l);
        l->f84 = func_80057C54;
        l->f88 = func_80050260;
        func_80055760();
        D_8009CF94 = -1;
        D_8009CF8C = -1;
        func_800647D0(l, func_80052F70());
        D_8009CF00 = 0;
        if (D_8009CF98 != 0) {
            v = D_8009CF98 - 1;
            l->f44 = v & 1;
            l->f48 = (v >> 1) & 0x7F;
            l->f5C = v >> 8;
        }
        if (D_8009D008 != 0) {
            D_8009D008 = 0;
        } else {
            D_800C0E46[D_8009CF98] = D_8009CF9C;
            func_80055760();
        }
        D_8009CF98 = 0;
    } else {
        func_800512AC(9, 0);
    }
}
