extern unsigned char *D_8009D2F0;
int func_80017E68(unsigned int *a0) {
    unsigned int *o = (unsigned int *)D_8009D2F0;
    unsigned int v = o[0x98 / 4];
    unsigned int w = *(unsigned int *)(*(unsigned int *)a0);
    unsigned int notall = (v & w) ^ w;
    *(unsigned int *)a0[1] = (notall < 1);
    return 1;
}
