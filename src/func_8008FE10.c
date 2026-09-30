void func_8008FE10(unsigned char *a0) {
    unsigned char *p;
    int f;
    int w;
    int prod;
    int cond;

    p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    *(unsigned short *)(a0 + 0x94) = *p << 8;
    f = (*(unsigned short *)(a0 + 0x94) & 0x7F00) >> 8;
    cond = *(unsigned short *)(a0 + 0x94) & 0x8000;
    w = *(int *)(a0 + 0x30);
    if (cond) {
        prod = f * w;
    } else {
        prod = f * ((w * 15) >> 8);
    }
    *(unsigned short *)(a0 + 0x92) = (unsigned int)prod >> 7;
}
