extern int D_800A76C8;
extern int D_800A76C4;
extern int D_800A76CC;

int func_80018660(int **a0) {
    int *p;
    int *q;

    p = &D_800A76C8;
    *p = (**a0 + *a0[1]) * 225 * 960 + *a0[2] * 60;
    if (*a0[3] == 1) {
        D_800A76CC = 0;
        D_800A76C4 |= 2;
    } else {
        *p = 0;
    }
    q = &D_800A76C4;
    *q = (*q | 1) & ~4;
    return 1;
}
