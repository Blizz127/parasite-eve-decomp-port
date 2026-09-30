void func_8008F430(unsigned char *a0) {
    unsigned char *p = *(unsigned char **)a0;
    unsigned char *q;
    unsigned char lo;
    unsigned char hi;
    *(unsigned char **)a0 = p + 1;
    lo = p[0];
    q = p + 2;
    *(unsigned char **)a0 = q;
    hi = p[1];
    q += (signed short)(lo | (hi << 8));
    *(unsigned char **)a0 = q;
}
