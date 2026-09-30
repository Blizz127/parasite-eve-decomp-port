void func_8008A02C(unsigned int *a0, unsigned int a1, int a2) {
    unsigned int *p = a0 + 1;
    unsigned int d = a1 - *a0;
    do {
        *a0 += d;
        *p += d;
        a0 += 0x10;
        p += 0x10;
        a2--;
    } while (a2);
}
