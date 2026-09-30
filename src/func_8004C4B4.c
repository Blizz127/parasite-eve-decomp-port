extern int D_8009CFF4;

extern int func_80062A34(int, int);
extern unsigned char *func_80062D2C(int, int, int, int);
extern void func_8004C520();

void func_8004C4B4(int a0) {
    unsigned char *p;

    if (func_80062A34(1, 0x1E) == 0) {
        p = func_80062D2C(0x1E, 0, 0, 0);
        *(unsigned int *)(p + 0x30) = (unsigned int)func_8004C520;
        func_80062D2C(0x1C, 0, 0, 0);
    }
    D_8009CFF4 = a0;
}
