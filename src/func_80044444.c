typedef struct W {
    unsigned char pad0[0x24];
    int f24;
    unsigned char pad28[0x1C];
    int f44;
    int f48;
    int f4C;
    int f50;
} W;

typedef struct Rec {
    unsigned char pad0[6];
    unsigned char b6;
} Rec;

extern int D_8009CF00;
extern int D_8009CF5C;
extern int D_8009CF8C;
extern int D_8009CF90;
extern int D_8009CF94;
extern int D_8009CEFC;
extern W *func_80062A20(W *, int);
extern void func_80052E30(int);
extern int func_80063428(W *);
extern Rec *func_8005332C(int);
extern int func_80055FE0(int);
extern int func_80057D18(int);
extern int func_80057D30(int);
extern void func_800512AC(int, int *);
extern void func_80053D2C(int);
extern void func_800525EC(void);
extern void func_800526C4(void);
extern void func_80052634(void);
extern int func_80058E08(int);
extern int func_80052F0C(void);
extern void func_800451D0(W *);
extern void func_80044924(W *, int, int);
extern int func_8005415C(int);
extern int func_8005833C(int);
extern void func_80055760(void);
extern W *func_80062A34(int, int);
extern void func_80062CB8(W *);
extern void func_80062F1C(W *);
extern void func_800439D8(void);

int func_80044444(W *self, int key)
{
    int r;
    W *w;
    int id;
    Rec *rec;
    int v;
    int t;
    int x;

    r = 0;
    w = func_80062A20(self, 0);
    func_80052E30(self->f24 == 13 || self->f24 == 52);
    if (D_8009CF00 != 0) {
        if (key & 0x10000) {
            id = func_80063428(w);
            rec = func_8005332C(id);
            if (func_80055FE0(id) != 0) {
                x = rec ? (rec->b6 < 10 ? func_80057D18 : func_80057D30)(id) : 0;
                v = x;
                func_800512AC(0, &v);
                if (D_8009CF5C != 0) {
                    func_80053D2C(D_8009CF5C);
                }
                D_8009CF00 = 0;
                func_800525EC();
            } else {
                func_800526C4();
            }
        } else if (key & 0x40) {
            func_800512AC(9, 0);
            func_80052634();
        }
        return r;
    }
    if (key & 0x10000) {
        if (self->f24 == 51) {
            id = func_80058E08(func_80063428(w));
        } else {
            id = func_80063428(w);
        }
        if (D_8009CF8C >= 0) {
            D_8009CF90 = func_80052F0C();
            D_8009CF94 = id;
            if (func_80055FE0(id) != 0) {
                func_800451D0(w);
            }
        } else if (func_80063428(w) >= 0 && func_8005332C(id) != 0 && D_8009CEFC == 0) {
            if (w->f4C >= 0) {
                w->f4C = -1;
            }
            func_80044924(w, self->f24 == 13 || self->f24 == 52, id);
        } else if (func_8005415C(id) >= 16 && func_8005415C(id) < 19) {
            if (func_8005833C(id) != 0) {
                func_800526C4();
                r = 1;
                goto out;
            }
        } else {
            w->f4C = w->f44;
            w->f50 = w->f48;
        }
        func_800525EC();
        r = 1;
    } else if (key & 0x40) {
        if (D_8009CF8C >= 0) {
            func_80055760();
            D_8009CF8C = -1;
        } else if (w->f4C < 0) {
            t = self->f24;
            if (t == 13 || t == 14) {
                w->f44 = -1;
                func_80062CB8(func_80062A34(2, 12));
            } else if (t == 1) {
                func_80062F1C(self);
                func_80062F1C(func_80062A34(1, 27));
                func_800439D8();
            } else if (t == 51 || t == 52) {
                w->f44 = -1;
                w = func_80062A34(2, 50);
                w->f44 = 0;
                func_80062CB8(w);
            }
        }
        func_80052634();
        r = 1;
    }
out:
    return r;
}
