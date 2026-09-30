extern unsigned char *D_8009CF58;
extern int D_8009CF18;

extern unsigned char *func_800532B4();
extern unsigned char *func_80062D2C(int, int, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80063158(unsigned char *, int, int);
extern void func_80064C20(unsigned char *);
extern void func_80062CB8(unsigned char *);
extern void func_800647D0(unsigned char *, int);
extern void func_8005DE88();
extern void func_8004F644();
extern void func_8004F730();
extern void func_80050204();
extern void func_8005022C();
extern void func_8004F7D8();
extern void func_8004F798();

void func_8004F490(void) {
    unsigned char *p;
    unsigned char *q;
    int m;
    unsigned char *r;

    D_8009CF58 = func_800532B4();
    if (D_8009CF58[6] < 0xA) {
        p = func_80062D2C(5, 0, 0, 0);
        q = func_8006322C(5, p, p);
        *(unsigned int *)(p + 0x30) = (unsigned int)func_8004F644;
        *(unsigned int *)(p + 0x2C) = (unsigned int)func_8004F730;
        m = 0x80;
        *(int *)(p + 0x34) = m;
        func_80063158(p, 0x44, 0x14);
        *(unsigned int *)(q + 0x30) = (unsigned int)func_80050204;
        func_80064C20(q);
        *(int *)(q + 0x70) = -1;
        func_80062CB8(p);
        p = func_80062D2C(6, 0, 0, 0);
        *(int *)(p + 0x34) = m;
        func_80063158(p, 0x44, 0x14);
        q = func_8006322C(6, p, p);
        *(unsigned int *)(q + 0x30) = (unsigned int)func_8005022C;
        func_80064C20(q);
        r = D_8009CF58;
        *(int *)(q + 0x3C) = 0x3E;
        func_800647D0(q, r[0x14]);
        D_8009CF18 = (D_8009CF58[6] != 9);
    } else {
        p = func_80062D2C(0x37, 0, 0, 0);
        *(unsigned int *)(p + 0x30) = (unsigned int)func_8004F7D8;
        *(unsigned int *)(p + 0x2C) = (unsigned int)func_8004F730;
        func_80062CB8(p);
    }
    q = func_80062D2C(0x13, 0, 0, 0);
    *(unsigned int *)(q + 0x30) = (unsigned int)func_8004F798;
    func_8005DE88();
}
