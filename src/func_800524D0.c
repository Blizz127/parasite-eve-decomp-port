extern unsigned char *D_8009D254;
int func_800524D0(void) {
    unsigned char *p = D_8009D254;
    if (p == 0) return 0;
    p = *(unsigned char **)p;
    if (p == 0) return 0;
    return (*(unsigned int *)(p + 0x4C) >> 9) & 1;
}
