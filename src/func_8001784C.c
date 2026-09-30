extern unsigned int *D_8009D300;

int func_8001784C(unsigned int **a0) {
    *(unsigned int *)a0[0] = *(unsigned int *)((unsigned char *)D_8009D300 + 0x18);
    *(unsigned int *)a0[1] = *(unsigned int *)((unsigned char *)D_8009D300 + 0x1C);
    return 1;
}
