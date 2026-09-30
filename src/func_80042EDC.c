extern int D_8009CED8, D_8009CEE4, D_8009CEE8, D_8009CEDC;
extern unsigned char D_800BD024;
void func_80042EDC(void) {
    int v;
    D_8009CED8 = 1;
    D_8009CEE4 = 1;
    D_8009CEE8 = 0;
    v = D_800BD024;
    D_8009CEDC = v;
    if (v < 0) {
        D_8009CEDC = 1;
    } else if (v >= 0x21) {
        D_8009CEDC = 0x20;
    }
}
