extern unsigned char *D_8009D2F0;
extern void func_8006FC18(unsigned int a, unsigned char *b, int c);
int func_800192DC(unsigned int *a0) {
    unsigned int x = *(unsigned int *)*a0;
    func_8006FC18(x, D_8009D2F0, 0);
    return 1;
}
