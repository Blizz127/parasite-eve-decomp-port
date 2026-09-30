typedef struct { int f; } SW;
typedef struct { short f; } SH;
typedef struct { unsigned char f; } SB;
#define W(p, o) (((SW *)((unsigned char *)(p) + (o)))->f)
#define H(p, o) (((SH *)((unsigned char *)(p) + (o)))->f)
#define B(p, o) (((SB *)((unsigned char *)(p) + (o)))->f)

typedef struct {
    unsigned char b[32];
} Blk32;

extern int D_8009CFC4;
extern int D_8009CFC8;
extern int D_8009CFCC;
extern int D_8009CFD4;
extern int D_8009CF0C;
extern int D_8009CF34;
extern int D_8009CF1C;
extern int D_8009CF18;
extern int D_8009CF30;
extern int D_8009CF38;
extern int D_8009CF14;
extern int D_8009CF10;
extern int D_8009CFA0;
extern void *D_8009CFA8;
extern int D_8009CEF8;
extern int D_800A1888;
extern int D_800A188C;
extern int D_800A1890;
extern int D_800A1894;
extern Blk32 D_800A1960;
extern unsigned char D_800A1980[];

extern unsigned char *func_80062A20(int, int);
extern unsigned char *func_80062A34(int, int);
extern void func_80062CB8(unsigned char *);
extern void func_800631AC(unsigned char *);
extern void func_8005267C(void);
extern int func_80063428(unsigned char *);
extern int func_800556E8(int);
extern unsigned char *func_8005332C(int);
extern void func_80052E30(int);
extern void func_80057D30(int);
extern void func_80062F3C(int);
extern unsigned char *func_80062CC4(void);
extern void func_80063198(unsigned char *);
extern void func_8004E94C(void);
extern void func_800525EC(void);
extern int func_80059F08(int);
extern int func_8005415C(int);
extern int func_80054520(int);
extern void func_80063158(unsigned char *, int, int);
extern unsigned char *func_80062D2C(int, unsigned char *, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80064C20(unsigned char *);
extern void func_80064B74(unsigned char *, int);
extern void func_80046378(unsigned char *, int);
extern int func_80055FE0(int);
extern void func_8004DD64(int);
extern void func_80052C08(unsigned char *, int);
extern int func_8005DC4C(int);
extern int func_8005F1A0(unsigned char *);
extern void func_8004CC50(int, int);
extern void func_800526C4(void);
extern void func_800512AC(int, int);
extern void func_800439D8(void);
extern void func_800490B0(void);
extern void func_80052634(void);
extern void func_8004620C();
extern void func_8004F9A0();
extern void func_80045A98();
extern void func_80045D0C();
extern void func_8004F978();
extern void func_8004FFD0();
extern void func_8004FA10();
extern void func_80044E14();
extern void func_80044E98();
extern void func_8004F950();
extern void func_8005033C();

#define CLOSE_MENUS()                                            \
    func_80062F3C(0xF);                                          \
    func_80062F3C(0xB);                                          \
    func_80062F3C(0xD);                                          \
    func_80062F3C(0x18);                                         \
    func_80062F3C(0x30);                                         \
    if (D_8009CF0C != 0) {                                       \
        func_80062CB8(func_80062A34(2, 0x32));                   \
    } else {                                                     \
        if (func_80062CC4() != 0) {                              \
            func_80063198((unsigned char *)W(func_80062CC4(), 4)); \
        }                                                        \
        func_80063198(func_80062A34(1, 0x1B));                   \
    }                                                            \
    D_8009CF34 = 0;

int func_800494AC(int a0, int flags)
{
    
    register unsigned char *m asm("$17");
    unsigned char *p;

    m = func_80062A20(a0, 0);
    if (flags & 0x1000) {
        p = func_80062A34(2, 0x30);
        if (p != 0) {
            W(m, 0x44) = -1;
            W(p, 0x44) = 0;
            W(p, 0x48) = 1;
            func_80062CB8(p);
            func_800631AC(func_80062A34(1, 0xF));
            func_800631AC(func_80062A34(1, 0xB));
            func_800631AC(func_80062A34(1, 0x2F));
            func_8005267C();
        }
        p = func_80062A34(2, 0x36);
        if (p == 0) {
            return 1;
        }
        W(m, 0x44) = -1;
        func_80062CB8(p);
        func_800631AC(func_80062A34(1, 0xF));
        func_800631AC(func_80062A34(1, 0xB));
        func_800631AC(func_80062A34(1, 0x2F));
        func_8005267C();
        return 1;
    }
    if (flags & 0x10000) {
        unsigned char *s0;
        int s3;
        register unsigned char *s2 asm("$18");
        register unsigned char *s1 asm("$17");

        unsigned char *q;
        void (*h)();

        s0 = (unsigned char *)func_80063428(func_80062A34(2, 0xD));
        s3 = func_800556E8((int)s0);
        s2 = func_8005332C(s3);
        if (D_8009CFC4 >= 0) {
            switch (D_8009CFC8) {
            case 0:
                H(s2, 0xE) = (H(s2, 0xE) + D_8009CFCC > 999) ? 999 : H(s2, 0xE) + D_8009CFCC;
                break;
            case 1:
                H(s2, 0x10) = (H(s2, 0x10) + D_8009CFCC > 999) ? 999 : H(s2, 0x10) + D_8009CFCC;
                break;
            case 2:
                H(s2, 0x12) = (H(s2, 0x12) + D_8009CFCC > 999) ? 999 : H(s2, 0x12) + D_8009CFCC;
                break;
            }
            func_80052E30(D_8009CFD4);
            func_80057D30(D_8009CFC4);
            CLOSE_MENUS();
            if (D_8009CF0C == 1) {
                func_80062F3C(0x36);
                func_8004E94C();
            }
            goto done;
        }
        if (D_8009CF1C != 0) {
            int ok;

            s0 = (unsigned char *)func_80059F08(0);
            {register int A_ asm("$3") = D_800A1888; int B_ = D_800A188C; register int C_ asm("$2") = D_800A1890; int D_ = D_800A1894; __asm__("" : "=r"(A_) : "0"(A_)); __asm__("" : "=r"(B_) : "0"(B_)); __asm__("" : "=r"(C_) : "0"(C_)); ok = 0; if (A_ + B_ + C_ + D_ != 0) {
                if (!(B(func_8005332C((int)s0), 5) & 0x40)) {
                    if (func_80054520(func_8005415C((int)s0) < 6 ? 0x3E : 1 << func_8005415C((int)s0)) != 0) {
                        ok = 1;
                    }
                }
            }}
            if (ok == 0) {
                goto out;
            }
            CLOSE_MENUS();
            func_80063158(func_80062A34(1, 0x36), 0x98, 0);
            D_8009CF18 = B(s2, 6) != 9;
            s2 = func_80062A34(2, 0);
            if (s2 == 0) {
                s2 = func_80062A34(2, 0x32);
            }
            {
                unsigned char *t; unsigned char *u = func_80062D2C(6, s2, 0, 0); t = func_8006322C(6, u, u); s0 = u;
                W(s0, 0x2C) = (int)func_8004620C;
                W(s0, 0x40) = 1;
                s0 = t;
            }
            W(s0, 0x30) = (int)func_8004F9A0;
            W(s0, 0x64) |= 0x80;
            if (D_8009CF1C != 0) {
                func_80064C20(s0);
            } else if (D_8009CF18 == 0) {
                func_80064B74(s0, 0x14);
            }
            if (W(s0, 0x44) >= 0) {
                func_80062CB8(s0);
            }
            { unsigned char *u = func_80062D2C(5, s2, 0, 0); unsigned char *t = func_8006322C(5, u, u); s1 = u; s0 = t; }
            W(s1, 0x30) = (int)func_80045A98;
            W(s1, 0x2C) = (int)func_80045D0C;
            W(s0, 0x30) = (int)func_8004F978;
            if (D_8009CF1C != 0) {
                func_80064C20(s0);
            } else if (D_8009CF18 == 0) {
                func_80064B74(s0, 0x12);
            }
            if (W(s0, 0x44) >= 0) {
                func_80062CB8(s0);
            }
            W(s1, 0x40) = 1;
            s0 = func_8006322C(0x1B, s1, s1);
            W(s0, 0x30) = (int)func_8004FFD0;
            func_80064C20(s0);
            func_80046378(s2, 1);
            if (D_8009CF0C == 0) {
                q = func_80062D2C(0x35, 0, 0, 0);
                W(func_8006322C(0x35, q, q), 0x30) = (int)func_8004FA10;
            }
            s1 = func_80062A34(2, 0x36);
            if (s1 != 0) {
                func_80062CB8(s1);
            }
            return 1;
        }
        if (D_8009CF30 != 0) {
            func_80062F3C(0xD);
            func_80062F3C(0x30);
            func_80062F3C(0x36);
            s1 = func_80062A34(1, 0xF);
            if ((W(s1, 4) = (int)func_80062A34(2, 0)) == 0) {
                W(s1, 4) = (int)func_80062A34(2, 0x32);
            }
            func_80063158(s1, 0x1C - W(s1, 0x18), 0x14);
            s1 = func_80062A34(1, 0xB);
            func_80063158(s1, 0x1C - W(s1, 0x18), 0x14);
            s1 = func_80062A34(2, 0x1D);
            W(s1, 0x44) = 0;
            W(s1, 0x48) = 0;
            {
            int w = func_8005415C(func_80059F08(0));
            W(s1, 0x58) = w == 8 ? 1 : w == 6 ? 2 : 3;
            }
            func_80062CB8(s1);
            D_800A1960 = *(Blk32 *)func_8005332C(func_80059F08(0));
        done:
            func_800525EC();
            return 1;
        }
        if (D_8009CF34 != 0 && func_80055FE0((int)s0) != 0) {
            CLOSE_MENUS();
            func_8004DD64(s3);
            goto done;
        }
        if (D_8009CF38 != 0) {
            if (func_80055FE0((int)s0) != 0) {
                int w;
                { unsigned char *u = func_80062D2C(0x29, m, 0, 1); s2 = func_8006322C(0x29, u, u); s0 = u; }
                W(s0, 0x30) = (int)func_80044E14;
                W(s0, 0x2C) = (int)func_80044E98;
                W(s2, 0x30) = (int)func_8004F950;
                D_8009CF14 = 5;
                func_80062CB8(s2);
                func_80052E30(D_8009CF10);
                D_800A1980[0] = 0xFF;
                func_80052C08(D_800A1980, func_8005DC4C(0x61));
                D_8009CFA0 = 0;
                h = func_8005033C;
                w = func_8005F1A0(D_800A1980) < 0x78 ? 0x78 : func_8005F1A0(D_800A1980);
                W(s0, 0x34) = w + 0x14;
                W(s0, 0x38) = 0x32;
                W(s0, 0x18) = (0x12C - w) >> 1;
                W(s2, 0x18) = (W(s0, 0x34) - 0x80) >> 1;
                D_8009CFA8 = h;
                W(s2, 0x1C) = W(s0, 0x38) - 0x14;
                goto done;
            }
            func_8004CC50(0x62, 0);
        }
    out:
        func_800526C4();
        return 1;
    }
    if (flags & 0x40) {
        p = func_80062A34(2, 0x36);
        if (p != 0) {
            W(m, 0x44) = -1;
            W(m, 0x5C) = 0;
            func_80062CB8(p);
            func_800631AC(func_80062A34(1, 0xF));
            func_800631AC(func_80062A34(1, 0xB));
            func_800631AC(func_80062A34(1, 0x2F));
        } else if (D_8009CF34 != 0 || D_8009CF38 != 0) {
            D_8009CF34 = 0;
            D_8009CF38 = 0;
            func_800512AC(9, 0);
        } else {
            func_80062F3C(0x36);
            if (D_8009CF1C != 0) {
                D_8009CF1C = 0;
                CLOSE_MENUS();
                func_80062F3C(0x2F);
                if (D_8009CF0C == 0) {
                    func_800439D8();
                    D_8009CEF8 = 1;
                }
            } else if (D_8009CF30 != 0) {
                func_800490B0();
            } else {
                CLOSE_MENUS();
            }
        }
        func_80052634();
    }
    return 1;
}
