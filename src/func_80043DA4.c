typedef struct W {
    int f0;
} W;

extern int D_8009CEF0;
extern int D_8009CF18;
extern int D_8009CF1C;
extern int D_8009CF30;
extern int D_8009CF0C;
extern int D_8009CEF8;
extern int D_800A1888[];
extern W *func_80062A20(W *, int);
extern int func_80063428(W *);
extern int func_8005B89C(void);
extern void func_80062F3C(int);
extern void func_80044174(W *);
extern void func_80046ABC(W *);
extern void func_80045EE4(W *);
extern void func_8004542C(W *);
extern W *func_80062A34(int, int);
extern void func_80046378(W *, int);
extern void func_80046378_b(W *, int) asm("func_80046378");
extern int func_80052534(void);
extern void func_800512AC(int, int *);
extern void func_800526C4(void);
extern void func_8004AD9C(W *);
extern void func_80046DFC(W *, int);
extern void func_80052E30(int);
extern int func_80053F20(int);
extern int func_80053F90(int);
extern void func_80048918(int, int, int);
extern void func_800525EC(void);
extern void func_80062F9C(void);
extern void func_80051244(void);
extern void func_80052634(void);
extern void func_8005D970(void);

int func_80043DA4(W *self, int key)
{
    int r;
    W *w;
    int sel;
    int mask;
    int k;
    int id;

    r = 0;
    if (key & 0x10000) {
        w = func_80062A20(self, 0);
        sel = func_80063428(w);
        mask = func_8005B89C() ? D_8009CEF0 & 0x1F : D_8009CEF0 & 0x1EF;
        k = -1;
        goto test;
    loop:
        if (sel < 0) {
            goto out;
        }
        sel -= mask & 1;
        mask >>= 1;
        k++;
    test:
        if (k < 9) {
            goto loop;
        }
    out:
        switch (k) {
        case 0:
            func_80062F3C(45);
            func_80062F3C(24);
            func_80062F3C(18);
            func_80044174(w);
            break;
        case 1:
            func_80062F3C(45);
            func_80062F3C(24);
            func_80062F3C(18);
            func_80046ABC(w);
            break;
        case 2:
            func_80062F3C(45);
            func_80062F3C(24);
            func_80062F3C(18);
            D_8009CF18 = 1;
            func_80045EE4(w);
            func_8004542C(w);
            func_80046378_b(func_80062A34(2, 5), 0);
            break;
        case 3:
            func_80062F3C(45);
            func_80062F3C(24);
            func_80062F3C(18);
            D_8009CF18 = 0;
            func_80045EE4(w);
            func_8004542C(w);
            func_80046378(func_80062A34(2, 5), 0);
            break;
        case 4:
            if (func_80052534() != 0) {
                func_80062F3C(45);
                func_80062F3C(24);
                func_80062F3C(18);
                func_800512AC(8, 0);
                break;
            }
            r = 1;
            func_800526C4();
            goto end;
        case 5:
            func_80062F3C(45);
            func_80062F3C(24);
            func_80062F3C(18);
            func_8004AD9C(w);
            break;
        case 6:
            func_80062F3C(45);
            func_80062F3C(24);
            func_80062F3C(18);
            func_80046DFC(w, 0);
            break;
        case 7:
            func_80062F3C(45);
            func_80062F3C(24);
            func_80062F3C(18);
            D_8009CF1C = 1;
            func_80052E30(0);
            D_800A1888[0] = func_80053F20(14) ? 999 : func_80053F20(12);
            D_800A1888[1] = func_80053F20(15) ? 999 : func_80053F20(13);
            if (D_8009CF0C != 0) {
                D_800A1888[2] = func_80053F90(14) ? 999 : func_80053F90(12);
                D_800A1888[3] = func_80053F90(15) ? 999 : func_80053F90(13);
            } else {
                D_800A1888[2] = D_800A1888[3] = 0;
            }
            func_80048918(830, -1, -1);
            D_8009CEF8 = 0;
            break;
        case 8:
            func_80062F3C(45);
            func_80062F3C(24);
            func_80062F3C(18);
            D_8009CF30 = 1;
            func_80048918(894, -1, -1);
            D_8009CEF8 = 0;
            break;
        default:
            goto rset;
        }
        r = 1;
        func_800525EC();
    } else if (key & 0x40) {
        func_80062F9C();
        func_800512AC(9, 0);
        if (func_8005B89C() == 0) {
            func_80051244();
        }
        func_80052634();
    rset:
        r = 1;
        __asm__ volatile("");
    } else if (key & 2) {
        func_8005D970();
    }
end:
    return r;
}
