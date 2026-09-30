void func_8008F6B0(unsigned char *a0) {
    unsigned char *p = *(unsigned char **)a0;
    unsigned char c;
    unsigned int f;
    *(unsigned char **)a0 = p + 1;
    c = *p;
    f = *(unsigned int *)(a0 + 0x38);
    *(unsigned short *)(a0 + 0x74) = 0;
    *(unsigned short *)(a0 + 0xD8) = (unsigned short)(c << 8);
    if (f & 0x100)
        *(unsigned int *)(a0 + 0xF4) |= 3;
}
