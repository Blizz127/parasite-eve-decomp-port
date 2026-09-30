void func_8008FBD4(int a0) {
    unsigned char *p = *(unsigned char **)a0;
    unsigned char c;
    *(unsigned char **)a0 = p + 1;
    c = *p;
    *(short *)(a0 + 0xDE) = (signed char)c;
}
