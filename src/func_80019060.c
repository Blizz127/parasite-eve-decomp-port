extern unsigned int *D_8009D300;
extern int D_8009CE00;
extern signed char func_8006EC08(void);

int func_80019060(void) {
    if (func_8006EC08() != 0) {
        D_8009CE00 -= 8;
        D_8009D300[4] = 1;
        return 0;
    }
    return 1;
}
