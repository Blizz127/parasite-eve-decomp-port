extern int D_800BCD80, D_800BCD84;
extern void func_8008CBA8(void);
void func_80086874(int a0) { D_800BCD80 = 0xA8; D_800BCD84 = a0 & 0x7F; func_8008CBA8(); }
