extern unsigned int *D_8009D2F0;
extern int func_8001CAB0(int a0, int a1, int a2, int a3);

int func_8001A390(int **a0) {
    int x2;

    x2 = D_8009D2F0[0x27] + *a0[3] * 2;
    *a0[4] = func_8001CAB0(*a0[0], *a0[1], x2, *(unsigned short *)a0[2]);
    return 1;
}
