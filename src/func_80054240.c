extern unsigned char *D_8009D048;
extern unsigned char D_800C0E48[];
extern signed char D_800C0E20[];
extern signed char D_800C0E22[];

int func_80054240(int a0)
{
    int r;

    r = 0;
    if (D_8009D048 == D_800C0E48) {
        if (D_800C0E20[0] == a0 || D_800C0E22[0] == a0) {
            r = 1;
        }
    }
    return r;
}
