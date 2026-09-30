extern unsigned char *volatile D_800B1624;
int func_80065A9C(unsigned int a0, unsigned int a1) {
    unsigned char *p = D_800B1624;
    unsigned char *q = D_800B1624;
    q += *(unsigned int *)(p + 0x10);
    q += a0 << 4;
    *q = *q | (a1 & 0x30);
    return 0;
}
