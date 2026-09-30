extern int D_8009CF24;

extern void func_80048254();
extern void func_80059FD0();
extern void func_80062F3C(int);
extern int func_80062A34(int, int);
extern void func_80062CB8(int);
extern int func_80059F08(int);
extern void func_80048918(int, int, int);
extern void func_80052634();

int func_80047FE0(int a0, int a1, int a2) {
    int r;
    int v;

    r = 0;
    if ((a2 & 0x10000) != 0) {
        func_80048254();
        r = 1;
    } else if ((a2 & 0x40) != 0) {
        func_80059FD0();
        func_80062F3C(0x2C);
        func_80062F3C(0xB);
        func_80062F3C(0xA);
        func_80062F3C(5);
        func_80062F3C(6);
        func_80062F3C(7);
        v = func_80062A34(2, 0);
        if (v == 0) {
            v = func_80062A34(2, 0x32);
        }
        func_80062CB8(v);
        func_80048918(0x33E, -1, func_80059F08(D_8009CF24 == 0));
        func_80052634();
        r = 1;
    }
    return r;
}
