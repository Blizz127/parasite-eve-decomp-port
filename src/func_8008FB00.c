void func_8008FB00(unsigned char *a0) {
    unsigned char *p;
    unsigned char *cur;
    unsigned char *end;
    unsigned int lo;
    unsigned int hi;

    p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    lo = *p;
    cur = *(unsigned char **)a0;
    end = cur + 1;
    *(unsigned char **)a0 = end;
    hi = *cur;
    *(unsigned short *)(a0 + 0x5A) = 0xFF;
    *(unsigned short *)(a0 + 0xE2) = 0;
    *(unsigned int *)(a0 + 0x38) |= 0x1000;
    *(unsigned char **)(a0 + 0x18) = end + (short)(lo | (hi << 8));
}
