extern unsigned char *volatile D_800B1624;
int func_80065A60(unsigned int a0, unsigned int a1, unsigned char a2) {
    unsigned char *p = D_800B1624;
    unsigned char *q = D_800B1624;
    q += *(unsigned int *)(p + 0x10);
    q += a0 << 4;
    q += *(unsigned int *)(q + 0xC);
    a1 = (a1 << 1) + (unsigned int)q;
    *(unsigned char *)(a1 + 1) = a2;
    return 0;
}
