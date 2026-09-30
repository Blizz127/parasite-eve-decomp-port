extern short *D_8009D048;
extern signed char D_800C0E20[];
extern signed char D_800C0E22[];
extern void func_80055760(void);

int func_80057C54(int a0, int a1, int a2, int a3)
{
    D_8009D048[a1] ^= D_8009D048[a3];
    D_8009D048[a3] ^= D_8009D048[a1];
    D_8009D048[a1] ^= D_8009D048[a3];
    if (D_800C0E20[0] == a1) {
        D_800C0E20[0] = a3;
    } else if (D_800C0E20[0] == a3) {
        D_800C0E20[0] = a1;
    }
    if (D_800C0E22[0] == a1) {
        D_800C0E22[0] = a3;
    } else if (D_800C0E22[0] == a3) {
        D_800C0E22[0] = a1;
    }
    func_80055760();
    return 1;
}
