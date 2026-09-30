extern unsigned int D_8009D110;
extern unsigned int D_8009D114;
extern int D_8009D124;
extern int D_8009D128;
extern int D_8009D164;
extern int D_8009D168;
extern void func_80062090(int a0, int a1, int a2);

void func_80064C80(void)
{
    D_8009D110 = 0xFF4000;
    D_8009D114 = 0x7F2000;
    D_8009D124 = D_8009D124 - 2;
    D_8009D128 = D_8009D128 - 2;
    func_80062090(D_8009D164, D_8009D168, 0);
    D_8009D110 = 0x808080;
    D_8009D114 = 0x404040;
    D_8009D124 = D_8009D124 + 2;
    D_8009D128 = D_8009D128 + 2;
}
