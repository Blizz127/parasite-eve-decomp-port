extern int func_800858B0(unsigned int a0);
extern void func_8008DB7C(void);
extern int D_8009B7EC;
extern int D_8009B7F0;
extern int D_8009B7F4;
extern int D_8009B7F8;
extern int D_8009CDE4;

void func_8008E23C(void) {
    int t;
    int a;
    int b;
    int c;
    register int tc asm("$2");

    t = func_800858B0(0xF2000002);
    func_8008DB7C();
    t = func_800858B0(0xF2000002) - t;
    if (t <= 0) {
        t += 0x44E8;
    }
    a = D_8009B7F0;
    b = D_8009B7F4;
    c = D_8009B7F8;
    tc = t;
    D_8009B7F8 = tc;
    D_8009B7EC = a;
    D_8009B7F0 = b;
    D_8009B7F4 = c;
    D_8009CDE4 = a + b + c + tc;
}
