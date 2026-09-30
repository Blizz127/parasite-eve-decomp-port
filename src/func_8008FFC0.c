void func_8008FFC0(unsigned char *a0) {
    unsigned char *p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    *(unsigned short *)(a0 + 0xA6) = (unsigned short)(*p << 8);
}
