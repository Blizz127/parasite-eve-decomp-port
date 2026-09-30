void func_800907DC(unsigned char *a0) {
    unsigned char *p;
    int n;
    unsigned char *base;

    p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    n = *p;
    if (n == 0) {
        n = 0x100;
    }
    base = (unsigned char *)(*(unsigned short *)(a0 + 0xCE) * 2 + (unsigned int)a0);
    if (*(unsigned short *)(base + 0x62) + 1 != n) {
        *(unsigned char **)a0 = p + 3;
    } else {
        unsigned int lo;
        unsigned int hi;
        {
            unsigned char *q = *(unsigned char **)a0;
            *(unsigned char **)a0 = q + 1;
            lo = *q;
        }
        {
            unsigned char *r = *(unsigned char **)a0;
            *(unsigned char **)a0 = r + 1;
            hi = *r;
        }
        *(unsigned char **)a0 = *(unsigned char **)a0 + (short)(lo | (hi << 8));
    }
}
