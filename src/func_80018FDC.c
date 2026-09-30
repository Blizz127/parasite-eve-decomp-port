extern unsigned int *D_8009D300;
extern int D_8009CE00;
extern short func_8006EBE4(void);
extern signed char func_8006EC08(void);

int func_80018FDC(int **a0) {
    if (func_8006EBE4() == **a0) {
        return 1;
    }
    if (func_8006EC08() != 0) {
        D_8009CE00 -= 0xC;
        D_8009D300[4] = 1;
        return 0;
    }
    return 1;
}
