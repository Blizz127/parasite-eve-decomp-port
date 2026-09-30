extern int D_8009CFB8;
extern int D_8009CF18;

extern int func_80062A20(int, int);
extern int func_80063428(int);
extern void func_8005B500(int, int);
extern void func_8005B7D0(int, int);
extern void func_80046EAC(int);
extern void func_800525EC();
extern void func_80062F1C();
extern void func_80052634();

int func_800474D0(int a0, int a1) {
    int v;
    void (*f)(int, int);

    if ((a1 & 0x10000) != 0) {
        v = func_80063428(func_80062A20(a0, 0));
        f = func_8005B500;
        if (D_8009CFB8 != 0) {
            f = func_8005B7D0;
        }
        f(D_8009CF18, v);
        func_80046EAC(1);
        func_800525EC();
    } else if ((a1 & 0x40) != 0) {
        func_80062F1C();
        func_80052634();
    }
    return 1;
}
