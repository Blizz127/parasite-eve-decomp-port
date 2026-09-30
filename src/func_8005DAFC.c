extern int D_800A802C;
int func_8005DAFC(unsigned int a0) {
    int *p = &D_800A802C;
    int off = *p;
    unsigned char *base;
    p -= 1;
    base = (unsigned char *)off + (int)p;
    if (a0 < *(unsigned short *)base) return (int)base + *(short *)((a0 << 1) + (int)base + 2);
    return 0;
}
