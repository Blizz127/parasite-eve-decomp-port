void func_8008F7BC(unsigned char *a0) {
    unsigned char *p;
    unsigned char c;
    unsigned short old;

    p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    c = *p;
    *(unsigned short *)(a0 + 0x78) = c;
    if (c == 0) {
        *(unsigned short *)(a0 + 0x78) = 0x100;
    }
    {
        int val;
        unsigned char *q = *(unsigned char **)a0;
        *(unsigned char **)a0 = q + 1;
        old = *(unsigned short *)(a0 + 0x76) & 0xFF00;
        val = ((((*q + 0x40) & 0xFF) << 8) - old) / *(unsigned short *)(a0 + 0x78);
        *(unsigned short *)(a0 + 0x76) = old;
        *(unsigned short *)(a0 + 0xDC) = val;
    }
}
