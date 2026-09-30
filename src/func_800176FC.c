extern int func_80070D6C(int a0, int a1);
extern int func_80070DD0(int a0, int a1);

int func_800176FC(int **a0) {
    register int x asm("$4");
    register int y asm("$5");

    x = *a0[1];
    y = *a0[2];
    if (x == y) {
        *a0[0] = func_80070D6C(x, y);
    } else {
        *a0[0] = func_80070DD0(x, y);
    }
    return 1;
}
