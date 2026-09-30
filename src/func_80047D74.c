extern int D_8009CF24;

extern unsigned char *func_80062A20(int, int);
extern unsigned char *func_80062A34(int, int);
extern void func_80062CB8(unsigned char *);
extern void func_8005267C();
extern void func_80048254();
extern void func_80059FD0();
extern void func_80062F3C(int);
extern int func_80059F08(int);
extern void func_80048918(int, int, int);
extern void func_80052634();

int func_80047D74(int a0, int a1) {
    unsigned char *p;
    unsigned char *q;

    p = func_80062A20(a0, 0);
    if ((a1 & 0x4000) != 0) {
        *(int *)(p + 0x44) = -1;
        p = func_80062A34(2, 0xB);
        *(int *)(p + 0x44) = 0;
        *(int *)(p + 0x48) = 0;
        func_80062CB8(p);
        func_8005267C();
    } else if ((a1 & 0x10000) != 0) {
        func_80048254();
    } else if ((a1 & 0x40) != 0) {
        func_80059FD0();
        func_80062F3C(0x2C);
        func_80062F3C(0xB);
        func_80062F3C(0xA);
        func_80062F3C(5);
        func_80062F3C(6);
        func_80062F3C(7);
        q = func_80062A34(2, 0);
        if (q == 0) {
            q = func_80062A34(2, 0x32);
        }
        func_80062CB8(q);
        func_80048918(0x33E, -1, func_80059F08(D_8009CF24 == 0));
        func_80052634();
    }
    return 1;
}
