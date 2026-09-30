extern int D_800BCD80, D_800BCD84;
extern void func_8008CBA8(void);
void func_80086CF8(int a0) { D_800BCD80 = 0xC8; D_800BCD84 = a0; func_8008CBA8(); }
