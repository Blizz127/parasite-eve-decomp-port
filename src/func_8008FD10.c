extern unsigned int D_8009C080[];

void func_8008FD10(unsigned char *a0) {
    *(unsigned int *)(a0 + 0x38) |= 1;
    if (*(unsigned short *)(a0 + 0x54) != 0) {
        unsigned char *q = *(unsigned char **)a0;
        unsigned int v;
        *(unsigned short *)(a0 + 0x88) = 0;
        *(unsigned char **)a0 = q + 1;
        v = *q;
        if (v != 0) {
            *(unsigned short *)(a0 + 0x94) = v << 8;
        }
    } else {
        unsigned char *q = *(unsigned char **)a0;
        *(unsigned char **)a0 = q + 1;
        *(unsigned short *)(a0 + 0x88) = *q;
    }
    {
        unsigned char *q = *(unsigned char **)a0;
        unsigned int v;
        *(unsigned char **)a0 = q + 1;
        v = *q;
        *(unsigned short *)(a0 + 0x8C) = v;
        if (v == 0) {
            *(unsigned short *)(a0 + 0x8C) = 0x100;
        }
    }
    {
        unsigned char *q = *(unsigned char **)a0;
        int w = *(unsigned short *)(a0 + 0x30);
        unsigned int t;
        unsigned int f;
        unsigned int prod;
        register unsigned int idx asm("$2");
        register unsigned int r7 asm("$3");
        register unsigned int t88 asm("$3");
        register unsigned int slot asm("$4");
        *(unsigned char **)a0 = q + 1;
        *(unsigned short *)(a0 + 0x90) = *q;
        t = *(unsigned short *)(a0 + 0x94);
        f = (t & 0x7F00) >> 8;
        if ((t & 0x8000) == 0) {
            prod = f * ((w * 15) >> 8);
        } else {
            prod = f * w;
        }
        r7 = prod >> 7;
        idx = *(unsigned short *)(a0 + 0x90);
        __asm__ __volatile__("" : "=r"(idx) : "0"(idx));
        *(unsigned short *)(a0 + 0x92) = r7;
        t88 = *(unsigned short *)(a0 + 0x88);
        __asm__ __volatile__("" : "=r"(t88) : "0"(t88));
        slot = D_8009C080[idx];
        *(unsigned short *)(a0 + 0x8A) = t88;
        *(unsigned short *)(a0 + 0x8E) = 1;
        *(unsigned int *)(a0 + 0x1C) = slot;
    }
}
