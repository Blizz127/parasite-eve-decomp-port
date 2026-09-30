extern unsigned char *D_8009D2C8;

void func_8008F224(unsigned char *a0) {
    unsigned char *base;
    unsigned char *p;

    p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    base = D_8009D2C8;
    *(unsigned int *)(base + 0x20) = (unsigned int)*p << 16;
    {
        unsigned char *q = *(unsigned char **)a0;
        unsigned char c;
        unsigned int t;
        *(unsigned char **)a0 = q + 1;
        c = *q;
        t = *(unsigned int *)(base + 0x20);
        *(unsigned short *)(base + 0x52) = 0;
        *(unsigned int *)(base + 0x20) = t | ((unsigned int)c << 24);
    }
}
