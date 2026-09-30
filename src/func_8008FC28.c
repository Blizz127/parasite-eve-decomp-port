void func_8008FC28(unsigned char *a0) {
    unsigned char *p = *(unsigned char **)a0;
    unsigned char c;
    *(unsigned char **)a0 = p + 1;
    c = *p;
    *(unsigned short *)(a0 + 0x7E) = c;
    if (c == 0)
        *(unsigned short *)(a0 + 0x7E) = 0x100;
    {
        unsigned char *q = *(unsigned char **)a0;
        unsigned char d;
        *(unsigned char **)a0 = q + 1;
        d = *q;
        *(unsigned short *)(a0 + 0xE4) = (signed char)d;
    }
}
