extern unsigned char *func_80062D2C(int, int, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80062CB8(unsigned char *);
extern void func_800647D0(unsigned char *, int);
extern void func_8004AE1C();
extern void func_8004FF30();

void func_8004AD9C(int a0) {
    unsigned char *p;
    unsigned char *q;

    p = func_80062D2C(0x20, a0, 0, 0);
    q = func_8006322C(0x20, p, p);
    *(unsigned int *)(p + 0x2C) = (unsigned int)func_8004AE1C;
    *(unsigned int *)(q + 0x30) = (unsigned int)func_8004FF30;
    func_80062CB8(q);
    func_800647D0(q, 4);
}
