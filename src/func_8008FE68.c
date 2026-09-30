void func_8008FE68(unsigned char *a0) {
    unsigned char *p;
    int n;
    int val;

    p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    n = *p;
    if (n == 0) {
        n = 0x100;
    }
    {
        unsigned char *q = *(unsigned char **)a0;
        *(unsigned char **)a0 = q + 1;
        val = ((*q << 8) - *(unsigned short *)(a0 + 0x94)) / n;
    }
    *(unsigned short *)(a0 + 0x96) = n;
    *(unsigned short *)(a0 + 0x98) = val;
}
