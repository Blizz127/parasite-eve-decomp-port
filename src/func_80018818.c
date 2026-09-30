extern void func_8006F6D4(int a0, int a1, int a2, int *a3, int *a4, int *a5);

int func_80018818(int **a0) {
    int x0;
    int x2;

    x0 = *a0[0];
    x2 = *a0[1];
    func_8006F6D4(x0, 1, x2, a0[2], a0[3], a0[4]);
    return 1;
}
