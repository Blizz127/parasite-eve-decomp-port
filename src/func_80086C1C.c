extern int D_800BCD80, D_800BCD84, D_800BCD90;
extern void func_8008CBA8(void);
void func_80086C1C(int a0, int a1) {
    D_800BCD80 = 0xC0;
    D_800BCD84 = a1 & 0x7F;
    D_800BCD90 = a0;
    func_8008CBA8();
}
