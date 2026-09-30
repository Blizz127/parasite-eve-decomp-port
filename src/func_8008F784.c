void func_8008F784(unsigned char *a0) {
    unsigned char *p = *(unsigned char **)a0;
    unsigned char c;
    *(unsigned char **)a0 = p + 1;
    c = *p;
    *(unsigned short *)(a0 + 0x78) = 0;
    *(unsigned int *)(a0 + 0xF4) |= 3;
    *(unsigned short *)(a0 + 0x76) = (unsigned short)(((c + 0x40) & 0xFF) << 8);
}
