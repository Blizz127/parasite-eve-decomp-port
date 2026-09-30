extern int D_8009CF10;
extern int D_8009CF14;
extern int D_8009CF44;
extern int D_8009CF48;
extern int D_8009CF4C;
extern int D_8009CF50;
extern int D_8009CFA0;
extern unsigned int D_8009CFA8;
extern int D_8009CFF8;
extern unsigned char D_800A1980[];

extern unsigned char *func_80062A20(int, int);
extern unsigned char *func_80062A34(int, int);
extern void func_80062F3C(int);
extern void func_80062F1C(int);
extern void func_80042A10(void);
extern void func_80052634(void);
extern int func_8006346C(unsigned char *);
extern unsigned char *func_800424B4(int, int);
extern void func_8003FFAC(int);
extern unsigned char *func_80062D2C(int, unsigned char *, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80062CB8(unsigned char *);
extern void func_80052E30(int);
extern int func_8005DC4C(int);
extern void func_80052C08(unsigned char *, int);
extern int func_8005F1A0(unsigned char *);
extern int func_80042964(int);
extern void func_8004D978(int);
extern void func_80042B50(void (*)());
extern void func_8004CE28(int, int);
extern void func_800525EC(void);
extern void func_800526C4(void);

extern void func_80044E14();
extern void func_80044E98();
extern void func_8004F950();
extern void func_80050544();
extern void func_800504F4();
extern void func_8005051C();

int func_8004D6D4(int a, int flags) {
    unsigned char *obj;
    unsigned char *obj2;
    unsigned char *nw;
    unsigned char *p;
    int r;
    int m;
    int h;
    int s;
    int t;
    register int t2 asm("$2");
    void (*fn)();

    obj = func_80062A20(a, 0);
    if (flags & 0x40) {
        func_80062F3C(0x3F);
        func_80062F1C(a);
        func_80042A10();
        func_80052634();
        return 1;
    }
    if (flags & 0x10000) {
        r = func_8006346C(obj);
        D_8009CF48 = r;
        if (r < 0) {
            goto fail;
        }
        p = func_800424B4(D_8009CF44, r);
        if (p == 0) {
            goto fail;
        }
        m = D_8009CF50;
        if (m == 0 && p[0] == 2) {
            goto fail;
        }
        D_8009CF4C = p[0];
        if (m != 0) {
            func_8003FFAC(D_8009CFF8);
            if (D_8009CF4C != 2) {
                nw = func_80062D2C(0x29, obj, 0, 1);
                obj = func_8006322C(0x29, nw, nw);
                *(unsigned int *)(nw + 0x30) = (unsigned int)func_80044E14;
                *(unsigned int *)(nw + 0x2C) = (unsigned int)func_80044E98;
                *(unsigned int *)(obj + 0x30) = (unsigned int)func_8004F950;
                D_8009CF14 = 0x6C;
                func_80062CB8(obj);
                t = D_8009CF10;
                *(int *)(obj + 0x44) = 1;
                func_80052E30(t);
                D_800A1980[0] = 0xFF;
                func_80052C08(D_800A1980, func_8005DC4C(0x44));
                D_8009CFA0 = 0;
                fn = func_80050544;
                if (func_8005F1A0(D_800A1980) < 0x78) {
                    h = 0x78;
                } else {
                    h = func_8005F1A0(D_800A1980);
                }
                *(int *)(nw + 0x34) = h + 0x14;
                *(int *)(nw + 0x38) = 0x32;
                *(int *)(nw + 0x18) = (0x12C - h) >> 1;
                *(int *)(obj + 0x18) = (*(int *)(nw + 0x34) - 0x80) >> 1;
                t2 = *(int *)(nw + 0x38);
                D_8009CFA8 = (unsigned int)fn;
                *(int *)(obj + 0x1C) = t2 - 0x14;
            } else if (func_80042964(D_8009CF44) < 0xF) {
                func_80062F3C(0x1F);
                func_8004D978(0x45);
                func_80042B50(func_800504F4);
            } else {
                s = D_8009CF44;
                if (func_80062A34(1, 0x28) == 0) {
                    func_8004CE28(s + 0x47, D_8009CF50 + 0x42);
                }
            }
        } else {
            func_80062F3C(0x1F);
            func_8004D978(0x46);
            func_80042B50(func_8005051C);
        }
        func_800525EC();
        return 1;
    fail:
        func_800526C4();
    }
    return 1;
}
