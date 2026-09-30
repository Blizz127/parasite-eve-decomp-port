extern int D_8009CF20;
extern int D_8009CEF4;

extern void func_80050AD8();
extern int func_80059F08(int);
extern int func_8005332C(int);
extern void func_800638D8(int, void (*)());

void func_8004FAF8(int a0) {
    D_8009CF20 = func_8005332C(func_80059F08(1));
    D_8009CEF4 = a0;
    func_800638D8(a0, func_80050AD8);
}
