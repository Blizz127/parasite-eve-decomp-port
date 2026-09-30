/* Forward a dereferenced argument to a builder and store the result.
 * VRAM 0x800194B0 / file 0x9CB0 / size 0x48. */
extern int func_80053D2C(int a0);

int func_800194B0(int *a0) {
    int v0 = a0[0];
    int r = func_80053D2C(*(int *)v0);

    *(int *)(a0[1]) = r;
    return 1;
}
