extern unsigned int D_8009D1A0;

int func_80017F44(unsigned int **a0) {
    if (D_8009D1A0 & **a0) {
        *a0[1] = 1;
    } else {
        *a0[1] = 0;
    }
    return 1;
}
