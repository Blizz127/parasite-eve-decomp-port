extern unsigned char *D_8009D2C8;

void func_8008F37C(unsigned char *a0) {
    unsigned char *p;
    unsigned short *fld;
    unsigned char c;
    int v;
    int old;

    p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    fld = (unsigned short *)(D_8009D2C8 + 0x58);
    c = *p;
    *fld = c;
    if (c == 0) {
        *fld = 0x100;
    }
    {
        unsigned char *q = *(unsigned char **)a0;
        *(unsigned char **)a0 = q + 1;
        v = *q << 16;
    }
    {
        unsigned char *r = *(unsigned char **)a0;
        *(unsigned char **)a0 = r + 1;
        v |= *r << 24;
    }
    old = *(int *)(D_8009D2C8 + 0x40) & ~0xFFFF;
    *(int *)(D_8009D2C8 + 0x40) = old;
    *(int *)(D_8009D2C8 + 0x44) =
        (v - old) / *(unsigned short *)(D_8009D2C8 + 0x58);
}
