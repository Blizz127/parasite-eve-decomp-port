extern int D_8009D048;
extern int D_8009D04C;
extern int D_8009D050;
extern int D_8009D054;
extern int D_8009D058;
extern int D_8009D064;
extern int D_800A1F84;
extern int D_800C0E48;
extern int D_8009D05C;
extern int func_80052F70();

void func_80052E30(int a0) {
    if (a0 != 0 && D_8009D04C != 0) {
        int v = D_8009D054;
        D_8009D048 = D_8009D04C;
        D_8009D058 = (int)&D_800A1F84;
        D_8009D064 = 4;
        D_8009D050 = v;
    } else {
        D_8009D048 = (int)&D_800C0E48;
        D_8009D050 = func_80052F70();
        D_8009D058 = (int)&D_8009D05C;
        D_8009D064 = 2;
    }
}
