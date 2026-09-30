extern int D_800BCD80;
extern void func_8008CBA8(void);
void func_800867E4(int a0) { int v; switch (a0) { case 1: v=0x9B; break; case 2: v=0x9D; break; default: v=0x99; break; } D_800BCD80=v; func_8008CBA8(); }
