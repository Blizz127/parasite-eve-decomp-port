extern int D_8009D000;
extern int D_8009CFFC;

extern int func_80062CC4();
extern unsigned char *func_80062D2C(int, int, int, int);
extern void func_80062CB8(unsigned char *);
extern void func_8004DA04();
extern void func_8004DA9C();
extern void func_800428D4();
extern void func_80042B50(void (*)());
extern int func_800428C4();
extern void func_8004CE28(int, int);
extern void func_80042910();

void func_80050580(int a0, int a1) {
    unsigned char *p;

    if (a1 != 0) {
        p = func_80062D2C(0x27, func_80062CC4(), 0, 1);
        *(unsigned int *)(p + 0x30) = (unsigned int)func_8004DA04;
        *(unsigned int *)(p + 0x2C) = (unsigned int)func_8004DA9C;
        func_80062CB8(p);
        D_8009D000 = 0x40;
        func_80042B50(func_800428D4);
    } else {
        func_8004CE28(func_800428C4() + 0x47, 0x4B);
        D_8009CFFC = (int)func_80042910;
    }
}
