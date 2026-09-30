typedef struct W {
    unsigned char pad0[0x24];
    int f24;
    unsigned char pad28[0x1C];
    int f44;
    int f48;
} W;

extern unsigned char *D_8009CF54;
extern int D_8009D004;
extern unsigned int D_800B0CD8;
extern W *func_80062A20(W *, int);
extern int func_8005BCB0(void);
extern W *func_80062A34(int, int);
extern void func_80062CB8(W *);
extern void func_8005267C(void);
extern unsigned char *func_8005DC4C(int);
extern int func_80063428(W *);
extern void func_8005BD10(int);
extern void func_800525EC(void);
extern int func_8005BD94(void);
extern void func_80052634(void);
extern unsigned char *func_8005BEDC(void);
extern void func_800512AC(int, int *);
extern int func_8005BEE8(void);
extern void func_80052594(int);
extern void func_800526C4(void);

int func_8004E074(W *self, int key)
{
    register W *w asm("$16");
    W *t;
    int pos;
    unsigned char *p;
    unsigned char *q;
    unsigned char *s;
    int ok;

    w = func_80062A20(self, 0);
    if (key & 0x8000) {
        pos = func_8005BCB0();
        w->f44 = -1;
        pos = w->f48 + pos * 3;
        if (pos >= 0) {
            if (pos < 3) {
                w = func_80062A34(2, 23);
                w->f48 = pos;
                goto done;
            }
            if (pos < 5) {
                w = func_80062A34(2, 24);
                w->f48 = pos - 3;
                goto done;
            }
        }
        w = func_80062A34(2, 25);
        w->f48 = 0;
    done:
        w->f44 = 0;
        func_80062CB8(w);
        func_8005267C();
    } else if (key & 0x10000) {
        t = func_80062A34(2, 23);
        D_8009CF54 = s = func_8005DC4C(t == 0 ? 117 : t->f48 + 115);
        func_8005BD10(s[func_80063428(w)]);
        func_800525EC();
    } else if (key & 0x40) {
        if (func_8005BD94() != 0) {
            func_80052634();
        } else {
            func_800526C4();
        }
    } else if (key & 0x800) {
        if (w->f24 != 25) {
            w->f44 = -1;
            t = func_80062A34(2, 25);
            t->f44 = 0;
            func_80062CB8(t);
        } else {
            p = func_8005BEDC();
            ok = 1;
            for (q = p; *q != 255; ) {
                ok &= (*q++ == 15);
            }
            if (p < q && !ok) {
                func_800512AC(9, 0);
                if (D_8009D004 == 0) {
                    func_80052594(func_8005BEE8());
                }
                D_800B0CD8 &= 0xFFFF7FFF;
                func_800525EC();
            } else {
                func_800526C4();
            }
        }
    }
    return 1;
}
