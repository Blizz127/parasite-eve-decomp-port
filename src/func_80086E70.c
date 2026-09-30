extern int D_800BCD80, D_800BCD84;
extern void func_8008CBA8(void);
void func_80086E70(int a0) { D_800BCD80 = 0xD4; D_800BCD84 = a0 & 0xFF; func_8008CBA8(); }
