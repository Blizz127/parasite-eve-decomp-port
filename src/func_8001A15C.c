extern int func_80079FB4(int a0, int a1);

int func_8001A15C(int **a0) {
    *a0[2] = func_80079FB4(**a0, *a0[1]);
    return 1;
}
