extern unsigned char *D_8009D2F0;
extern unsigned int *D_8009D300;
extern int D_8009CE00;
extern signed char func_8002FAF8(void *a0, int a1);

int func_800184EC(unsigned char **a0) {
    *(int *)a0[1] = func_8002FAF8(D_8009D2F0, *a0[0]);
    if (*(int *)a0[1] != 0) {
        return 1;
    }
    D_8009D300[4] = 1;
    D_8009CE00 -= 0x10;
    return 0;
}
