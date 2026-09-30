void func_8008FCE4(unsigned char *a0) {
    unsigned char *p = *(unsigned char **)a0;
    unsigned char c;
    *(unsigned char **)a0 = p + 1;
    c = *p;
    {
        unsigned short v = *(unsigned short *)(a0 + 0xE0);
        v += (signed char)c;
        *(unsigned short *)(a0 + 0xE0) = v;
    }
}
