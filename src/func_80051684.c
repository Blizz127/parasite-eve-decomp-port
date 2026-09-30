extern unsigned char *D_8009D254;
void func_80051684(unsigned int a0) {
    unsigned char *p = D_8009D254;
    if (p) {
        p = *(unsigned char **)p;
        if (p)
            *(unsigned int *)(p + 8) = a0 << 16;
    }
}
