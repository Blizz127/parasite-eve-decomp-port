extern unsigned char *D_8009D2C8;
extern unsigned short D_800BCD78;
extern unsigned int D_8009D2C4;

void func_800904C4(unsigned char *a0) {
    unsigned char *p;
    unsigned int c;

    p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    c = *p;
    if (*(unsigned short *)(a0 + 0x54) == 0) {
        if (c & 0xC0) {
            unsigned char *b = D_8009D2C8;
            *(unsigned short *)(b + 0x5A) =
                (*(unsigned short *)(b + 0x5A) + (c & 0x3F)) & 0x3F;
        } else {
            *(unsigned short *)(D_8009D2C8 + 0x5A) = c;
        }
    } else {
        if (c & 0xC0) {
            D_800BCD78 = (D_800BCD78 + (c & 0x3F)) & 0x3F;
        } else {
            D_800BCD78 = c;
        }
    }
    D_8009D2C4 |= 0x10;
}
