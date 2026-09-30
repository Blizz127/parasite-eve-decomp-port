extern int D_8009CF2C;
extern int D_8009CFBC;

extern int func_80062A20(int, int);
extern int func_8006346C(int);
extern void func_800526C4();
extern void func_8004784C();
extern void func_800480AC();
extern void func_800525EC();
extern void func_80062F1C(int);
extern void func_80052634();

int func_8004778C(int a0, int a1) {
    int v;

    if ((a1 & 0x10000) != 0) {
        v = func_8006346C(func_80062A20(a0, 0));
        D_8009CF2C = v;
        if (v >= 0) {
            if ((v & 1) != 0 || D_8009CFBC == 0) {
                func_8004784C();
            } else {
                func_800480AC();
            }
            func_800525EC();
        } else {
            func_800526C4();
        }
    }
    if ((a1 & 0x40) != 0) {
        func_80062F1C(a0);
        func_80052634();
    }
    return 1;
}
