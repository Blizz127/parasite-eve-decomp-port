typedef struct W {
    int f0;
    struct W *f4;
    unsigned char pad8[0x10];
    int f18;
    unsigned char pad1C[0x10];
    void *f2C;
    void *f30;
    unsigned char pad34[0x10];
    int f44;
    int f48;
    unsigned char pad4C[0x34];
    int f80;
} W;

extern int D_8009CF30;
extern int D_8009CF0C;
extern int D_8009CF34;
extern int D_8009CF1C;
extern W *func_80062A20(W *, int);
extern int func_80054288(void);
extern W *func_80062A34(int, int);
extern void func_80063158(W *, int, int);
extern void func_80063198(W *);
extern void func_80063D30(W *);
extern void func_80062CB8(W *);
extern W *func_80062D2C(int, W *, int, int);
extern W *func_8006322C(int, W *, W *);
extern void func_800525EC(void);
extern void func_800526C4(void);
extern void func_80062F1C(W *);
extern void func_80062F3C(int);
extern void func_80048918(int, int, int);
extern W *func_80062CC4(void);
extern void func_8004E704(int);
extern void func_80052634(void);
extern void func_8004FA10();

int func_8004EF58(W *self, int key)
{
    register W *l asm("$17");
    W *w;
    W *v;
    int fresh;
    int x;
    int dx;

    func_80062A20(self, 0);
    if (key & 0x14000) {
        if (func_80054288() != 0) {
            l = func_80062A34(2, 7);
            fresh = l != 0;
            if (l == 0) {
                asm("" : "=r"(l) : "0"(l));
                l = func_80062A20(func_80062A34(1, 13), 0);
                w = func_80062A34(1, 15);
                v = func_80062A20(func_80062A34(1, 13), 0);
                dx = (D_8009CF30 ? 176 : (v->f80 ? 162 : 156)) - w->f18;
                func_80063158(w, dx, 0);
                func_80063198(w);
                w = func_80062A34(1, 11);
                func_80063158(w, dx, 0);
                func_80063198(w);
                w = func_80062A34(1, 47);
                func_80063158(w, dx, 0);
                func_80063198(w);
            }
            l->f44 = 0;
            l->f48 = 0;
            func_80063D30(l);
            func_80062CB8(l);
            if (fresh) {
                self = func_80062D2C(53, 0, 0, 0);
                func_8006322C(53, self, self)->f30 = func_8004FA10;
            }
            func_800525EC();
        } else {
            func_800526C4();
        }
    }
    if (key & 0x40) {
        func_80062F1C(self);
        if (func_80062A34(1, 5) != 0) {
            func_80062F3C(5);
            func_80062F3C(6);
            func_80062F3C(7);
            func_80062F3C(53);
            func_80048918(830, -1, -1);
        } else {
            func_80062F3C(15);
            func_80062F3C(11);
            func_80062F3C(13);
            func_80062F3C(24);
            func_80062F3C(48);
            if (D_8009CF0C != 0) {
                func_80062CB8(func_80062A34(2, 50));
            } else {
                if (func_80062CC4() != 0) {
                    func_80063198(func_80062CC4()->f4);
                }
                func_80063198(func_80062A34(1, 27));
            }
            D_8009CF34 = 0;
            func_80062F3C(47);
            D_8009CF1C = 0;
            D_8009CF30 = 0;
            func_8004E704(D_8009CF0C - 1);
        }
        func_80052634();
    }
    return 1;
}
