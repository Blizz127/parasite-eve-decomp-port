void func_80090108(unsigned char *a0) {
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
        val = ((*q << 7) - *(unsigned short *)(a0 + 0xB4)) / n;
    }
    *(unsigned short *)(a0 + 0xB6) = n;
    *(unsigned short *)(a0 + 0xB8) = val;
}
