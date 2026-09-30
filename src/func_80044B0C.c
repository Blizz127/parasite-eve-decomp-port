typedef struct W {
    int f0;
    struct W *f4;
    unsigned char pad8[0x3C];
    int f44;
    int f48;
    int f4C;
    int f50;
    int f54;
} W;

typedef struct Rec {
    unsigned char pad0[6];
    unsigned char b6;
    unsigned char pad7[7];
    unsigned char bE;
} Rec;

extern int D_8009CF10;
extern int D_8009CDA8;
extern int D_8009CF04;
extern int D_8009CF0C;
extern int D_8009CF08;
extern int D_80092234[][3];
extern signed char D_800C0E22[];
extern void func_80052E30(int);
extern W *func_80062A20(W *, int);
extern int func_80063428(W *);
extern Rec *func_8005332C(int);
extern int func_80055FE0(int);
extern void func_80062F1C(W *);
extern void func_80062F3C(int);
extern int func_8005B89C(void);
extern W *func_80062CC4(void);
extern void func_80057834(int);
extern void func_800525EC(void);
extern void func_80055724(void);
extern void func_80055FB4(int);
extern int func_80057654(int);
extern int func_80052F0C(void);
extern int func_80059A40(int);
extern void func_80044F8C(void);
extern void func_8004CC50(int, int);
extern void func_80044274(int);
extern void func_800526C4(void);
extern void func_80052634(void);

int func_80044B0C(W *self, int key)
{
    int r;
    int ok;
    Rec *rec;
    W *l;
    int k;

    r = 0;
    func_80052E30(D_8009CF10);
    if (key & 0x10000) {
        switch (D_80092234[D_8009CDA8][func_80063428(func_80062A20(self, 0))]) {
        case 0:
            ok = 0;
            k = D_8009CF04;
            rec = func_8005332C(k);
            if (func_80055FE0(k) != 0) {
                if (D_8009CF0C != 1 || rec->b6 != 10 || rec->bE < 4) {
                    ok = 1;
                }
            }
            if (!ok) {
                goto fail;
            }
            func_80062F1C(self);
            if (D_8009CF0C == 2) {
                func_80062F3C(51);
                func_80062F3C(52);
            }
            if (func_8005B89C() != 0) {
                func_80062F1C(func_80062CC4()->f4);
            }
            func_80057834(D_8009CF04);
            r = 1;
            func_800525EC();
            break;
        case 1:
            l = self->f4;
            l->f4C = l->f44;
            l->f50 = l->f48;
            if (D_8009CF0C == 0) {
                func_80055724();
                func_80055FB4(l->f54 * l->f48 + l->f44);
            }
            func_80062F1C(self);
        done:
            r = 1;
            func_800525EC();
            break;
        case 2:
            if (func_80057654(D_8009CF04) == 0) {
                goto fail;
            }
            if (func_80052F0C() != 0 || D_8009CF04 != D_800C0E22[0] || func_80059A40(0) != 0) {
                func_80044F8C();
            } else {
                func_8004CC50(29, 0);
            }
            goto done;
        case 3:
            if (D_8009CF08 != 0) {
                goto fail;
            }
            func_80062F1C(self);
            func_80044274(D_8009CF04);
            goto done;
        fail:
            r = 1;
            func_800526C4();
            break;
        default:
            r = 1;
            break;
        }
    } else if (key & 0x40) {
        func_80062F1C(self);
        r = 1;
        func_80052634();
    }
    return r;
}
