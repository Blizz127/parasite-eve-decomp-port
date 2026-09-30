extern int *func_80062A20(int, int);
extern int *func_80062A34(int, int);
extern void func_80047FE0(int *, int *, int);
extern void func_80062CB8(int *);
extern void func_8005267C();

int func_80047F48(int a0, int a1) {
    int *p;

    p = func_80062A20(a0, 0);
    func_80047FE0(p, func_80062A34(2, 6), a1);
    if ((a1 & 0x1000) != 0) {
        p[0x11] = -1;
        p = func_80062A34(2, 0x1C);
        p[0x11] = 0;
        p[0x12] = p[0x16] - 1;
        func_80062CB8(p);
        func_8005267C();
    }
    return 1;
}
