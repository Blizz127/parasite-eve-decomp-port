extern void func_80062F3C(int);
extern unsigned char *func_80062A34(int, int);
extern unsigned char *func_80062A20(unsigned char *, int);
extern void func_800631AC(unsigned char *);
extern void func_80047E94(unsigned char *);
extern void func_80047BEC(unsigned char *);
extern void func_80062CB8(unsigned char *);

void func_80047560(void) {
    unsigned char *p;
    unsigned char *q;

    func_80062F3C(0x35);
    *(int *)(func_80062A34(1, 5) + 0x34) = 0x80;
    *(int *)(func_80062A34(1, 6) + 0x34) = 0x80;
    *(int *)(func_80062A34(2, 6) + 0x3C) = 0x3E;
    func_800631AC(func_80062A34(1, 7));
    p = func_80062A34(2, 0);
    if (p == 0) {
        p = func_80062A34(2, 0x32);
    }
    func_80047E94(p);
    func_80047BEC(p);
    p = func_80062A20(func_80062A34(1, 5), 1);
    if (p != 0) {
        func_80062CB8(p);
        *(int *)(p + 0x44) = 0;
    }
    p = func_80062A34(2, 0x1B);
    q = func_80062A34(2, 0x1C);
    *(unsigned int *)(p + 0x7C) = (unsigned int)q;
    *(unsigned int *)(q + 0x78) = (unsigned int)p;
    p = func_80062A34(2, 5);
    if (p != 0) {
        *(int *)(p + 0x44) = -1;
    }
}
