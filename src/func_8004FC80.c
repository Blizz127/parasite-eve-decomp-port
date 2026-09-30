extern int func_80063428();
extern int func_800556E8(int);
extern int func_800515F8(int *);
extern int func_800579D4(int, int);
extern void func_80061044(int, int);
extern void func_80050B48();
extern void func_800638D8(int, void (*)());

void func_8004FC80(int arg0) {
    int local;
    register int a asm("$17");
    register int b asm("$16");
    register int t asm("$18");

    t = arg0;
    a = func_800556E8(func_80063428());
    b = func_800515F8(&local);
    b = b - func_800579D4(a, b);
    func_80061044(b, local);
    func_800638D8(t, func_80050B48);
}
