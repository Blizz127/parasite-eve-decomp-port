extern int D_800BCD80, D_800BCD84, D_800BCD88, D_800BCD8C, D_800BCD90;
extern void func_8008CBA8(void);
void func_80086CA4(int a0, int a1, int a2, int a3) { D_800BCD80=0xC2; D_800BCD84=a1; D_800BCD88=a2&0x7F; D_800BCD8C=a3&0x7F; D_800BCD90=a0; func_8008CBA8(); }
