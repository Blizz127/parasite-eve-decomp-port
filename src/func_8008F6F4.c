void func_8008F6F4(unsigned char *a0) {
    unsigned char *p;
    unsigned char c;
    unsigned short old;

    p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    c = *p;
    *(unsigned short *)(a0 + 0x74) = c;
    if (c == 0) {
        *(unsigned short *)(a0 + 0x74) = 0x100;
    }
    {
        int val;
        unsigned char *q = *(unsigned char **)a0;
        *(unsigned char **)a0 = q + 1;
        old = *(unsigned short *)(a0 + 0xD8) & 0xFF00;
        val = ((*q << 8) - (short)old) / *(unsigned short *)(a0 + 0x74);
        *(unsigned short *)(a0 + 0xD8) = old;
        *(unsigned short *)(a0 + 0xDA) = val;
    }
}
