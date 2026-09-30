extern int D_8009D000;

extern unsigned char *func_80062D2C(int, int, int, int);
extern int func_80062CC4();
extern void func_80062CB8(unsigned char *);
extern void func_8004DA04();
extern void func_8004DA9C();

void func_8004D978(int a0) {
    unsigned char *p;

    p = func_80062D2C(0x27, func_80062CC4(), 0, 1);
    *(unsigned int *)(p + 0x30) = (unsigned int)func_8004DA04;
    *(unsigned int *)(p + 0x2C) = (unsigned int)func_8004DA9C;
    func_80062CB8(p);
    D_8009D000 = a0;
}
