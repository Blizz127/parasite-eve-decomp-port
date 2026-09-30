extern int D_8009CEF4;
extern int D_800C0E44;

extern void func_800434C0();
extern void func_800638D8(int, void (*)());
extern void func_800614AC(int);
extern void func_800622B0(int);

void func_8004FEEC(int a0) {
    D_8009CEF4 = a0;
    func_800638D8(a0, func_800434C0);
    func_800614AC(D_800C0E44);
    func_800622B0(0);
}
