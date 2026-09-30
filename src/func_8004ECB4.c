typedef struct W {
    unsigned char pad0[0x2C];
    void *f2C;
    void *f30;
    unsigned char pad34[0x10];
    int f44;
} W;

extern int D_8009CF0C;
extern int D_8009CFB8;
extern int D_8009CF1C;
extern int D_8009CF30;
extern int D_800A1888[];
extern W *func_80062A20(W *, int);
extern int func_80063428(W *);
extern W *func_80062A34(int, int);
extern W *func_80062D2C(int, W *, int, int);
extern W *func_8006322C(int, W *, W *);
extern void func_80062CB8(W *);
extern void func_800525EC(void);
extern void func_800631AC(W *);
extern void func_80062F3C(int);
extern void func_80052E30(int);
extern int func_80053F20(int);
extern int func_80053F90(int);
extern void func_80048918(int, int, int);
extern void func_8004E97C(void);
extern void func_80052634(void);
extern void func_800471E4();
extern void func_800471BC();

int func_8004ECB4(W *self, int key)
{
    W *w;
    W *l;
    W *n;
    int sel;
    int id;

    w = func_80062A20(self, 0);
    if (key & 0x10000) {
        sel = func_80063428(w);
        switch ((D_8009CF0C == 1 && sel == 2) ? 4 : sel) {
        case 0:
            w = func_80062A34(2, 51);
            w->f44 = 0;
            func_80062CB8(w);
            func_800525EC();
            break;
        case 1:
            n = func_80062D2C(59, w, 0, 0);
            l = func_8006322C(59, n, n);
            n->f2C = func_800471E4;
            l->f30 = func_800471BC;
            func_80062CB8(l);
            func_800631AC(func_80062A34(1, 51));
            D_8009CFB8 = D_8009CF0C;
            break;
        case 2:
            func_80062F3C(51);
            func_80062F3C(52);
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
            id = 830;
            goto show;
        case 3:
            func_80062F3C(51);
            func_80062F3C(52);
            D_8009CF30 = 1;
            id = 894;
        show:
            func_80048918(id, -1, -1);
            func_800525EC();
            break;
        case 4:
            if (key & 0x10000) {
                func_8004E97C();
                func_800525EC();
            }
            break;
        }
    } else if (key & 0x40) {
        func_8004E97C();
        func_80052634();
    }
    return 1;
}
