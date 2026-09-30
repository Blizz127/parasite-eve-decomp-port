extern int D_800BCD80, D_800BCD84;
extern void func_8008CBA8(void);
void func_80086770(int a0) { D_800BCD80 = 0x90; D_800BCD84 = a0 & 0xFFFFFF; func_8008CBA8(); }
