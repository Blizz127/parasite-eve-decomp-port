extern int D_8009CF50;
extern int D_8009CF14;
extern int D_8009CF10;
extern int D_8009CFA4;
extern int D_8009CFA8;
extern int D_8009CFFC;
extern unsigned char D_800A19C0[];

extern unsigned char *func_80062A34(int, int);
extern int func_800428C4();
extern int func_8005DC4C(int);
extern unsigned char *func_80062D2C(int, unsigned char *, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80062CD0(unsigned char *);
extern void func_80052E30(int);
extern void func_80052BCC(unsigned char *, int);
extern void func_80052C08(unsigned char *, int);
extern int func_8005F1A0(unsigned char *);
extern void func_8004CC50(int, int);
extern void func_80044E14();
extern void func_80044E98();
extern void func_8004F950();
extern void func_80050580();
extern void func_80042910();

void func_8004DAA4(void) {
    unsigned char *p;
    unsigned char *q;
    unsigned char *c;
    int t;
    int w;
    int cb;
    int mark;
    int arg;
    int t2;

    if (D_8009CF50 != 0) {
        if (func_80062A34(1, 0x2A) == 0) {
            c = func_80062A34(2, 0x24);
            t = func_8005DC4C(func_800428C4() + 0x47);
            p = func_80062D2C(0x2A, c, 0, 1);
            q = func_8006322C(0x2A, p, p);
            *(unsigned int *)(p + 0x30) = (unsigned int)func_80044E14;
            *(unsigned int *)(p + 0x2C) = (unsigned int)func_80044E98;
            *(unsigned int *)(q + 0x30) = (unsigned int)func_8004F950;
            D_8009CF14 = 0x6C;
            func_80062CD0(q);
            mark = 0x4A;
            cb = (int)func_80050580;
            arg = D_8009CF10;
            *(int *)(q + 0x44) = 1;
            func_80052E30(arg);
            if (t != 0) {
                func_80052BCC(D_800A19C0, t);
            } else {
                D_800A19C0[0] = 0xFF;
            }
            func_80052C08(D_800A19C0, func_8005DC4C(0x49));
            D_8009CFA4 = mark;
            if (func_8005F1A0(D_800A19C0) < 0x78) {
                w = 0x78;
            } else {
                w = func_8005F1A0(D_800A19C0);
            }
            *(int *)(p + 0x34) = w + 0x14;
            *(int *)(p + 0x38) = 0x42;
            *(int *)(p + 0x18) = (0x12C - w) >> 1;
            *(int *)(q + 0x18) = (*(int *)(p + 0x34) - 0x80) >> 1;
            t2 = *(int *)(p + 0x38);
            D_8009CFA8 = cb;
            *(int *)(q + 0x1C) = t2 - 0x14;
        }
    } else {
        if (func_80062A34(1, 0x28) == 0) {
            func_8004CC50(func_800428C4() + 0x47, 0x49);
            D_8009CFFC = (int)func_80042910;
        }
    }
}
