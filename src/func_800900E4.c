void func_800900E4(unsigned char *a0) {
    unsigned char *p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    *(unsigned short *)(a0 + 0xB4) = (unsigned short)(*p << 7);
}
