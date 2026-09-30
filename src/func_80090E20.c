extern unsigned char *D_8009D2C8;
extern void func_800903A0();

void func_80090E20(unsigned char *a0, int a1)
{
    unsigned char *p;
    unsigned char *q;
    unsigned int m;
    unsigned int bit;
    int i;
    int hi;
    unsigned char c;

    p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    if ((*(unsigned short *)(a0 + 0x60) = *p) == 0) {
        *(unsigned short *)(a0 + 0x60) = 0x100;
    }
    q = *(unsigned char **)a0;
    hi = *(unsigned short *)(a0 + 0x5E) & 0xFF00;
    *(unsigned char **)a0 = q + 1;
    c = *q;
    *(short *)(a0 + 0xD6) = ((short)(c << 8) - hi) / *(unsigned short *)(a0 + 0x60);
    *(unsigned short *)(a0 + 0x5E) = hi;
    if (!(*(unsigned int *)(a0 + 0x38) & 0x800)) {
        i = 0;
        bit = 1;
        m = *(unsigned int *)(D_8009D2C8 + 4) | *(unsigned int *)(D_8009D2C8 + 0x30);
        for (; bit & 0xFFFFFF; bit <<= 1, i++) {
            if (!(m & bit)) {
                break;
            }
        }
        if (bit & 0xFFFFFF) {
            *(unsigned int *)(D_8009D2C8 + 0x30) |= bit;
            *(unsigned short *)(a0 + 0x5C) = i;
            *(unsigned int *)(a0 + 0x38) |= 0x800;
        }
    }
    func_800903A0(a0, a1);
}
