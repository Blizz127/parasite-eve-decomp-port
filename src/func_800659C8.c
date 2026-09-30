extern unsigned char *volatile D_800B1624;
int func_800659C8(unsigned int a0, unsigned int a1) {
    unsigned char *p = D_800B1624;
    unsigned char *q = D_800B1624;
    a1 >>= 8;
    q += *(unsigned int *)(p + 0x10);
    q += a0 << 4;
    *(unsigned short *)(q + 8) = a1;
    return 0;
}
