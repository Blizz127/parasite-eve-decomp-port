extern unsigned int *D_8009D2F0;
extern int D_8009CE00;

int func_80017E20(unsigned int **a0) {
    if (**a0 != *a0[1]) {
        D_8009CE00 = D_8009D2F0[0x27] + *a0[2] * 2;
    }
    return 1;
}
