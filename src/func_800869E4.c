extern int D_800BCD80, D_800BCD84, D_800BCD88;
extern void func_8008CBA8(void);
void func_800869E4(int a0, int a1) { D_800BCD80 = 0xAB; D_800BCD84 = a0 & 0xFF; D_800BCD88 = a1 & 0xFF; func_8008CBA8(); }
