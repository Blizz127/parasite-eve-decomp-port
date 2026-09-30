extern void *D_8009D254;

int func_800C6CE0(unsigned char *a0) {
    void *p;
    int r;
    unsigned char t;

    p = *(void **)(a0 + 8);
    r = 0;
    if (p != 0) {
        unsigned char *q = *(unsigned char **)p;
        if (q == 0) {
            r = 1;
        } else {
            r = 3;
            if (*(int *)(q + 0x10) <= 0) {
                r = 2;
            }
        }
    }
    t = a0[1];
    if (t == 7) {
        r = 4;
    }
    if (t == 0xE) {
        r = 4;
    }
    if (p == D_8009D254) {
        r = 5;
    }
    return r;
}
