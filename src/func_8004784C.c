extern int D_8009CF14;
extern int D_8009CF2C;

extern int func_80062CC4();
extern unsigned char *func_80062D2C(int, int, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80062CB8(unsigned char *);
extern void func_8004790C();
extern void func_80047A30();
extern void func_8004F950();

void func_8004784C(void) {
    unsigned char *p;
    unsigned char *q;

    p = func_80062D2C(4, func_80062CC4(), 0, 1);
    q = func_8006322C(4, p, p);
    *(unsigned int *)(p + 0x30) = (unsigned int)func_8004790C;
    *(unsigned int *)(p + 0x2C) = (unsigned int)func_80047A30;
    *(unsigned int *)(q + 0x30) = (unsigned int)func_8004F950;
    D_8009CF14 = 5;
    func_80062CB8(q);
    if ((D_8009CF2C & 1) != 0) {
        *(int *)(p + 0x38) -= 0x1C;
        *(int *)(q + 0x1C) -= 0x1C;
    }
}
