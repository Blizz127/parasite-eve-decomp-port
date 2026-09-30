extern int D_8009CFE4;

extern unsigned char *func_80062D2C(int, int, int, int);
extern void func_80062CB8(unsigned char *);
extern int func_8005E884();
extern void func_8004B5DC();
extern void func_8004B650();

void func_8004B584(int a0) {
    unsigned char *p;

    p = func_80062D2C(0x38, a0, 0, 1);
    *(unsigned int *)(p + 0x30) = (unsigned int)func_8004B5DC;
    *(unsigned int *)(p + 0x2C) = (unsigned int)func_8004B650;
    func_80062CB8(p);
    D_8009CFE4 = func_8005E884();
}
