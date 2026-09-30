void func_8008F514(unsigned char *a0) {
    unsigned char *p;
    unsigned char c;
    int old;

    p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    c = *p;
    *(unsigned short *)(a0 + 0x6E) = c;
    if (c == 0) {
        *(unsigned short *)(a0 + 0x6E) = 0x100;
    }
    {
        int val;
        unsigned char *q = *(unsigned char **)a0;
        *(unsigned char **)a0 = q + 1;
        old = *(unsigned short *)(a0 + 0x6C) & 0x7F00;
        val = ((*q << 8) - old) / *(unsigned short *)(a0 + 0x6E);
        *(unsigned short *)(a0 + 0x6C) = old;
        *(unsigned short *)(a0 + 0xD4) = val;
    }
}
