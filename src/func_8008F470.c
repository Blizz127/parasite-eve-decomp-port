extern unsigned char *D_8009D2C8;

void func_8008F470(unsigned char *a0) {
    unsigned char *p;
    int c;

    p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    c = *p;
    if ((int)(unsigned short)*(unsigned short *)(D_8009D2C8 + 0x56) >= c) {
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
    } else {
        *(unsigned char **)a0 = p + 3;
    }
}
