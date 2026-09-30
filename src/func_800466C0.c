typedef struct W {
    unsigned char pad0[0x18];
    int f18;
    int f1C;
    unsigned char pad20[0xC];
    void *f2C;
    void *f30;
    int f34;
    int f38;
    unsigned char pad3C[8];
    int f44;
    unsigned char pad48[0x14];
    int f5C;
    unsigned char pad60[8];
    int f68;
} W;

typedef struct Rec {
    unsigned char pad0[5];
    unsigned char b5;
} Rec;

extern int D_8009CF1C;
extern int D_8009CF14;
extern int D_8009CF10;
extern int D_8009CFA0;
extern void *D_8009CFA8;
extern int D_8009CF0C;
extern int D_800A1888[];
extern unsigned char D_800A1980[];
extern W *func_80062A20(W *, int);
extern int func_80063428(W *);
extern int func_800556E8(int);
extern W *func_80062A34(int, int);
extern void func_80052E30(int);
extern Rec *func_8005332C(int);
extern void func_80059EC8(int, int);
extern void func_80062F3C(int);
extern void func_80047560(void);
extern void func_800525EC(void);
extern int func_80054240(int);
extern int func_8005B89C(void);
extern void func_80046574(W *, int);
extern W *func_80062D2C(int, W *, int, int);
extern W *func_8006322C(int, W *, W *);
extern void func_80062CB8(W *);
extern unsigned char *func_8005DC4C(int);
extern void func_80052C08(unsigned char *, unsigned char *);
extern void func_80052BCC(unsigned char *, unsigned char *);
extern int func_8005F1A0(unsigned char *);
extern unsigned char *func_80053068(int);
extern void func_800526C4(void);
extern void func_80062F1C(W *);
extern void func_80048918(int, int, int);
extern W *func_80062CC4(void);
extern void func_80052634(void);
extern void func_80044E14();
extern void func_80044E98();
extern void func_8004F950();

int func_800466C0(W *self, int key)
{
    register int r asm("$18");
    W *w;
    W *l;
    W *x;
    int id;
    int ok;
    int width;
    int width2;
    void *fp;

    r = 0;
    w = func_80062A20(self, 0);
    if (key & 0x10000) {
        id = func_800556E8(func_80063428(w));
        if (D_8009CF1C != 0) {
            func_80052E30(func_80063428(func_80062A34(2, 54)) == 1);
            ok = (D_800A1888[0] + D_800A1888[1] + D_800A1888[2] + D_800A1888[3] != 0) && (func_8005332C(id)->b5 & 0x40) == 0;
            if (!ok) {
                goto fail;
            }
            func_80059EC8(1, id);
            func_80062F3C(54);
            func_80047560();
        done:
            func_800525EC();
            r = 1;
            goto end;
        }
        if (func_80054240(id) != 0) {
            goto fail;
        }
        if (func_8005B89C() != 0) {
            x = func_80062D2C(41, w, 0, 1);
            l = func_8006322C(41, x, x);
            w = x;
            w->f30 = func_80044E14;
            w->f2C = func_80044E98;
            l->f30 = func_8004F950;
            D_8009CF14 = 5;
            func_80062CB8(l);
            func_80052E30(D_8009CF10);
            D_800A1980[0] = 255;
            func_80052C08(D_800A1980, func_8005DC4C(0));
            D_8009CFA0 = 0;
            fp = func_80046574;
            width = func_8005F1A0(D_800A1980) < 120 ? 120 : func_8005F1A0(D_800A1980);
            w->f34 = width + 20;
            w->f38 = 50;
            w->f18 = (300 - width) >> 1;
            l->f18 = (w->f34 - 128) >> 1;
            l->f1C = w->f38 - 20;
            D_8009CFA8 = fp;
            func_80052BCC(D_800A1980, func_8005DC4C(113));
            func_80052C08(D_800A1980, func_80053068(id));
            if (func_8005F1A0(D_800A1980) >= 120) {
                width2 = func_8005F1A0(D_800A1980);
            } else {
                width2 = 120;
            }
            self = func_80062A34(1, 41);
            self->f34 = width2 + 20;
            self->f18 = (300 - width2) >> 1;
            func_80062A34(2, 41)->f18 = (self->f34 - 128) >> 1;
            goto done;
        }
        func_80046574(self, 1);
        goto done;
    fail:
        func_800526C4();
        r = 1;
        goto end;
    }
    if (key & 0x1040) {
        if (D_8009CF0C != 0) {
            func_80062F3C(53);
            w->f44 = -1;
            w->f5C = 0;
            func_80062CB8(func_80062A34(2, 54));
        } else if (key & 0x40) {
            func_80062F3C(53);
            if (D_8009CF1C != 0) {
                func_80062F1C(self);
                func_80062F3C(5);
                func_80062F3C(6);
                func_80062CB8(func_80062A34(2, 0));
                func_80048918(830, -1, -1);
            } else {
                w->f44 = -1;
                w = func_80062A34(2, 6);
                if (w == func_80062CC4()) {
                    w->f44 = w->f68 == 0;
                    func_80062CB8(w);
                } else {
                    func_80062CB8(func_80062A34(2, 5));
                }
            }
        } else {
            r = 1;
            goto end;
        }
        func_80052634();
        r = 1;
    }
end:
    return r;
}
