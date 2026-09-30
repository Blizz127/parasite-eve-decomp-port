extern int *D_8009D2F0;
extern int func_80079FB4(int a0, int a1);

int func_80019C4C(int **a0) {
    int t;
    int x;
    int y;

    y = (D_8009D2F0[0xA] - **a0) >> 16;
    x = (D_8009D2F0[0xC] - *a0[1]) >> 16;
    t = 0x1400 - func_80079FB4(x, y);
    if (t > 0x1000) {
        t -= 0x1000;
    }
    t -= ((short *)D_8009D2F0)[0x1D];
    if (t < 0) {
        t += 0x1000;
    }
    *a0[2] = t;
    return 1;
}
