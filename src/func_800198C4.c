extern unsigned char *D_8009D2F0;
extern void func_8002FAD8(unsigned char *a, unsigned int b, unsigned int c, unsigned int d);
int func_800198C4(unsigned int *a0) {
    unsigned int b = *(unsigned char *)*a0;
    func_8002FAD8(D_8009D2F0, b, *(unsigned int *)a0[1], *(unsigned int *)a0[2]);
    return 1;
}
