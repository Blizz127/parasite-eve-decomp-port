extern unsigned char *D_8009D2C8;
extern unsigned int D_8009D2C4;

void func_8008F328(unsigned char *a0) {
    unsigned char *base;
    unsigned int v;
    unsigned char *p;

    p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    v = (unsigned int)*p << 16;
    {
        unsigned char *q = *(unsigned char **)a0;
        *(unsigned char **)a0 = q + 1;
        base = D_8009D2C8;
        v |= (unsigned int)*q << 24;
    }
    *(unsigned short *)(base + 0x58) = 0;
    D_8009D2C4 |= 0x80;
    *(unsigned int *)(base + 0x40) = v;
}
