extern void func_8006F6D4(int a0, int a1, int a2, int a3, int a4, int a5);

int func_800187C0(int **a0) {
    register int p0 asm("$5");
    int x4;
    int x2;

    p0 = (int)a0[0];
    x4 = *a0[3];
    x2 = *a0[1];
    p0 = *(int *)p0;
    func_8006F6D4(p0, 0, x2, *a0[2], x4, *a0[4]);
    return 1;
}
