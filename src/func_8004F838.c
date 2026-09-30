extern int D_8009CEF4;
extern int D_8009CEF8;

extern void func_80050748();
extern void func_800638D8(int, void (*)());
extern void func_8005EB58(int);
extern void func_8005EB64(int);
extern void func_8005E8A4(int, int);
extern int func_8005257C();
extern void func_80033A40();

void func_8004F838(int a0) {
    int n;

    D_8009CEF4 = a0;
    func_800638D8(a0, func_80050748);
    func_8005EB58(1);
    n = *(int *)(a0 + 0x38);
    while (n != 0) {
        func_8005EB64(0x68);
        func_8005E8A4(0, 0x10);
        n--;
    }
    if (func_8005257C() == 0) {
        if (D_8009CEF8 != 0) {
            func_80033A40();
        }
    }
}
