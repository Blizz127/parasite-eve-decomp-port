extern unsigned char *D_8009D2F0;
int func_80019450(unsigned int **a0) {
    unsigned char *p = D_8009D2F0;
    int n = *(short *)(p + 0x224) << 1;
    int v = *(int *)*a0;
    unsigned char *q = *(unsigned char **)(p + 0x1B4);
    *(short *)(q + 0x14) = (short)((n * v) >> 16);
    return 1;
}
