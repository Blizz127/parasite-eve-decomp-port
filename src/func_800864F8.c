extern int D_800BCD80, D_800BCD84, D_800BCD90;
extern int func_8008CBA8(void);
int func_800864F8(int a0, int a1) {
    int *p = &D_800BCD80;
    int r;
    *p = 0x19;
    D_800BCD84 = a0;
    r = func_8008CBA8();
    *p = 0xC0;
    D_800BCD84 = a1 & 0x7F;
    D_800BCD90 = 0;
    func_8008CBA8();
    return r;
}
