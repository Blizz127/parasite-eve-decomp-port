/* Forward a dereferenced argument to a builder and store the result.
 * VRAM 0x80018C10 / file 0x9410 / size 0x48. */
extern int func_8006599C(int a0);

int func_80018C10(int *a0) {
    int v0 = a0[0];
    int r = func_8006599C(*(int *)v0);

    *(int *)(a0[1]) = r;
    return 1;
}
