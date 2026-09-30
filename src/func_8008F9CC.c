extern unsigned char *D_8009D2C8;
extern unsigned char D_800B2900[];
extern unsigned int D_800B2910[];
extern unsigned int D_800BCD50;
extern void func_8008F0D0(unsigned char *a0, unsigned char *a1, unsigned int a2);

void func_8008F9CC(unsigned char *a0, unsigned int a1) {
    unsigned char *p;
    unsigned int old;
    int c;
    unsigned char *rec;

    p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    c = *p;
    if (*(unsigned short *)(a0 + 0x54) == 0) {
        if (*(unsigned int *)D_8009D2C8 & 0x100) {
            if (c >= 0x20) {
                c += 0x30;
            }
        }
    }
    rec = &D_800B2900[c * 0x40];
    old = *(unsigned short *)(a0 + 0x5A);
    *(unsigned short *)(a0 + 0x5A) = c;
    if (old != 0xFF) {
        if (*(unsigned short *)(a0 + 0x54) != 0 ||
            ((*(unsigned int *)(D_8009D2C8 + 0x14) & a1) & D_800BCD50) == 0) {
            *(unsigned int *)(a0 + 0xF4) |= 0x10;
            *(unsigned int *)(a0 + 0x30) =
                (*(unsigned int *)(a0 + 0x30) * *(unsigned int *)(rec + 0x10)) /
                D_800B2910[old * 0x10];
        }
    }
    func_8008F0D0(a0, rec, 0x1010);
    *(unsigned int *)(a0 + 0x38) &= ~0x1000;
}
