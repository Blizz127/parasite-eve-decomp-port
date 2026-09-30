extern unsigned int *D_8009D2F0;
extern unsigned int *D_8009D300;
extern int D_8009CE00;

int func_80019B08(void) {
    unsigned short *d;
    unsigned short f;

    d = (unsigned short *)D_8009D300;
    f = d[4];
    if (f & 0x20) {
        if (D_8009D2F0[0x26] & 8) {
            d[4] = f & 0xFFDF;
            return 1;
        }
    } else {
        d[4] = f | 0x20;
    }
    D_8009CE00 -= 8;
    D_8009D300[4] = 1;
    return 0;
}
