extern void func_800679C4(int a, int b, int c);
int func_80018B30(unsigned char *a0) {
    unsigned int v0 = *(unsigned int *)(a0 + 0);
    unsigned int v1 = *(unsigned int *)(a0 + 4);
    unsigned int a2 = *(unsigned int *)(a0 + 8);
    func_800679C4(*(short *)v0, *(short *)v1, *(short *)a2);
    return 1;
}
