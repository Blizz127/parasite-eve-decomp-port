extern unsigned int D_8009C080[];

void func_80090078(unsigned char *a0) {
    unsigned char *p;
    int c;

    *(unsigned int *)(a0 + 0x38) |= 4;
    p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    c = *p;
    *(unsigned short *)(a0 + 0xAE) = c;
    if (c == 0) {
        *(unsigned short *)(a0 + 0xAE) = 0x100;
    }
    {
        unsigned char *q = *(unsigned char **)a0;
        unsigned char d;
        unsigned int val;
        *(unsigned char **)a0 = q + 1;
        d = *q;
        *(unsigned short *)(a0 + 0xB2) = d;
        val = D_8009C080[d];
        *(unsigned short *)(a0 + 0xB0) = 1;
        *(unsigned int *)(a0 + 0x24) = val;
    }
}
