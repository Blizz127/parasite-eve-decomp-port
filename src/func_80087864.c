void func_80087864(unsigned int a0, unsigned int a1) {
    unsigned short *p = (unsigned short *)(0x1F801C08 + (a0 << 4));
    *p = (*p & 0xFFF0) | a1;
}
