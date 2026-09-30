void func_8008FCBC(int a0) {
    unsigned char *p = *(unsigned char **)a0;
    unsigned char c;
    *(unsigned char **)a0 = p + 1;
    c = *p;
    *(short *)(a0 + 0xE0) = (signed char)c;
}
