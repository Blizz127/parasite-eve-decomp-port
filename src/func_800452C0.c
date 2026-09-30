extern int D_8009CFAC;
extern int D_8009CF94;
extern int D_8009CF8C;

extern int func_8005E120();
extern int func_80056B24(int);
extern void func_800526C4();
extern void func_8005267C();
extern void func_80057094();
extern void func_80055760();
extern void func_80062F1C(int);
extern void func_800525EC();
extern void func_80052634();

int func_800452C0(int a0, int a1) {
    if ((a1 & 0x1000) != 0) {
        if (func_80056B24(-func_8005E120()) != 0) {
            func_800526C4();
        } else {
            func_8005267C();
        }
        D_8009CFAC = 1;
    } else if ((a1 & 0x4000) != 0) {
        if (func_80056B24(func_8005E120()) != 0) {
            func_800526C4();
        } else {
            func_8005267C();
        }
        D_8009CFAC = 0;
    } else if ((a1 & 0x10000) != 0) {
        func_80057094();
        D_8009CF94 = -1;
        D_8009CF8C = -1;
        func_80055760();
        func_80062F1C(a0);
        func_800525EC();
    }
    if ((a1 & 0x40) != 0) {
        D_8009CF94 = -1;
        D_8009CF8C = -1;
        func_80055760();
        func_80062F1C(a0);
        func_80052634();
    }
    return 1;
}
