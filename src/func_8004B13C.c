extern int D_8009CFE0;
extern unsigned char D_800922D4[];

extern unsigned char *func_80062D2C(int, int, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80062CB8(unsigned char *);
extern int func_800614A0();
extern void func_8004B214();
extern void func_8004B394();
extern void func_8004B534();
extern void func_8004B55C();

void func_8004B13C(int a0) {
    unsigned char *p;
    unsigned char *q;
    unsigned char *r;
    register unsigned char *t asm("$4");

    p = func_80062D2C(0x2E, a0, 0, 0);
    q = func_8006322C(0x2E, p, p);
    *(unsigned int *)(p + 0x30) = (unsigned int)func_8004B214;
    *(unsigned int *)(p + 0x2C) = (unsigned int)func_8004B394;
    *(unsigned int *)(p + 0x4C) = (unsigned int)D_800922D4;
    *(int *)(p + 0x40) = 1;
    *(unsigned int *)(q + 0x30) = (unsigned int)func_8004B534;
    *(int *)(q + 0x28) = 1;
    r = func_8006322C(0x31, p, p);
    *(unsigned int *)(r + 0x30) = (unsigned int)func_8004B55C;
    *(unsigned int *)(q + 0x7C) = (unsigned int)r;
    *(unsigned int *)(r + 0x78) = (unsigned int)q;
    t = q;
    if (*(int *)(q + 0x44) < 0) {
        t = r;
    }
    func_80062CB8(t);
    D_8009CFE0 = func_800614A0();
}
