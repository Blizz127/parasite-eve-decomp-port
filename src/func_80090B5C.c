extern void func_800902AC(unsigned char *a0);

void func_80090B5C(unsigned char *a0) {
    unsigned char *p;
    int v;

    p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    v = *p;
    *(unsigned short *)(a0 + 0xBC) = v ? v + 1 : 0x101;
    func_800902AC(a0);
}
