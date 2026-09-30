extern int D_8009CF0C;
extern int D_8009CF18;
extern int D_8009CF1C;
extern int D_8009CF30;
extern int D_8009CF34;
extern int D_8009CF38;
extern int D_8009CF40;
extern int D_8009CF68;
extern int D_8009CF80;
extern int D_8009CFC4;
extern int D_8009CFC8;
extern int D_8009CFCC;
extern int D_8009CFD4;
extern int D_800A1898[];
extern int D_800A18B4[];
extern int D_800A18D8[];
extern int D_800A18FC[];
extern unsigned char D_800A1960;
extern int D_800C0E10;
extern unsigned short D_800C0E28[];

typedef struct Wnd {
    unsigned char pad0[0x24];
    int f24;
    unsigned char pad28[4];
    void (*f2C)();
    void (*f30)();
    int f34;
    int f38;
    unsigned char pad3C[4];
    int f40;
    int f44;
    int f48;
} Wnd;

extern unsigned char *func_8005332C(int);
extern Wnd *func_80062CC4(void);
extern Wnd *func_80062D2C(int, Wnd *, int, int);
extern Wnd *func_8006322C(int, Wnd *, Wnd *);
extern Wnd *func_80062A34(int, int);
extern void func_80063158(Wnd *, int, int);
extern void func_80062CB8(Wnd *);
extern void func_80064B74(Wnd *, int);
extern void func_80063D30(Wnd *);
extern void func_80064C20(Wnd *);
extern void func_800647D0(Wnd *, int);
extern void func_800631AC(Wnd *);
extern int func_80052F0C(void);
extern void func_80052E30(int);
extern void func_800543CC(int, int);
extern void func_800542A0(int);
extern int func_80054288(void);
extern int func_800556E8(int);
extern void func_8005B91C(int, int, int *, int);
extern void func_8004BF08(void);
extern void func_80049008(void);
extern void func_80048F24(void);

extern void func_8004EF58();
extern void func_8004EF30();
extern void func_800494AC();
extern void func_8004FB48();
extern void func_8004905C();
extern void func_800491C8();
extern void func_80050060();
extern void func_80045FA4();
extern void func_8004F9A0();
extern void func_80049354();
extern void func_80050088();
extern void func_8004A0C8();

void func_80048918(int a0, int a1, int a2) {
    Wnd *w;
    Wnd *o;
    unsigned char *it;
    int i;
    int j;
    int k;
    int kind;
    unsigned short *ps;
    int *pd;
    int *pb;
    int *pf;

    it = func_8005332C(a1);
    if (D_8009CF0C) {
        w = func_80062D2C(54, func_80062CC4(), 0, 0);
        o = func_8006322C(54, w, w);
        w->f2C = func_8004EF58;
        if (D_8009CF30) {
            func_80063158(w, 0, 20);
        }
        o->f30 = func_8004EF30;
        func_80062CB8(o);
    }
    w = func_80062D2C(13, func_80062CC4(), 0, 0);
    o = func_8006322C(D_8009CF30 ? 16 : 13, w, w);
    w->f2C = func_800494AC;
    w->f40 = 1;
    o->f30 = func_8004FB48;
    if (D_8009CF1C) {
        func_80064B74(o, 21);
        if (o->f44 < 0) {
            o->f44 = 0;
        }
        func_80063D30(o);
    }
    if (D_8009CF0C) {
        func_80063158(w, 0, D_8009CF30 ? -16 : 20);
        o->f44 = -1;
    }
    D_8009CFD4 = func_80052F0C();
    func_80052E30(0);
    D_8009CFC4 = a1;
    D_8009CF34 = a1 == -2;
    D_8009CF38 = a1 == -3;
    if (D_8009CF34) {
        func_80064C20(o);
        o->f44 = 0;
    }
    if (D_8009CF1C) {
        func_800543CC(a0, -1);
    } else if (!D_8009CF34 && !D_8009CF38) {
        func_800542A0(a0);
    }
    func_800647D0(o, func_80054288());
    if (a2 >= 0) {
        i = 0;
        while (i < func_80054288()) {
            if (func_800556E8(i) == a2) {
                break;
            }
            i++;
        }
        if (i < func_80054288()) {
            o->f48 = i;
            func_80063D30(o);
        }
    }
    if (o->f44 >= 0 && D_8009CF0C == 0) {
        func_80062CB8(o);
    }
    if (D_8009CF30) {
        func_80063158(w, 0, 56);
        w->f38 -= 48;
        w = func_80062D2C(24, 0, 0, 0);
        w->f30 = func_8004905C;
        w->f34 += 20;
        func_80063158(w, -136, 0);
        if (D_8009CF0C == 0) {
            unsigned int neg = (unsigned int)o->f44 >> 31;

            w = func_80062D2C(48, func_80062A34(2, 0), 0, 0);
            o = func_8006322C(48, w, w);
            w->f2C = func_800491C8;
            o->f30 = func_80050060;
            if (neg) {
                o->f48 = 0;
                o->f44 = 0;
            }
            if (o->f44 >= 0) {
                func_80062CB8(o);
            }
        }
        ps = D_800C0E28;
        for (j = 0, pd = D_800A18D8, pb = D_800A18B4, pf = D_800A18FC; j < 7; j++) {
            *pd = *ps++;
            *pf = 0;
            func_8005B91C(j, *pd, pb, 0);
            pd++;
            pb++;
            pf++;
        }
        D_8009CF80 = 0;
        D_8009CF40 = 0;
        D_8009CF68 = D_800C0E10;
        for (k = 6; k >= 0; k--) {
            D_800A1898[k] = 0;
        }
        func_8004BF08();
    }
    func_800631AC(func_80062A34(1, 1));
    func_800631AC(func_80062A34(1, 27));
    if (D_8009CF1C) {
        if (func_80062A34(1, 47) == 0) {
            w = func_80062D2C(47, 0, 0, 0);
            w->f30 = func_80045FA4;
        }
    }
    w = func_80062D2C(11, o, 0, 0);
    o = func_8006322C(11, w, w);
    w->f40 = 1;
    o->f30 = func_8004F9A0;
    func_80064C20(o);
    func_80063158(w, 6, 0);
    w = func_80062D2C(15, o, 0, 0);
    if (D_8009CF30) {
        o = func_8006322C(29, w, w);
        w->f2C = func_80049354;
        o->f30 = func_80050088;
        o->f44 = -1;
        D_800A1960 = 0;
    }
    w->f30 = func_8004A0C8;
    if (it) {
        D_8009CFC8 = it[14] & 3;
    } else {
        D_8009CFC8 = 0;
    }
    if (it) {
        D_8009CFCC = *(unsigned short *)(it + 12);
    } else {
        D_8009CFCC = 0;
    }
    D_8009CF18 = a0 != 0x200;
    kind = func_80062CC4()->f24;
    if (kind == 48 || kind == 54) {
        func_80049008();
    } else {
        func_80048F24();
    }
}
