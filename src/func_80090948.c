void func_80090948(int a0) {
    unsigned int v;
    unsigned char *p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    v = *p;
    *(unsigned short *)(a0 + 0xD2) = 0;
    *(unsigned short *)(a0 + 0x58) = v;
    *(unsigned short *)(a0 + 0x56) = v;
    *(unsigned short *)(a0 + 0xD0) = v;
}
