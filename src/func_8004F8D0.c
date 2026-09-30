extern int D_8009CEF4;

extern void func_80050804();
extern void func_80052E30(int);
extern void func_800638D8(int, void (*)());

void func_8004F8D0(int a0) {
    func_80052E30(0);
    D_8009CEF4 = a0;
    func_800638D8(a0, func_80050804);
}
