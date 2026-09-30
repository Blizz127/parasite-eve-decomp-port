extern unsigned short D_800F3424;
extern unsigned short D_800F3426;
extern unsigned short D_800F341C;
extern unsigned short D_800F341E;
extern unsigned char D_800F3422;
extern unsigned char D_800F33AC;
extern unsigned char D_800E224C;
extern short D_800E27AC;
extern int func_80077A64(int a0, int a1, int a2, int a3);

void func_800C2EAC(unsigned int a0) {
    a0 &= 0xFF;
    if (a0 == 0) {
        D_800F3424 = 0x340;
        D_800F3426 = 0x100;
        D_800F341C = 0;
        D_800F341E = 0x1D7;
        D_800F3422 = 0;
    }
    if (a0 == 1) {
        D_800F3424 = 0x340;
        D_800F3426 = 0x160;
        D_800F341E = 0x1DB;
        D_800F341C = 0;
        D_800F3422 = 0x60;
    }
    if (a0 == 2) {
        D_800F3424 = 0x340;
        D_800F3426 = 0x100;
        D_800F341C = 0;
        D_800F341E = 0x1D6;
        D_800F3422 = 0;
    }
    if (a0 == 3) {
        D_800F3424 = 0x380;
        D_800F3426 = 0x100;
        D_800F341C = 0;
        D_800F341E = 0x1C8;
        D_800F3422 = 0;
    }
    D_800E27AC = func_80077A64(D_800F33AC, D_800E224C, D_800F3424, D_800F3426);
}
