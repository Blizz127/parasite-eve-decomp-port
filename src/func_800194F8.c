/* Forward a dereferenced argument to a builder and store the result.
 * VRAM 0x800194F8 / file 0x9CF8 / size 0x48. */
extern int func_80053E6C(int a0);

int func_800194F8(int *a0) {
    int v0 = a0[0];
    int r = func_80053E6C(*(int *)v0);

    *(int *)(a0[1]) = r;
    return 1;
}
