extern int func_80062A20(int, int);
extern void func_800525EC();
extern int func_80063428(int);
extern void func_800649D0(int);
extern void func_80062F1C(int);
extern void func_80052634();

int func_8004B0A4(int a0, int a1) {
    int v;

    v = func_80062A20(a0, 0);
    if ((a1 & 0x10000) != 0) {
        func_800525EC();
        func_800649D0(func_80063428(v));
        func_80062F1C(a0);
    } else if ((a1 & 0x40) != 0) {
        func_80062F1C(a0);
        func_80052634();
    }
    return 1;
}
