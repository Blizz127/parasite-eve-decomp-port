extern int D_8009CF20;
extern int D_8009CF18;
extern int D_8009CEF4;

extern int func_80059F08(int);
extern unsigned char *func_8005332C(int);
extern void func_800647D0(int, int);
extern void func_80050AD8();
extern void func_800638D8(int, void (*)());

void func_8004F9A0(int a0) {
    unsigned char *p;

    p = func_8005332C(func_80059F08(0));
    D_8009CF20 = (int)p;
    D_8009CF18 = (p[6] != 9);
    func_800647D0(a0, p[0x14]);
    D_8009CEF4 = a0;
    func_800638D8(a0, func_80050AD8);
}
