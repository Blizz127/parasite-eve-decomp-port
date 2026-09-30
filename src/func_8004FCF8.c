extern int D_8009CEF4;

extern void func_80050B94();
extern void func_800638D8(int, void (*)());
extern void func_8005EB58(int);
extern void func_8005EB64(int);
extern void func_8005E8A4(int, int);

void func_8004FCF8(int a0) {
    int n;

    D_8009CEF4 = a0;
    func_800638D8(a0, func_80050B94);
    func_8005EB58(1);
    n = *(int *)(a0 + 0x38);
    while (n != 0) {
        func_8005EB64(0x68);
        func_8005E8A4(0, 0x10);
        n--;
    }
}
