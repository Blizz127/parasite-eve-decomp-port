extern int D_8009CFB8;
extern int D_8009CF18;

extern unsigned char *func_80062D2C(int, int, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80062CB8(unsigned char *);
extern void func_80063158(unsigned char *, int, int);
extern void func_800474D0();
extern void func_800474A8();

void func_800473E4(int a0, int a1) {
    unsigned char *p;
    unsigned char *q;

    p = func_80062D2C(0x3C, a0, 0, 0);
    q = func_8006322C(0x3C, p, p);
    *(unsigned int *)(p + 0x2C) = (unsigned int)func_800474D0;
    *(unsigned int *)(q + 0x30) = (unsigned int)func_800474A8;
    func_80062CB8(q);
    if (D_8009CFB8 == 0) {
        func_80063158(p, 0x70 - *(int *)(p + 0x18), 0x20 - *(int *)(p + 0x1C));
    }
    D_8009CF18 = (a1 == 0);
    if (a1 != 0) {
        func_80063158(p, 0, 0x14);
    }
}
