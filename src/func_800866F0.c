extern int D_800BCD80, D_800BCD84;
extern void func_8008CBA8(void);
void func_800866F0(int a0) { D_800BCD80 = 0x30; D_800BCD84 = a0 & 0x3FF; func_8008CBA8(); }
