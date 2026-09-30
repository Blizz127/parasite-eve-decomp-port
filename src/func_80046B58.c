extern int func_80062A20(int, int);
extern int func_8006346C(int);
extern void func_80046C20();
extern void func_800525EC();
extern void func_800526C4();
extern void func_80062F1C(int);
extern int func_80062A34(int, int);
extern void func_800439D8();
extern void func_80052634();

int func_80046B58(int a0, int a1) {
    int r;
    int v;

    r = 0;
    v = func_80062A20(a0, 0);
    if ((a1 & 0x10000) != 0) {
        if (func_8006346C(v) >= 0) {
            r = 1;
            func_80046C20();
            func_800525EC();
        } else {
            r = 1;
            func_800526C4();
        }
    } else if ((a1 & 0x40) != 0) {
        func_80062F1C(a0);
        func_80062F1C(func_80062A34(1, 0x1D));
        r = 1;
        func_800439D8();
        func_80052634();
    }
    return r;
}
