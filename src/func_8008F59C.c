void func_8008F59C(unsigned char *a0) {
    if (*(unsigned int *)(a0 + 0x38) & 8) {
        unsigned char *p = *(unsigned char **)a0;
        *(unsigned char **)a0 = p + 1;
        *(unsigned short *)(a0 + 0x6A) = (unsigned short)(*p << 7);
    } else {
        signed char *p = *(signed char **)a0;
        int v;
        *(signed char **)a0 = p + 1;
        v = *p;
        *(unsigned short *)(a0 + 0x72) = 0;
        *(unsigned int *)(a0 + 0xF4) |= 3;
        *(unsigned int *)(a0 + 0x44) = v << 23;
    }
}
