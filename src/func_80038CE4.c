extern unsigned char *D_80091A28;
int func_80038CE4(unsigned int a0) {
    unsigned char *base = D_80091A28;
    unsigned char *p = base + (a0 & 0xFF);
    unsigned int v = *(unsigned char *)(p + 0x1D);
    return *(unsigned char *)(base + v + 4);
}
