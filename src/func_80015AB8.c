extern void func_800677A0(int a, int b, int c);
int func_80015AB8(unsigned char *a0) {
    unsigned int v0 = *(unsigned int *)(a0 + 0);
    unsigned int v1 = *(unsigned int *)(a0 + 4);
    unsigned int a2 = *(unsigned int *)(a0 + 8);
    func_800677A0(*(unsigned int *)v0, *(short *)v1, *(short *)a2);
    return 1;
}
