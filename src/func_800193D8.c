extern void func_80065AD4(int a, int b, int c);
int func_800193D8(unsigned char *a0) {
    unsigned int v0 = *(unsigned int *)(a0 + 0);
    unsigned int v1 = *(unsigned int *)(a0 + 4);
    unsigned int a2 = *(unsigned int *)(a0 + 8);
    func_80065AD4(*(unsigned int *)v0, *(unsigned int *)v1, *(unsigned int *)a2);
    return 1;
}
