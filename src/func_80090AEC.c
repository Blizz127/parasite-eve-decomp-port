extern void func_8009019C(unsigned char *a0);

void func_80090AEC(unsigned char *a0) {
    unsigned char *p;
    int v;

    p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    v = *p;
    *(unsigned short *)(a0 + 0xBA) = v ? v + 1 : 0x101;
    func_8009019C(a0);
}
