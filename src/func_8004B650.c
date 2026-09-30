extern int D_8009CFE4;

extern void func_8005E850(int, int);
extern void func_8005267C();
extern void func_80062F1C(int);
extern void func_800525EC();
extern int func_8005E884();
extern void func_80052634();

int func_8004B650(int a0, int a1) {
    if ((a1 & 0x1000) != 0) {
        func_8005E850(0, -1);
        func_8005267C();
    } else if ((a1 & 0x4000) != 0) {
        func_8005E850(0, 1);
        func_8005267C();
    } else if ((a1 & 0x10000) != 0) {
        func_80062F1C(a0);
        func_800525EC();
    } else if ((a1 & 0x40) != 0) {
        func_8005E850(0, D_8009CFE4 - func_8005E884());
        func_80062F1C(a0);
        func_80052634();
    }
    return 1;
}
