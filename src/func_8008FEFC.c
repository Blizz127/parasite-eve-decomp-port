extern unsigned int D_8009C080[];

void func_8008FEFC(unsigned char *a0) {
    *(unsigned int *)(a0 + 0x38) |= 2;
    if (*(unsigned short *)(a0 + 0x54) != 0) {
        unsigned char *q = *(unsigned char **)a0;
        unsigned int v;
        *(unsigned short *)(a0 + 0x9C) = 0;
        *(unsigned char **)a0 = q + 1;
        v = *q;
        if (v != 0) {
            *(unsigned short *)(a0 + 0xA6) = v << 8;
        }
    } else {
        unsigned char *q = *(unsigned char **)a0;
        *(unsigned char **)a0 = q + 1;
        *(unsigned short *)(a0 + 0x9C) = *q;
    }
    {
        unsigned char *q = *(unsigned char **)a0;
        unsigned int v;
        *(unsigned char **)a0 = q + 1;
        v = *q;
        *(unsigned short *)(a0 + 0xA0) = v;
        if (v == 0) {
            *(unsigned short *)(a0 + 0xA0) = 0x100;
        }
    }
    {
        unsigned char *q = *(unsigned char **)a0;
        unsigned int idx;
        unsigned int t9c;
        unsigned int slot;
        *(unsigned char **)a0 = q + 1;
        idx = *q;
        t9c = *(unsigned short *)(a0 + 0x9C);
        *(unsigned short *)(a0 + 0xA4) = idx;
        slot = D_8009C080[idx];
        *(unsigned short *)(a0 + 0x9E) = t9c;
        *(unsigned short *)(a0 + 0xA2) = 1;
        *(unsigned int *)(a0 + 0x20) = slot;
    }
}
