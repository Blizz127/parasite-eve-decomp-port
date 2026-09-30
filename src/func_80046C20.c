extern int D_8009CFB4;
extern int D_8009CF14;
extern int D_8009CF10;
extern int D_8009CFA0;
extern int D_8009CFA8;
extern unsigned char D_800A1980[];

extern unsigned char *func_80062A34(int, int);
extern int func_80063428(unsigned char *);
extern int func_800556E8(int);
extern unsigned char *func_80062CC4();
extern int func_8005DC9C(int);
extern unsigned char *func_80062D2C(int, unsigned char *, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80062CB8(unsigned char *);
extern void func_80052E30(int);
extern void func_80052BCC(unsigned char *, int);
extern int func_8005DC4C(int);
extern void func_80052C08(unsigned char *, int);
extern int func_8005F1A0(unsigned char *);
extern void func_80044E14();
extern void func_80044E98();
extern void func_8004F950();
extern void func_80046DBC();

void func_80046C20(void) {
    unsigned char *p;
    unsigned char *q;
    unsigned char *c;
    int t2;
    register int t asm("$18");
    int w;
    int cb;

    D_8009CFB4 = func_800556E8(func_80063428(func_80062A34(2, 8)));
    c = func_80062CC4();
    t = func_8005DC9C(D_8009CFB4 + 0xEB);
    p = func_80062D2C(0x29, c, 0, 1);
    q = func_8006322C(0x29, p, p);
    *(unsigned int *)(p + 0x30) = (unsigned int)func_80044E14;
    *(unsigned int *)(p + 0x2C) = (unsigned int)func_80044E98;
    *(unsigned int *)(q + 0x30) = (unsigned int)func_8004F950;
    D_8009CF14 = 5;
    func_80062CB8(q);
    cb = (int)func_80046DBC;
    func_80052E30(D_8009CF10);
    if (t != 0) {
        func_80052BCC(D_800A1980, t);
    } else {
        D_800A1980[0] = 0xFF;
    }
    func_80052C08(D_800A1980, func_8005DC4C(0x1A));
    D_8009CFA0 = 0;
    if (func_8005F1A0(D_800A1980) < 0x78) {
        w = 0x78;
    } else {
        w = func_8005F1A0(D_800A1980);
    }
    *(int *)(p + 0x34) = w + 0x14;
    *(int *)(p + 0x38) = 0x32;
    *(int *)(p + 0x18) = (0x12C - w) >> 1;
    *(int *)(q + 0x18) = (*(int *)(p + 0x34) - 0x80) >> 1;
    t2 = *(int *)(p + 0x38);
    D_8009CFA8 = cb;
    *(int *)(q + 0x1C) = t2 - 0x14;
}
