void func_8008783C(int a0, int a1) {
    unsigned char *p = (unsigned char *)0x1F801C08 + (a0 << 4);
    unsigned short v = *(unsigned short *)p;
    v = (v & 0xFF0F) | (a1 << 4);
    *(unsigned short *)p = v;
}
