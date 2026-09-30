void func_80090970(unsigned char *a0) {
    signed char *p;
    int v;

    p = *(signed char **)a0;
    *(signed char **)a0 = p + 1;
    v = *p;
    if (v != 0) {
        v += *(short *)(a0 + 0xD0);
        if (v <= 0) {
            v = 1;
        } else if (v >= 0x100) {
            v = 0xFF;
        }
    }
    *(short *)(a0 + 0xD2) = v;
}
