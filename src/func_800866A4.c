extern int D_800BCD80;
extern int D_800BCD84;
extern int D_800BCD88;
extern void func_8008CBA8(unsigned int a0, unsigned int a1);

void func_800866A4(unsigned int a0, unsigned int a1) {
    a0 &= 0xFFFF;
    a1 &= 0xFFFFFF;
    D_800BCD80 = 0x21;
    D_800BCD84 = a0;
    D_800BCD88 = a1;
    func_8008CBA8(a0, a1);
}
