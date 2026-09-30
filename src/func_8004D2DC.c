extern int D_8009CFF8;

extern unsigned char *func_80062A20(int, int);
extern int func_8006346C(unsigned char *);
extern int func_80042848(int);
extern int func_80042770(int);
extern int func_80042B28();
extern void func_80042A10();
extern void func_8004298C(int, int);
extern void func_800525EC();
extern unsigned char *func_80062A34(int, int);
extern void func_80062F1C(unsigned char *);
extern void func_8005C1EC(int);
extern void func_800512AC(int, int);
extern void func_800526C4();
extern void func_80052634();

int func_8004D2DC(int a0, int a1) {
    int k;

    k = func_8006346C(func_80062A20(a0, 0));
    if ((a1 & 0x10000) != 0) {
        switch (k) {
        case 0:
        case 1:
            if (func_80042848(k) != 0 && func_80042770(k) != 0 && func_80042B28() == 0) {
                func_80042A10();
                func_8004298C(k, 1);
                func_800525EC();
            }
            break;
        case 2:
            func_80062F1C(func_80062A34(1, 0x25));
            func_80062F1C(func_80062A34(1, 0x26));
            func_80062F1C(func_80062A34(1, 0x24));
            func_80062F1C(func_80062A34(1, 0x13));
            func_8005C1EC(0);
            func_800512AC(9, 0);
            D_8009CFF8 = 0;
            func_800525EC();
            break;
        default:
            func_800526C4();
            break;
        }
    } else if ((a1 & 0x40) != 0) {
        func_80062F1C(func_80062A34(1, 0x25));
        func_80062F1C(func_80062A34(1, 0x26));
        func_80062F1C(func_80062A34(1, 0x24));
        func_80062F1C(func_80062A34(1, 0x13));
        func_8005C1EC(0);
        func_800512AC(9, 0);
        D_8009CFF8 = 0;
        func_80052634();
    }
    return 1;
}
