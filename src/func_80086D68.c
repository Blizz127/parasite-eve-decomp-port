extern int D_800BCD80, D_800BCD84, D_800BCD88, D_800BCD8C;
extern void func_8008CBA8(void);
void func_80086D68(int a0, int a1, int a2) { D_800BCD80 = 0xCA; D_800BCD84 = a0; D_800BCD88 = a1; D_800BCD8C = a2; func_8008CBA8(); }
